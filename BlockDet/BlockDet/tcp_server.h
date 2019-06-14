#pragma once
#include "boost/asio.hpp"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"
#include "peer_connection.h"

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