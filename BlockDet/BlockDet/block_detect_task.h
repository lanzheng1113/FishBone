#pragma once
#include "boost/asio.hpp"
#include "boost/noncopyable.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "BlockDet.h"

#define PINGPONG_TIMEOUT_DEFAULT 2000

class udp_connect_task
	: public boost::noncopyable
{
public:
protected:
private:
};

class tcp_connect_task
	: public boost::noncopyable
{
public:
protected:
private:
};

typedef boost::function<void(bool)> function_type_task_complete;

using namespace boost::asio;

class peer_connection;
typedef boost::shared_ptr<peer_connection> peer_connection_ptr;

class pingpong_task :
	public boost::noncopyable
{
public:
	pingpong_task(const peer_connection_ptr& conn, const ip::address& addr, unsigned short tcp_port, 
		unsigned short udp_port, io_service& io, function_type_task_complete on_task_completed)
		: m_conn(conn),
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
		m_abort(false)
	{
		udp_snd_buf[0] = 0;
		udp_snd_buf[1] = 1;
		udp_snd_buf[2] = 3;
		start();
	}

	~pingpong_task()
	{
		;
	}

	time_t start_time() const { return m_start_time; }
	bool tcp_completed() const { return m_tcp_completed; }
	bool tcp_result() const { return m_tcp_result; }
	bool udp_completed() const { return m_udp_completed; }
	bool udp_result() const { return m_udp_result; }

	void abort()
	{
		//
		// IMPORTANT! 
		// (1)We must make sure this function can trigger `m_on_all_task_completed` at last!
		// The peer connection depend on it to determinate whether a connection object can be free.
		// (2)Consider Thread A called abort() and thread B is setting m_tcp_completed/m_udp_completed and will call `m_on_all_task_completed` very soon.
		// It seems has a conflict here. Is there an scene that `m_on_all_task_completed` would not be called?
		//
		
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
				m_tcp_sock->shutdown(socket_base::shutdown_both);
				m_tcp_sock->close();
			}
		}

		if (!m_udp_completed)
		{
			if (m_udp_sock->is_open())
			{
				m_udp_sock->shutdown(socket_base::shutdown_both);
				m_udp_sock->close();		// this would make the on recv failed. //<<<==== break here.
			}
		}
	}
protected:
private:
	void start()
	{
		BASSERT(m_start_time == 0);	//This function is supposed to call only once.
		m_start_time = time(NULL);
		m_tcp_sock = boost::make_shared<ip::tcp::socket>(m_io);
		ip::tcp::endpoint ep(m_addr, m_tcp_port);
		m_tcp_sock->async_connect(ep, boost::bind(&pingpong_task::tcp_connect_handler, this, _1));
		
		m_udp_sock = boost::make_shared<ip::udp::socket>(m_io);
		m_udp_sock->async_send_to(buffer(udp_snd_buf), ip::udp::endpoint(m_addr, m_udp_port), boost::bind(&pingpong_task::udp_send_to_handler, this, _1, _2));
	}

	void tcp_connect_handler(const boost::system::error_code& ec)
	{
		if (m_abort)
		{
			//The socket has been closed when `abort` is called .
			//So we do not need to close it.
			m_tcp_result = false;
			m_tcp_completed = true;
			check_notify();
		}

		boost::recursive_mutex::scoped_lock l(m_mutex);
		if (ec)
		{
			LOG_INFO("pingpong_task [%08x] tcp connect failed with error:%d(%s)", (unsigned int)this, ec.value(), ec.message().c_str());
			m_tcp_result = false;
		}
		else
		{
			LOG_INFO("pingpong_task [%08x] is successful.", (unsigned int)this);
			m_tcp_result = true;
			m_tcp_sock->shutdown(ip::tcp::socket::shutdown_both);
			m_tcp_sock->close();
		}
		m_tcp_completed = true;
		if (m_udp_completed)
		{
			m_on_all_task_completed(m_tcp_result && m_udp_result);
		}
	}

	void udp_send_to_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes sent.
		)
	{
		if (m_abort)
		{
			//The socket has been closed when `abort` is called .
			//So we do not need to close it.
			m_udp_result = false;
			m_udp_completed = true;
			check_notify();
		}

		if (!m_udp_sock->is_open())
		{
			LOG_INFO("pingpong_task [%08x] refuse to handle the udp send event as socket has been closed (or not open yet).", (unsigned int)this);
			return;
		}

		if (error)
		{
			LOG_INFO("pingpong_task [%08x] udp send failed with error:%d(%s)", (unsigned int)this, error.value(), error.message().c_str());
			udp_close(false);
			return;
		}

		if (bytes_transferred != 3)
		{
			LOG_INFO("pingpong_task [%08x] udp send failed because sent bytes is wrong. expert 3 sent %d", (unsigned int)this, bytes_transferred);
			udp_close(false);
			return;
		}

		m_udp_sock->async_receive_from(buffer(udp_rcv_buf), ip::udp::endpoint(m_addr, m_udp_port), boost::bind(&pingpong_task::udp_rcv_handler, this, _1, _2));
	}

	void udp_rcv_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes received.
		)
	{
		boost::recursive_mutex::scoped_lock l(m_mutex);
		if (!m_udp_sock->is_open())
		{
			LOG_INFO("pingpong_task [%08x] refuse to handle udp socket recv event as socket has been closed (or not open yet)", (unsigned int)this);
			return;
		}

		if (error)
		{
			LOG_INFO("pingpong_task [%08x] udp recv failed with error:%d(%s)", (unsigned int)this, error.value(), error.message().c_str());
			udp_close(false);
			return;
		}

		if (bytes_transferred != 3)
		{
			LOG_INFO("pingpong_task [%08x] udp recv failed because sent bytes is wrong. expert 3 received %d", (unsigned int)this, bytes_transferred);
			udp_close(false);
			return;
		}

		LOG_INFO("pingpong_task [%08x] succeeded\n", (unsigned int)this);
		udp_close(true);
	}

	void udp_close(bool result)
	{
		m_udp_sock->shutdown(socket_base::shutdown_both);
		m_udp_sock->close();
		m_udp_result = result;
		m_udp_completed = true;
		check_notify();
	}

	void check_notify()
	{
		static bool is_notified = false;
		if (is_notified)
		{
			return;
		}
		if (m_tcp_completed && m_udp_completed)
		{
			is_notified = true;
			m_on_all_task_completed(m_tcp_result && m_udp_result);
		}
	}

private:
	io_service& m_io;				//use to launch a connection on tcp/udp.
	peer_connection_ptr m_conn;		//the tcp connection "user-->this server".
	ip::address m_addr;				//user address we are going to connect.
	unsigned short m_tcp_port;		//
	unsigned short m_udp_port;		//

	unsigned int m_time_out;		//resereved.
	time_t m_start_time;			//task start time.

	bool m_tcp_completed;			//true if tcp task finished, false for on processing.
	bool m_tcp_result;				//true for connected, false for not connected or timeout.
	boost::shared_ptr<ip::tcp::socket> m_tcp_sock;	// the tcp socket "this server --> user"

	bool m_udp_completed;
	bool m_udp_result;
	boost::shared_ptr<ip::udp::socket> m_udp_sock;	// the udp socket "this server --> user"

	char udp_snd_buf[3];
	char udp_rcv_buf[3];

	function_type_task_complete m_on_all_task_completed;

	bool m_abort;				//is aborting.
	boost::recursive_mutex m_mutex;		//This mutex make sure that `abort`/`on send`/`on recv` can not work on the same time.
};

typedef boost::shared_ptr<pingpong_task> pingpong_task_ptr;
