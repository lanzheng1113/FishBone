#include "stdafx.h"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"
#include "peer_connection.h"
#include "BlockDet.h"

peer_connection::peer_connection(tcp_socket_ptr sock, disconnect_cbk_type disconnect_cbk, io_service& ios) :m_closed(false),
m_sock(sock),
pending_io_count(0),
m_disconnect_cbk(disconnect_cbk),
m_io(ios),
m_need_wait_task_finished(true)
{
	LOG_INFO("[%08x]peer_connection() with [s:%u].\n", (unsigned int)this, m_sock->native_handle());
	BTrace("~peer_connection()");
	m_data_max_size = AMIB_MESSAGE_LENTH;
	m_recv_buffer.resize(m_data_max_size);
	m_send_buffer.resize(3);
	async_read_some();
}

peer_connection::~peer_connection()
{
	LOG_INFO("[%08x]~peer_connection().\n", (unsigned int)this);
	BTrace("~peer_connection()");
}

void peer_connection::async_read_some()
{
	if (m_closed)
	{
		LOG_INFO("[%08x]Async-READ-request is not delivered, because the socket has closed.\n", (unsigned int)this);
		return;
	}
	m_sock->async_read_some(boost::asio::buffer(m_recv_buffer), boost::bind(&peer_connection::read_handler, this, _1, _2));
	pending_io_count++;
	LOG_INFO("[%08x]An async-READ-request is delivered, the IO count is %d now\n", (unsigned int)this, pending_io_count);
}

int peer_connection::get_pending_io_count()
{
	return pending_io_count;
}

int peer_connection::get_is_closed()
{
	return m_closed;
}

tcp_socket_ptr peer_connection::get_sock()
{
	return m_sock;
}

void peer_connection::read_handler(const boost::system::error_code& error, /* Result of operation. */ std::size_t bytes_transferred /* Number of bytes read. */)
{
	pending_io_count--;
	if (m_closed)
	{
		LOG_ERROR("[%08x] received handle return. the connecting is closed (or closing).\n", (unsigned int)this);
		return;
	}

	if (error || 0 == bytes_transferred)
	{
		LOG_ERROR("[%08x]The connection receiving handler get a error: %d [%s], bytes transferred %u\n",
			(unsigned int)this, error.value(), error.message().c_str(), bytes_transferred);
		shutdown_and_close();
		return;
	}
	//
	// reserved, process time out.
	//
#ifdef _DEBUG
	if (0)
	{
		shutdown_and_close();
		return;
	}
#else
	throw std::logic_error("reserved for timeout.");
#endif
	//assert(m_recv_buffer.size() <= m_data_max_size)

	m_data_received.insert(m_data_received.end(), m_recv_buffer.begin(), m_recv_buffer.end());

	if (m_data_received.size() > (size_t)m_data_max_size)
	{
		LOG_ERROR("[%08x]The connection receiving handler get a data oversize error, max size or want size is %d but got %d.\n",
			(unsigned int)this, m_data_max_size, m_data_received.size());
		shutdown_and_close();
		return;
	}
	else if (m_data_received.size() == m_data_max_size)
	{
		//
		// received a full packet.
		// Note our message is a fixed message with length `m_data_size`, so we can simply calculate the message length whether equal to it;
		//
		unsigned short tcp_port = *(unsigned short*)(m_data_received.data() + 7);
		unsigned short udp_port = *(unsigned short*)(m_data_received.data() + 9);
		// Check the format of message.
		if (!((*(unsigned short*)(m_data_received.data()) == m_data_max_size - 2)
			&& (m_data_received[2] == 'A' && m_data_received[3] == 'M' && m_data_received[4] == 'I' && m_data_received[5] == 'B' && m_data_received[6] == 0)
			&& tcp_port != 0 && udp_port != 0))
		{
			LOG_ERROR("[%08x]The connection receiving handler got a data format error.", (unsigned int)this);
			shutdown_and_close();
			return;
		}
		//
		//     get_ip()
		//     new_task((id)h_socket, ip, tcp_port_to_detect, udp_port_to_detect, boost::bind(send_ok,this), boost::bind(send_no,this)); 
		//
		ip::tcp::endpoint ep_remote = m_sock->remote_endpoint();
		ip::address addr = ep_remote.address();
		LOG_INFO("[%08x]The connection receiving handler data received.", (unsigned int)this);
		m_pingpong_task = boost::make_shared<pingpong_task>(shared_from_this(), addr, tcp_port, udp_port, m_io, boost::bind(&peer_connection::on_detect_finished, this, _1));
	}
	else
	{
		// Some data is still on the way.
		LOG_ERROR("[%08x]The connection receiving handler: some data is still on the way, %d bytes want %d bytes received.\n",
			(unsigned int)this, m_data_max_size, m_data_received.size());
		m_recv_buffer.clear();
		m_recv_buffer.resize(m_data_max_size);
		m_sock->async_read_some(boost::asio::buffer(m_recv_buffer), boost::bind(&peer_connection::read_handler, this, _1, _2));
		pending_io_count++;
	}
}

