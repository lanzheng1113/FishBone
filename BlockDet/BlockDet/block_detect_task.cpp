#include "stdafx.h"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"
#include "BlockDet.h"
#include "block_detect_task.h"

pingpong_task::pingpong_task(const peer_connection_ptr& conn, const ip::address& addr, unsigned short tcp_port, unsigned short udp_port, io_service& io, function_type_task_complete on_task_completed) : m_conn(conn),
m_addr(addr),
m_tcp_port(tcp_port),
m_udp_port(udp_port),
m_time_out(PINGPONG_TIMEOUT_DEFAULT),
m_io(io),
m_tcp_completed(false),
m_tcp_result(false),
m_udp_completed(false),
m_udp_result(false),
m_on_all_task_completed(on_task_completed),
m_abort(false),
m_is_notified(false)
{
	udp_snd_buf[0] = 0;
	udp_snd_buf[1] = 1;
	udp_snd_buf[2] = 3;
	start();
}

pingpong_task::~pingpong_task()
{
	;
}

void pingpong_task::abort(EAbortByWhom w)
{
	//
	// IMPORTANT! 
	// (1)We must make sure this function can trigger `m_on_all_task_completed` at last!
	// The peer connection depend on it to determinate whether a connection object can be free.
	// (2)Consider Thread A called abort() and thread B is setting m_tcp_completed/m_udp_completed and will call `m_on_all_task_completed` very soon.
	// It seems has a conflict here. Is there an scene that `m_on_all_task_completed` would not be called?
	//

	//
	// if the abort() function is called by timer, do not need to cancel the timer.
	// 
	if (w == abort_by_external)
	{
		m_task_timeout_timer->cancel();
	}
	//
	// We can not abort when the recv/send is on processing.
	//
	boost::recursive_mutex::scoped_lock l(m_mutex);
	if (m_tcp_completed && m_udp_completed)
	{
		LOG_ERROR("It seem some conflict here when abort is called.");
		return;
	}

	m_abort = true;
	if (!m_tcp_completed)
	{
		if (m_tcp_sock->is_open())
		{
			boost::system::error_code ec;
			m_tcp_sock->shutdown(socket_base::shutdown_both, ec);
			m_tcp_sock->close(ec);
		}
	}

	if (!m_udp_completed)
	{
		if (m_udp_sock->is_open())
		{
			boost::system::error_code ec;
			m_udp_sock->shutdown(socket_base::shutdown_both, ec);
			m_udp_sock->close(ec);		// this would make the on recv failed.
		}
	}
}

void pingpong_task::start()
{
	BASSERT(m_start_time == 0);	//This function is supposed to call only once.
	m_start_time = time(NULL);

	//m_task_timeout_timer will be expired in 3 seconds and the expired_callback will be called if expired.
	m_task_timeout_timer = boost::make_shared<boost::asio::deadline_timer>(m_io, boost::posix_time::milliseconds(m_time_out));
	m_task_timeout_timer->async_wait(boost::bind(&pingpong_task::task_timeout_handler, this, _1));

	m_tcp_sock = boost::make_shared<ip::tcp::socket>(m_io);
	ip::tcp::endpoint ep(m_addr, m_tcp_port);
	m_tcp_sock->async_connect(ep, boost::bind(&pingpong_task::tcp_connect_handler, this, _1));

	m_udp_sock = boost::make_shared<ip::udp::socket>(m_io, ip::udp::endpoint(ip::udp::v4(), 0));
	m_udp_sock->async_send_to(buffer(udp_snd_buf), ip::udp::endpoint(m_addr, m_udp_port), boost::bind(&pingpong_task::udp_send_to_handler, this, _1, _2));
	boost::system::error_code none_used;
	LOG_INFO("task started try connect to %s, port: tcp %d, udp %d", m_addr.to_string(none_used).c_str(), m_tcp_port, m_udp_port);
}

