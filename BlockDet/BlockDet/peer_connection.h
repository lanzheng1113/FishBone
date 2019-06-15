#pragma once
#include "boost/asio.hpp"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"
#include "boost/function.hpp"

#define  AMIB_MESSAGE_LENTH 11
using namespace boost::asio;

class udp_connect_task;
class tcp_connect_task;

class peer_connection;
typedef boost::function<void(peer_connection*)> disconnect_cbk_type;
//
//Remote send message:
//
//'\x00\x09'(2 Byte: length of the following data) 'AMIB\0x00'(5 Bytes Message body) '\x0000'(2 Bytes: tcp_port) '\x0000'(2 Bytes: udp_port)
//total 11 bytes, the structure is like follow struct:
// 		struct message_am_i_blocked {
// 			unsigned short msg_body_len = 9;
// 			struct message_boddy{
// 				char msg[5] = "AMIB";
// 				unsigned short tcp_port;
// 				unsigned short udp_port;
// 			}_msg;
//       }
// 	
class peer_connection
	: public boost::noncopyable
{
public:
	peer_connection(tcp_socket_ptr sock, disconnect_cbk_type disconnect_cbk)
		:m_closed(false),
		m_sock(sock),
		pending_io_count(0)
	{
		LOG_INFO("[%u]peer_connection().\n", m_sock->native_handle());
		BTrace("~peer_connection()");
		m_data_max_size = AMIB_MESSAGE_LENTH;
		m_recv_buffer.resize(m_data_max_size);
		m_send_buffer.resize(3);
		async_read_some();
	}
	~peer_connection()
	{
		LOG_INFO("[%u]~peer_connection().\n", m_sock->native_handle());
		BTrace("~peer_connection()");
	}

	void async_read_some()
	{
		if (m_closed)
		{
			LOG_INFO("Async-READ-request is not delivered, because the socket has closed.\n");
			return;
		}
		m_sock->async_read_some(boost::asio::buffer(m_recv_buffer), boost::bind(&peer_connection::read_handler, this, _1, _2));
		pending_io_count++;
		LOG_INFO("An async-READ-request is delivered, the IO count is %d now\n", pending_io_count);
	}

	//
	// If the pending IO count on a socket is not 0, we cannot delete this `peer_connection` object, because it may raise a access violation,
	// since the `read_handler` and `write_handler` is running in IO thread, they will be called when the IO finished.
	// The best way is process the `timeout` event (which will call shutdown and close too) in `read_handler` and `write_handler` functions.
	// And it is better to post only one READ/WRITE request on a socket at the same time, it will make easier to control the deleting of `this` object.
	//
	int get_pending_io_count()
	{
		return pending_io_count;
	}

	int get_is_closed()
	{
		return m_closed;
	}

	tcp_socket_ptr get_sock()
	{
		return m_sock;
	}
protected:
	void read_handler(const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes read.
		)
	{
		pending_io_count--;
		if (m_closed)
		{
			LOG_ERROR("[s:%u] received handle return. the connecting is closed (or closing).\n", (unsigned int)m_sock->native_handle());
			return;
		}

		if (error || 0 == bytes_transferred)
		{
			LOG_ERROR("[s:%u]The connection receiving handler get a error: %d [%s], bytes transferred %u\n", 
				(unsigned int)m_sock->native_handle(), error.value(), error.message().c_str(), bytes_transferred);
			shutdown_and_close();
			return;
		}
		//
		// reserved, process time out.
		//
		if (0)
		{
			shutdown_and_close();
			return;
		}

		//assert(m_recv_buffer.size() <= m_data_max_size)

		m_data_received.insert(m_data_received.end(), m_recv_buffer.begin(), m_recv_buffer.end());

		if (m_data_received.size() > (size_t)m_data_max_size)
		{
			LOG_ERROR("[s:%u]The connection receiving handler get a data oversize error, max size or want size is %d but got %d.\n", 
				(unsigned int)m_sock->native_handle(), m_data_max_size, m_data_received.size());
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
				LOG_ERROR("[s:%u]The connection receiving handler got a data format error.", (unsigned int)m_sock->native_handle());
				shutdown_and_close();
				return;
			}
			//
			//     get_ip()
			//     new_task((id)h_socket, ip, tcp_port_to_detect, udp_port_to_detect, boost::bind(send_ok,this), boost::bind(send_no,this)); 
			//

			LOG_INFO("[s:%u]The connection receiving handler data received.", (unsigned int)m_sock->native_handle());
			//
			// Here is the testing code.
			//
			if (time(NULL) % 2)
			{
				send_ok();
			}
			else
				send_no();
		}
		else
		{
			// Some data is still on the way.
			LOG_ERROR("[s:%u]The connection receiving handler: some data is still on the way, %d bytes want %d bytes received.\n", 
				(unsigned int)m_sock->native_handle(), m_data_max_size, m_data_received.size());
			m_recv_buffer.clear();
			m_recv_buffer.resize(m_data_max_size);
			m_sock->async_read_some(boost::asio::buffer(m_recv_buffer), boost::bind(&peer_connection::read_handler, this, _1, _2));
			pending_io_count++;
		}
	}

	void write_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes sent.
		)
	{
		pending_io_count--;
		if (m_closed)
		{
			LOG_ERROR("[s:%u] write handle return. the connecting is closed (or closing).\n", (unsigned int)m_sock->native_handle()); 
			return;
		}
			
		if (error || bytes_transferred == 0)
		{
			LOG_ERROR("[s:%u]The connection receiving handler get a error: %d [%s] bytes transferred %u\n", 
				(unsigned int)m_sock->native_handle(), error.value(), error.message().c_str(), bytes_transferred);
			shutdown_and_close();
			return;
		}

		//assert(bytes_transferred <= m_send_buffer.size())
		if (bytes_transferred != m_send_buffer.size())
		{
			LOG_INFO("[s:%u]The `ACK` has partly sent, total %d send %u.\n", (unsigned int)m_sock->native_handle(), m_send_buffer.size(), bytes_transferred);
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
			LOG_INFO("[s:%u]The `ACK` had been sent.\n", (unsigned int)m_sock->native_handle());
			shutdown_and_close();
		}
	}

	void send_ok()
	{
		if (m_closed)
		{
			LOG_INFO("[s:%u]The connection had been closed, abort sending `ACK-Ok`\n", (unsigned int)m_sock->native_handle());
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
		LOG_INFO("[s:%u]The `ACK-Yes` is sending.\n", (unsigned int)m_sock->native_handle());
		return;
	}

	void send_no()
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
private:
	//
	// 必须要保证shutdown_and_close这个函数
	//
	void shutdown_and_close()
	{
		BASSERT(!m_closed);
		boost::system::error_code ignored_ec;
		m_sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		m_sock->close(ignored_ec);
		//Note: please make sure this callback is the last function called in this (peer_connection) object.
		m_disconnect_cbk(this);
		m_closed = true;
		LOG_INFO("[s:%u]The connection has been closed.\n", (unsigned int)m_sock->native_handle());
	}
private:
	std::vector<char> m_recv_buffer;		//The buffer used to read.
	std::vector<char> m_data_received;		//Data we have received.
	std::vector<char> m_send_buffer;
	tcp_socket_ptr m_sock;
	int m_data_max_size;
	boost::shared_ptr<udp_connect_task> t_udp;
	boost::shared_ptr<tcp_connect_task> t_tcp;
	disconnect_cbk_type m_disconnect_cbk;
	bool m_closed;
	int pending_io_count;
};

typedef boost::shared_ptr<peer_connection> peer_connection_ptr;