void peer_connection::on_detect_finished(bool bOk)
{
	if (bOk)
	{
		send_ok();
	}
	else
	{
		send_no();
	}
}

void peer_connection::write_handler(const boost::system::error_code& error, /* Result of operation. */ std::size_t bytes_transferred /* Number of bytes sent. */)
{
	pending_io_count--;
	if (m_closed)
	{
		LOG_ERROR("[%08x] write handle return. the connecting is closed (or closing).\n", (unsigned int)this);
		return;
	}

	if (error || bytes_transferred == 0)
	{
		LOG_ERROR("[%08x]The connection receiving handler get a error: %d [%s] bytes transferred %u\n",
			(unsigned int)this, error.value(), error.message().c_str(), bytes_transferred);
		shutdown_and_close();
		return;
	}

	//assert(bytes_transferred <= m_send_buffer.size())
	if (bytes_transferred != m_send_buffer.size())
	{
		LOG_INFO("[%08x]The `ACK` has partly sent, total %d send %u.\n", (unsigned int)this, m_send_buffer.size(), bytes_transferred);
		std::vector<char> temp(m_send_buffer.begin() + bytes_transferred, m_send_buffer.end());
		m_send_buffer = temp;
		//Send the data left.
		m_sock->async_send(boost::asio::buffer(m_send_buffer), boost::bind(&peer_connection::write_handler, this, _1, _2));
		pending_io_count++;
	}
	else
	{
		//
		//All of the data has been sent.
		//Just shutdown and close the socket.
		//
		LOG_INFO("[%08x]The `ACK` had been sent.\n", (unsigned int)this);
		shutdown_and_close();
	}
}

void peer_connection::send_ok()
{
	if (m_closed)
	{
		LOG_INFO("[%08x]The connection had been closed, abort sending `ACK-Ok`\n", (unsigned int)this);
		return;
	}
	// 		struct reply
	// 		{
	// 			unsigned short length;
	// 			unsigned char reply_code;
	// 		};
	m_send_buffer[0] = 0;
	m_send_buffer[1] = 1;
	m_send_buffer[2] = 0;
	m_sock->async_send(boost::asio::buffer(m_send_buffer), boost::bind(&peer_connection::write_handler, this, _1, _2));
	pending_io_count++;
	LOG_INFO("[%08x]The `ACK-Yes` is sending.\n", (unsigned int)this);
	return;
}

void peer_connection::send_no()
{
	if (m_closed)
	{
		LOG_INFO("[s:%u]The connection had been closed, abort sending `ACK-No`\n", (unsigned int)m_sock->native_handle());
		return;
	}
	// 		struct reply
	// 		{
	// 			unsigned short length;
	// 			unsigned char reply_code;
	// 		};
	m_send_buffer[0] = 0;
	m_send_buffer[1] = 1;
	m_send_buffer[2] = 1;
	m_sock->async_send(boost::asio::buffer(m_send_buffer), boost::bind(&peer_connection::write_handler, this, _1, _2));
	pending_io_count++;
	LOG_INFO("[s:%u]The `ACK-No` is sending.\n", (unsigned int)m_sock->native_handle());
	return;
}

void peer_connection::shutdown_and_close()
{
	BASSERT(!m_closed);
	if (!(m_pingpong_task->tcp_completed() && m_pingpong_task->udp_completed()))
	{
		// how to waiting for abort finished? we are definitely can not wait here use a `while(true) sleep(1)`. 
		// because this function is maybe called in an io service thread.
		m_pingpong_task->abort();
	}
	LOG_INFO("[%08x]Closing connection.\n", (unsigned int)this);
	boost::system::error_code ignored_ec;
	m_sock->shutdown(ip::tcp::socket::shutdown_both, ignored_ec);
	m_sock->close(ignored_ec);
	//Note: please make sure this callback is the last function called in this (peer_connection) object.
	m_disconnect_cbk(shared_from_this());
	m_closed = true;
}
