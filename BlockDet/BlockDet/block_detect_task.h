#pragma once
#include "boost/asio.hpp"
#include "boost/noncopyable.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/function.hpp"
#include "BlockDet.h"

#define PINGPONG_TIMEOUT_DEFAULT 2000

typedef boost::function<void(bool)> function_type_task_complete;

using namespace boost::asio;

class peer_connection;
typedef boost::shared_ptr<peer_connection> peer_connection_ptr;

enum EAbortByWhom
{
	abort_by_external,
	abort_by_timer
};

class pingpong_task :
	public boost::noncopyable
{
public:
	pingpong_task(const peer_connection_ptr& conn, const ip::address& addr, unsigned short tcp_port, 
		unsigned short udp_port, io_service& io, function_type_task_complete on_task_completed);

	~pingpong_task();

	time_t start_time() const { return m_start_time; }
	bool tcp_completed() const { return m_tcp_completed; }
	bool tcp_result() const { return m_tcp_result; }
	bool udp_completed() const { return m_udp_completed; }
	bool udp_result() const { return m_udp_result; }

	void abort(EAbortByWhom w);
protected:
private:
	void start();

	void tcp_connect_handler(const boost::system::error_code& ec);

	void udp_send_to_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes sent.
		);

	void udp_rcv_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes received.
		);

	void udp_close(bool result);

	void check_notify();

	/**
	 * \brief when task time out the function would be called, or when task is completed (timer cancel).
	 * 
	 * \param Result of operation.
	 * \return
	 */
	void task_timeout_handler(const boost::system::error_code& error);
private:
	io_service& m_io;				//use to launch a connection on tcp/udp.
	peer_connection_ptr m_conn;		//the tcp connection "user-->this server".
	ip::address m_addr;				//user address we are going to connect.
	unsigned short m_tcp_port;		//
	unsigned short m_udp_port;		//

	unsigned int m_time_out;		//resereved.
	time_t m_start_time;			//task start time.

	bool m_tcp_completed;			//true if tcp task finished, false for on processing.
	bool m_tcp_result;				//true for connected, false for not connected or timeout.
	boost::shared_ptr<ip::tcp::socket> m_tcp_sock;	// the tcp socket "this server --> user"

	bool m_udp_completed;
	bool m_udp_result;
	boost::shared_ptr<ip::udp::socket> m_udp_sock;	// the udp socket "this server --> user"

	char udp_snd_buf[3];
	char udp_rcv_buf[3];

	//
	// (1)it is a very important callback to notify `peer_connection` object that we have completed both of tcp and udp task.
	// NOTE: if this function not called, the `peer_connection` object which contain this `task` would not be freed.
	// So make sure call it when the tasks are completed or canceled.
	// (2)use check_notify instead of m_on_all_task_completed, the `check_notify` function will cancel the timer.
	//
	function_type_task_complete m_on_all_task_completed;	

	bool m_abort;						//is aborting.
	boost::recursive_mutex m_mutex;		//This mutex make sure that `abort`/`on send`/`on recv` can not work on the same time.

	boost::shared_ptr<boost::asio::deadline_timer> m_task_timeout_timer;	//when this timer is expired or canceled the function `task_timeout_handler` would be called.

	bool m_is_notified;
};

typedef boost::shared_ptr<pingpong_task> pingpong_task_ptr;
