#pragma once
#include "boost/asio.hpp"
#include "boost/function.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "block_detect_task.h"

#define  AMIB_MESSAGE_LENTH 11
using namespace boost::asio;

class peer_connection;

typedef boost::shared_ptr<peer_connection> peer_connection_ptr;

typedef boost::function<void(peer_connection_ptr)> disconnect_cbk_type;

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
class peer_connection : 
	public boost::noncopyable,
	public boost::enable_shared_from_this<peer_connection>
{
public:
	peer_connection(tcp_socket_ptr sock, disconnect_cbk_type disconnect_cbk, io_service& ios);
	~peer_connection();
	//
	// If the pending IO count on a socket is not 0, we cannot delete this `peer_connection` object, because it may raise a access violation,
	// since the `read_handler` and `write_handler` is running in IO thread, they will be called when the IO finished.
	// The best way is process the `timeout` event (which will call shutdown and close too) in `read_handler` and `write_handler` functions.
	// And it is better to post only one READ/WRITE request on a socket at the same time, it will make easier to control the deleting of `this` object.
	//
	int get_pending_io_count() { return pending_io_count; }

	int get_is_closed(){ return m_closed; }

	bool get_is_pingpong_task_finished() { return m_pingpong_task_finished; }

	bool get_is_pingpong_task_started() { return NULL != m_pingpong_task.get(); }

	bool get_is_watch_dog_finish_work() { return m_is_watch_dog_finish_work; }

	tcp_socket_ptr get_sock() { return m_sock; }

protected:
	void async_read_some();

	enum PeerCloseByWhom
	{
		close_by_watch_dog,
		close_by_read_handler,
		close_by_write_handler
	};

	void read_handler(const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes read.
		);

	void on_detect_finished(bool bOk);

	void write_handler(
		const boost::system::error_code& error, // Result of operation.
		std::size_t bytes_transferred           // Number of bytes sent.
		);

	void send_ok();

	void send_no();
private:
	// Make sure it would be called.
	void shutdown_and_close(PeerCloseByWhom w);

	/**
	 * \brief
	 * 
	 * \param error Result of operation.
	 * \return
	 */
	void watch_dog_handler(const boost::system::error_code& error);
private:
	std::vector<char> m_recv_buffer;		//The buffer used to read.
	std::vector<char> m_data_received;		//Data we have received.
	std::vector<char> m_send_buffer;
	tcp_socket_ptr m_sock;
	int m_data_max_size;
	disconnect_cbk_type m_disconnect_cbk;
	bool m_closed;
	// Should we need to wait for the task `m_pingpong_task` finished.
	// If this flag is set, this connection object cannot be deleted, or it may make a thread conflict when the `ping-pong` task is not finished.
	bool m_need_wait_task_finished;		
	int pending_io_count;
	pingpong_task_ptr m_pingpong_task;
	bool m_pingpong_task_finished;
	io_service& m_io;		//an ref to tcp_server::io_service.
	// a watch dog to make sure the socket is closed even when some error occurred.
	// we must prevent socket is use out, if some error occurred.
	boost::shared_ptr<boost::asio::deadline_timer> m_watch_dog_for_free_socket; 
	bool m_is_watch_dog_finish_work;
};


