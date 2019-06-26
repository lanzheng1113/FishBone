#pragma once
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/make_shared.hpp"
#include <iostream>

using namespace boost::asio;
class CAsioTCPClient
{
	typedef ip::tcp::socket socket_type;
	typedef boost::shared_ptr<socket_type> sock_ptr;
public:
	CAsioTCPClient(io_service& io, const boost::asio::ip::address& addr, int tid)
		: m_ep(addr, 6688)
		, m_io(io)
		, m_tid(tid)
	{
		*(unsigned short*)m_send_buf = 9;
		m_send_buf[2] = 'A';
		m_send_buf[3] = 'M';
		m_send_buf[4] = 'I';
		m_send_buf[5] = 'B';
		m_send_buf[6] = 0;
		*(unsigned short*)(m_send_buf + 7) = 3799;
		*(unsigned short*)(m_send_buf + 9) = 3799;
		m_sock = boost::make_shared<socket_type>(m_io);
		m_stoped = false;
	}

	void start()
	{
		m_sock->async_connect(m_ep, boost::bind(&CAsioTCPClient::connect_handler, this, _1));
	}

	bool stoped() const { return m_stoped; }
private:
	

	void connect_handler(const boost::system::error_code& ec)
	{
		if (ec)
		{
			std::cout << "[" << m_tid << "]" << "connect failed with error:" << ec << "(" << ec.message() << ")" << std::endl;
			close();
			return;
		}
		std::cout << "[" << m_tid << "]" << "send to:" << m_sock->remote_endpoint().address() << ":" << m_sock->remote_endpoint().port() << std::endl;
		m_sock->async_send(buffer(m_send_buf), boost::bind(&CAsioTCPClient::send_handler, this, _1, _2));
	}

	void recv_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes received.
		)
	{
		if (error || bytes_transferred == 0)
		{
			std::cout << "[" << m_tid << "]" << "error: " << error << ". Bytes: " << bytes_transferred << std::endl;
			close();
			return;
		}

		if (bytes_transferred == 3)
		{
			std::cout << "[" << m_tid << "]" << "We have received the respond." << std::endl;
			unsigned char reply_code = m_buf[2];
			std::cout << "[" << m_tid << "]" << "TCP:" << ((reply_code & 0x01) == 0 ? "Failed." : "Succeeded.") << std::endl;
			std::cout << "[" << m_tid << "]" << "UDP:" << ((reply_code & 0x02) == 0 ? "Failed." : "Succeeded.") << std::endl;
			std::cout << "[" << m_tid << "]" << "Socket is closing now." << std::endl;
			close();
		}
		else
		{
			std::cout << "[" << m_tid << "]" << "Failed to receive responds. " << std::endl;
			close();
		}
	}

	void send_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes sent.
		)
	{
		if (error || bytes_transferred == 0)
		{
			std::cout << "[" << m_tid << "]" << "error:" << error << "." << error.message() << ". Bytes:" << bytes_transferred << std::endl;
			close();
			return;
		}
		else
		{
			std::cout << "[" << m_tid << "]" << "sent " << bytes_transferred << " Bytes." << std::endl;
		}

		if (bytes_transferred == sizeof(m_send_buf))
		{
			memset(m_buf, 0, sizeof(m_buf));
			m_sock->async_receive(boost::asio::buffer(m_buf), boost::bind(&CAsioTCPClient::recv_handler, this, _1, _2));
			std::cout << "[" << m_tid << "]" << "now waiting for responds." << std::endl;
		}
		else
		{
			std::cout << "[" << m_tid << "]" << "sent bytes not correct, abort!" << std::endl;
			close();
			return;
		}
	}

	void close()
	{
		if (m_stoped)
		{
			return;
		}
		std::cout << "[" << m_tid << "]" << "Client is closing now" << std::endl;
		if (m_sock)
		{
			m_sock->close();
		}
		m_stoped = true;
	}

private:
	ip::tcp::endpoint m_ep;
	io_service& m_io;
	enum { max_length = 1024 };
	char m_buf[max_length];
	char m_send_buf[11] = { 0 };
	sock_ptr m_sock;
	bool m_stoped;
	int m_tid;
};
