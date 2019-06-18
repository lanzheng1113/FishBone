#include "stdafx.h"
#include "AsioTCPClient.h"
#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"

using boost::asio::ip::udp;
enum { max_length = 1024 };

class async_udp_server
{
public:
	async_udp_server(boost::asio::io_service& io)
		: m_io(io)
	{
	}

	void run()
	{
		m_sock = boost::make_shared<udp::socket>(m_io, udp::endpoint(udp::v4(), 3799));
		std::cout << "UDP server with port 3799 is running now." << std::endl;
		boost::shared_ptr<udp::endpoint> sender_endpoint = boost::make_shared<udp::endpoint>();
		m_sock->async_receive_from(boost::asio::buffer(m_receive_buffer), *sender_endpoint, boost::bind(&async_udp_server::udp_async_recv_from_handler, this, _1, _2, sender_endpoint));
	}
protected:
private:
	void udp_async_recv_from_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred,           // Number of bytes received.
		boost::shared_ptr<udp::endpoint> sender_endpoint
		)
	{
		std::cout << "UDP data received " << bytes_transferred << " bytes" << std::endl;
		char send_buf[3] = { 0,1,3 };
		std::cout << "Send UDP data 013" << std::endl;
		m_sock->async_send_to(boost::asio::buffer(send_buf), *sender_endpoint,
			boost::bind(&async_udp_server::udp_async_send_to_handler, this, _1, _2, sender_endpoint));
	}

	void udp_async_send_to_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred,           // Number of bytes sent.
		boost::shared_ptr<udp::endpoint> sender_endpoint
		)
	{
		boost::shared_ptr<udp::endpoint> s = boost::make_shared<udp::endpoint>();
		m_sock->async_receive_from(boost::asio::buffer(m_receive_buffer), *s, boost::bind(&async_udp_server::udp_async_recv_from_handler, this, _1, _2, s));
	}
private:
	boost::asio::io_service& m_io;
	boost::shared_ptr<udp::socket> m_sock;
	char m_receive_buffer[max_length];
};

typedef ip::tcp::socket tcp_socket_type;
typedef boost::shared_ptr<tcp_socket_type> tcp_socket_ptr;

// this class is used to test `pingpong` task
class async_tcp_server
{
public:
	async_tcp_server(boost::asio::io_service& io)
		: m_io(io)
		, m_acceptor(m_io, ip::tcp::endpoint(ip::tcp::v4(), 3799))
	{
		accept();
	}
	~async_tcp_server() { ; }

protected:
private:
	void accept()
	{
		tcp_socket_ptr sock = boost::make_shared<tcp_socket_type>(m_io);
		m_acceptor.async_accept(*sock, boost::bind(&async_tcp_server::accept_handler, this, _1, sock));
	}

	void accept_handler(const boost::system::error_code& ec, tcp_socket_ptr sock)
	{
		if (ec)
		{
			return;
		}
		printf("SOCKET: %u Connected.\n", (unsigned int)sock->native_handle());
		// we do not need to do anything in `pingpong` task because we just test whether it can be connected or not. Just close it after connection was built.
		boost::system::error_code ec_unused;
		sock->shutdown(socket_base::shutdown_both, ec_unused);
		sock->close(ec_unused);
		printf("Socket is closed.");
		accept();
	}
private:
	boost::asio::io_service& m_io;
	ip::tcp::acceptor m_acceptor;
};

boost::shared_ptr<CAsioTCPClient> cc;

void resolve_handler(const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it, io_service& io)
{
	if (!ec)
	{
		std::cout << "resolved ip: " << it->endpoint().address().to_string() << std::endl;
		cc = boost::make_shared<CAsioTCPClient>(io, it->endpoint().address());
		cc->start();
	}
	else
	{
		std::cout << "Failed to resolve." << std::endl;
	}
}

int main()
{
	try
	{
		io_service io;
		std::cout << "tcp client start." << std::endl;
		boost::asio::ip::tcp::resolver resolver(io);
		boost::asio::ip::tcp::resolver::query query("pe.joy189.com", "8888");
		resolver.async_resolve(query, boost::bind(resolve_handler, _1, _2, boost::ref(io)));

		async_udp_server udp_server(io);
		async_tcp_server tcps_server(io);
		udp_server.run();
		io.run();
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	system("pause");
	return 0;
}


