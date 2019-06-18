#include "stdafx.h"
#include "AsioTCPClient.h"
#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"

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

void tcp_peer_connect(io_service& io)
{
	std::cout << "tcp client start." << std::endl;
	CAsioTCPClient cl(io);
	cl.start();
	while (!cl.stoped())
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
	std::cout << "tcp client finished." << std::endl;
}

int main()
{
	try
	{
		io_service io;
		boost::thread thd(tcp_peer_connect, boost::ref(io));
		async_udp_server udp_server(io);
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


