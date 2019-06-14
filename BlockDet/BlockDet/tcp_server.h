#pragma once
#include "boost/asio.hpp"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"

#ifdef _WIN32
  #ifdef _DEBUG
  #define BTrace OutputDebugStringA
  #endif // _DEBUG
#else
  #ifdef _DEBUG
  #define BTrace printf
  #endif // _DEBUG
#endif


using namespace boost::asio;

typedef ip::tcp::socket tcp_socket_type;
typedef boost::shared_ptr<tcp_socket_type> tcp_socket_ptr;

#define  AMIB_MESSAGE_LENTH 11

class udp_connect_task
{
public:
protected:
private:
};

class tcp_connect_task
{
public:
protected:
private:
};

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
	peer_connection(tcp_socket_ptr sock)
	{
		BTrace("~peer_connection()");
		m_data_max_size = AMIB_MESSAGE_LENTH;
		m_recv_buffer.resize(m_data_max_size);
		m_send_buffer.resize(3);
		m_sock = sock;

		async_read_some();
	}
	~peer_connection()
	{
		BTrace("~peer_connection()");
	}

	void async_read_some()
	{
		m_sock->async_read_some(boost::asio::buffer(m_recv_buffer), boost::bind(&peer_connection::read_handler, this, _1, _2));
	}
protected:
	void read_handler(const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes read.
		)
	{
		if (error || 0 == bytes_transferred)
		{
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
		
		if (m_data_received.size() > m_data_max_size)
		{
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
				shutdown_and_close();
				return;
			}
			//
			//     get_ip()
			//     new_task((id)h_socket, ip, tcp_port_to_detect, udp_port_to_detect, boost::bind(send_ok,this), boost::bind(send_no,this)); 
			//

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
			m_sock->async_read_some(boost::asio::buffer(m_recv_buffer), boost::bind(&peer_connection::read_handler, this, _1, _2));
		}
	}

	void write_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes sent.
		)
	{
		if (error || bytes_transferred == 0)
		{
			shutdown_and_close();
			return;
		}

		//assert(bytes_transferred <= m_send_buffer.size())
		if (bytes_transferred != m_send_buffer.size())
		{
			std::vector<char> temp(m_send_buffer.begin() + bytes_transferred, m_send_buffer.end());
			m_send_buffer = temp;
			//Send the data left.
			m_sock->async_send(boost::asio::buffer(m_send_buffer), boost::bind(&peer_connection::write_handler, this, _1, _2));
		}
		else
		{
			//
			//All of the data has been sent.
			//Just shutdown and close the socket.
			//
			shutdown_and_close();
		}
	}

	void send_ok()
	{
		// 		struct reply
		// 		{
		// 			unsigned short length;
		// 			unsigned char reply_code;
		// 		};
		m_send_buffer[0] = 0;
		m_send_buffer[1] = 1;
		m_send_buffer[2] = 0;
		m_sock->async_send(boost::asio::buffer(m_send_buffer), boost::bind(&peer_connection::write_handler,this, _1, _2));
		return;
	}

	void send_no()
	{
		// 		struct reply
		// 		{
		// 			unsigned short length;
		// 			unsigned char reply_code;
		// 		};
		m_send_buffer[0] = 0;
		m_send_buffer[1] = 1;
		m_send_buffer[2] = 1;
		return;
	}
private:
	void shutdown_and_close()
	{
		boost::system::error_code ignored_ec;
		m_sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		m_sock->close(ignored_ec);
	}
private:
	std::vector<char> m_recv_buffer;		//The buffer used to read.
	std::vector<char> m_data_received;		//Data we have received.
	std::vector<char> m_send_buffer;
	tcp_socket_ptr m_sock;
	int m_data_max_size;
	udp_connect_task t_udp;
	tcp_connect_task t_tcp;
};

typedef boost::shared_ptr<peer_connection> peer_connection_ptr;



class tcp_server
{
public:

	tcp_server()
		:m_acceptor(m_io, ip::tcp::endpoint(ip::tcp::v4(), 6688))
	{ 
		accept();
	}
	~tcp_server() { ; }
public:
	void run()
	{
		boost::system::error_code ec(0, boost::system::system_category());
		m_io.run(ec);
		if (ec.value() != 0)
		{
			throw std::exception(ec.message().c_str());
		}
	}
private:
	void accept()
	{
		tcp_socket_ptr sock = boost::make_shared<tcp_socket_type>(m_io);
		m_acceptor.async_accept(*sock, boost::bind(&tcp_server::accept_handler, this, _1, sock));
	}

	void accept_handler(const boost::system::error_code& ec, tcp_socket_ptr sock)
	{
		if (ec)
		{
			return;
		}
		peer_connection_ptr conn = boost::make_shared<peer_connection>(sock);
		
		accept();
	}
private:
	io_service m_io;
	ip::tcp::acceptor m_acceptor;
	std::list<peer_connection_ptr> m_peer_conns_am_i_blocked;
	
};