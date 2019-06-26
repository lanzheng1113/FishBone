#pragma once
#include "boost/asio.hpp"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"
#include "peer_connection.h"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/thread.hpp"
#include <list>


class tcp_server {
public:

    tcp_server()
    : m_acceptor(m_io, ip::tcp::endpoint(ip::tcp::v4(), 6688)) {
        b_print_stop = false;
        accept();
    }

    ~tcp_server() {
        ;
    }
public:

    void run() {
        boost::system::error_code ec(0, boost::system::system_category());
#ifndef DISABLE_PRINT_COUNT
        m_t = boost::make_shared<boost::thread>(&tcp_server::print_task_count_not_freed, this);
#endif
        m_io.run(ec);
        if (ec.value() != 0) {
            printf("Server has stop running with a error : %d/%s\n", ec.value(), ec.message().c_str());
            throw std::logic_error(ec.message().c_str());
        }
        //wait all connection object has been deleted
        printf("The server is now waitting all connections quiting safely ...\n");
        while(m_peer_conns_am_i_blocked.size())
        {
            try_remove_connections();
            sleep_seconds(1);
            printf("Waiting seconds: %d\r");
        }
        printf("\n");
    }
    
    void stop()
    {
        m_io.stop();
    }

private:

    void accept() {
        tcp_socket_ptr sock = boost::make_shared<tcp_socket_type>(m_io);
        m_acceptor.async_accept(*sock, boost::bind(&tcp_server::accept_handler, this, _1, sock));
    }

#ifndef DISABLE_PRINT_COUNT

    void print_task_count_not_freed() {
        while (true) {
            if (!b_print_stop) {
                boost::recursive_mutex::scoped_lock l(m_mutex_conn_list);
                printf("count of tasks : %u\n", m_peer_conns_am_i_blocked.size());
            }
            boost::this_thread::sleep(boost::posix_time::seconds(1));
        }
    }
#endif

    void accept_handler(const boost::system::error_code& ec, tcp_socket_ptr sock) {
        if (ec) {
            printf("the accept_hander receive an error %d/%s\n", ec.value(), ec.message().c_str());
            return;
        }
        LOG_INFO("SOCKET: %u Connected.", (unsigned int) sock->native_handle());
        peer_connection_ptr conn = boost::make_shared<peer_connection>(sock, boost::bind(&tcp_server::connection_remove, this, _1), m_io);
        boost::recursive_mutex::scoped_lock l(m_mutex_conn_list);
        m_peer_conns_am_i_blocked.push_back(conn);
        accept();
    }
    
    void try_remove_connections()
    {
        boost::recursive_mutex::scoped_lock l(m_mutex_conn_list);
        for (std::list<peer_connection_ptr>::iterator it = m_peer_conns_am_i_blocked.begin(); it != m_peer_conns_am_i_blocked.end();) {
            {
                // Check other peers
                // if `peer-connection` is closed, and no pending io on the closed socket.
                if (((*it)->get_is_closed())
                        && 0 == (*it)->get_pending_io_count()
                        && (*it)->get_is_watch_dog_finish_work()
                        && (((*it)->get_is_pingpong_task_started() && (*it)->get_is_pingpong_task_finished()) || !(*it)->get_is_pingpong_task_started())) {
                    // and if the `ping-pong` task which base on the connection has been finished, or not started yet (many reason cause that, such as invalid request from client, or an unexpected network error).
                    LOG_INFO("connection object: %p has been removed (DELAY).", (*it).get());
                    it = m_peer_conns_am_i_blocked.erase(it);
                } else
                    ++it;
            }
        }
    }

    void connection_remove(peer_connection_ptr ptr) {
        try_remove_connections();
    }
private:
    io_service m_io;
    ip::tcp::acceptor m_acceptor;
    std::list<peer_connection_ptr> m_peer_conns_am_i_blocked;
    boost::recursive_mutex m_mutex_conn_list;
    bool b_print_stop;
#ifndef DISABLE_PRINT_COUNT
    boost::shared_ptr<boost::thread> m_t;
#endif
};