// TestClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
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
	CAsioTCPClient() :m_ep(ip::address::from_string("127.0.0.1"), 6688)
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
		start();
	}

	void run()
	{
		m_io.run();
	}

	bool stoped() const { return m_stoped; }
private:
	void start()
	{
		m_sock->async_connect(m_ep, boost::bind(&CAsioTCPClient::connect_handler, this, _1));
	}

	void connect_handler(const boost::system::error_code& ec)
	{
		if (ec)
		{
			std::cout << "connect failed with error:" << ec << std::endl;
			close();
			return;
		}
		std::cout << "send to:" << m_sock->remote_endpoint().address() << ":" << m_sock->remote_endpoint().port() << std::endl;
		m_sock->async_send(buffer(m_send_buf), boost::bind(&CAsioTCPClient::send_handler, this, _1, _2));
	}

	void recv_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes received.
		)
	{
		if (error || bytes_transferred == 0)
		{
			std::cout << "error: " << error << ". Bytes: " << bytes_transferred << std::endl;
			close();
			return;
		}

		if (bytes_transferred == 3)
		{
			std::cout << "We have received the respond.Socket is closing now." << std::endl;
			close();
		}
		else
		{
			std::cout << "Failed to receive responds. " << std::endl;
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
			std::cout << "error:" << error << "." << error.message() << ". Bytes:" << bytes_transferred << std::endl;
			close();
			return;
		}
		else
		{
			std::cout << "sent " << bytes_transferred << " Bytes." << std::endl;
		}

		if (bytes_transferred == sizeof(m_send_buf))
		{
			memset(m_buf, 0, sizeof(m_buf));
			m_sock->async_receive(boost::asio::buffer(m_buf), boost::bind(&CAsioTCPClient::recv_handler, this, _1, _2));
			std::cout << "now waiting for responds." << std::endl;
		}
		else
		{
			std::cout << "sent bytes not correct, abort!" << std::endl;
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
		std::cout << "Client is closing now" << std::endl;
		if (m_sock)
		{
			m_sock->close();
		}
		m_io.stop();
		m_stoped = true;
	}

private:
	io_service m_io;
	ip::tcp::endpoint m_ep;
	enum { max_length = 1024 };
	char m_buf[max_length];
	char m_send_buf[11] = { 0 };
	sock_ptr m_sock;
	bool m_stoped;
};

#include "boost/thread.hpp"

int main()
{
	try
	{
		std::cout << "client start." << std::endl;
		CAsioTCPClient cl;
		cl.run();
		while (!cl.stoped())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	


	system("pause");
	return 0;
}


