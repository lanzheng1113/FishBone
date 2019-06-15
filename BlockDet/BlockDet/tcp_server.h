#pragma once
#include "boost/asio.hpp"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"
#include "peer_connection.h"
#include "boost/thread/recursive_mutex.hpp"

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
		LOG_INFO("SOCKET: %u Connected.\n",(unsigned int)sock->native_handle());
		peer_connection_ptr conn = boost::make_shared<peer_connection>(sock, boost::bind(&tcp_server::connection_remove, this, _1));
		boost::recursive_mutex::scoped_lock l(m_mutex_conn_list);
		m_peer_conns_am_i_blocked.push_back(conn);
		accept();
	}

	void connection_remove(peer_connection_ptr ptr)
	{
		bool bFindElement = false;
		boost::recursive_mutex::scoped_lock l(m_mutex_conn_list);
		for (std::list<peer_connection_ptr>::const_iterator it = m_peer_conns_am_i_blocked.begin(); it != m_peer_conns_am_i_blocked.end();)
		{
			if ((*it) == ptr)
			{
				if (ptr->get_pending_io_count() == 0)
				{
					LOG_INFO("connection object: %08x has been removed (IMM).\n", (unsigned int)ptr.get());
					it = m_peer_conns_am_i_blocked.erase(it);
					bFindElement = true;		//Debug TEST only. see the `BASSERT(bFindElement)` at the end of function;
				}
				else
					++it;
			}
			else
			{
				//Check other peers
				if (((*it)->get_is_closed()) && 0 == (*it)->get_pending_io_count())
				{
					LOG_INFO("connection object: %08x has been removed (DELAY).\n", (unsigned int)(*it).get());
					it = m_peer_conns_am_i_blocked.erase(it);
				}
				else
					++it;
			}
		}
		if (!bFindElement)
		{
			LOG_ERROR("ERROR! The object which wrapping SOCKET %u was not Found\n", (unsigned int)(ptr->get_sock()->native_handle()));
		}
		BASSERT(bFindElement);
	}
private:
	io_service m_io;
	ip::tcp::acceptor m_acceptor;
	std::list<peer_connection_ptr> m_peer_conns_am_i_blocked;
	boost::recursive_mutex m_mutex_conn_list;
};