void pingpong_task::tcp_connect_handler(const boost::system::error_code& ec)
{
	boost::recursive_mutex::scoped_lock l(m_mutex);
	if (m_abort)
	{
		//The socket has been closed when `abort` is called .
		//So we do not need to close it.
		m_tcp_result = false;
		m_tcp_completed = true;
		LOG_INFO("Try notify the peer-connection object that our work has finished. The tcp socket has been closed when `abort` is called, So we do not need to close it.");
		check_notify();
	}

	if (ec)
	{
		LOG_INFO("pingpong_task [%p] tcp connect failed with error:%d(%s)", this, ec.value(), ec.message().c_str());
		m_tcp_result = false;
	}
	else
	{
		LOG_INFO("pingpong_task [%p] tcp connecting is successful.", this);
		m_tcp_result = true;
		boost::system::error_code ec_s;
		m_tcp_sock->shutdown(ip::tcp::socket::shutdown_both, ec_s);
		m_tcp_sock->close(ec_s);
	}

	m_tcp_completed = true;
	if (m_udp_completed)
	{
		check_notify();
	}
}

void pingpong_task::udp_send_to_handler(const boost::system::error_code& error, /* Result of operation. */ std::size_t bytes_transferred /* Number of bytes sent. */)
{
	if (m_abort)
	{
		//The socket has been closed when `abort` is called .
		//So we do not need to close it.
		m_udp_result = false;
		m_udp_completed = true;
		LOG_INFO("Try notify the peer-connection object that our work has finished. The udp socket has been closed when `abort` is called, So we do not need to close it.");
		check_notify();
	}

	if (error)
	{
		LOG_INFO("pingpong_task [%p] udp send failed with error:%d(%s)", this, error.value(), error.message().c_str());
		udp_close(false);
		return;
	}

	if (bytes_transferred != 3)
	{
		LOG_INFO("pingpong_task [%p] udp send failed because sent bytes is wrong. expert 3 sent %d", this, bytes_transferred);
		udp_close(false);
		return;
	}
	ip::udp::endpoint ep_udp(m_addr, m_udp_port);   // must this style in linux, define it before called. unknown reason.
	m_udp_sock->async_receive_from(buffer(udp_rcv_buf), ep_udp, boost::bind(&pingpong_task::udp_rcv_handler, this, _1, _2));
}

void pingpong_task::udp_rcv_handler(const boost::system::error_code& error, /* Result of operation. */ std::size_t bytes_transferred /* Number of bytes received. */)
{
	boost::recursive_mutex::scoped_lock l(m_mutex);
	if (error)
	{
		LOG_INFO("pingpong_task [%p] udp recv failed with error:%d(%s)", this, error.value(), error.message().c_str());
		udp_close(false);
		return;
	}

	if (bytes_transferred != 3)
	{
		LOG_INFO("pingpong_task [%p] udp recv failed because sent bytes is wrong. expert 3 received %d", this, bytes_transferred);
		udp_close(false);
		return;
	}

	LOG_INFO("pingpong_task [%p] succeeded", this);
	udp_close(true);
}

void pingpong_task::udp_close(bool result)
{
	boost::system::error_code ec;
	m_udp_sock->shutdown(socket_base::shutdown_both, ec);
	m_udp_sock->close(ec);
	m_udp_result = result;
	m_udp_completed = true;
	LOG_INFO("UDP socket has closed");
	check_notify();
}

void pingpong_task::check_notify()
{
	if (m_is_notified)
	{
		return;
	}

	if (m_tcp_completed && m_udp_completed)
	{
		LOG_INFO("cancel the task timer.");
		m_task_timeout_timer->cancel();
		m_is_notified = true;
		LOG_INFO("Notify the peer-connection object that our work has finished.");
		m_on_all_task_completed(m_tcp_result, m_udp_result);
	}
}

void pingpong_task::task_timeout_handler(const boost::system::error_code& error)
{
	if (boost::asio::error::operation_aborted == error.value())
	{
		// The timer was canceled for some reason,
		// such as this task was aborted by `peer_connection` object or all task has been finished before the timer expired.
		// No need to do anything when being canceled.
		LOG_INFO("pingpong_task [%p]: task timer canceled.", this);
		return;
	}

	// timer expired in the setting time.
	LOG_INFO("pingpong_task [%p]: task timer expired.", this);
	m_task_timeout_timer->expires_at(boost::posix_time::pos_infin);
	if (m_tcp_completed && m_udp_completed)
	{
		// All completed. We should log the situation should not happen.
		// It seems some multi-thread conflict happened.
		LOG_WARN("pingpong_task [%p] It seems some multi-thread conflict happened. Because the tcp/udp task were completed, timer should be abort but not expired the timer in this case.", this);
		return;
	}
	else
	{
		LOG_WARN("pingpong_task [%p] Timer expired, aborting the task.", this);
		abort(abort_by_timer);
	}
}
