// BlockDet.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "BlockDet.h"
#include "tcp_server.h"

int main()
{
	//
	//监听3999端口
	//
	tcp_server s;
	s.run();
    return 0;
}
// 
// /**
// * 客户端的请求
// */
// struct CLIENT_REQUEST
// {
// 	boost::asio::ip::address a;
// 	boost::asio::ip::tcp::socket sock_tcp;	//TCP的连接
// 	unsigned short tcp_port;
// 	boost::asio::ip::udp::socket sock_udp;
// 	unsigned short udp_port;
// };
// 
// #include "boost/function.hpp"
// #include <vector>
// //
// // 一个线程，用于处理异步连接tcp
// //
// bool exit_flag = false;
// typedef boost::function<void()> AsyncConnectTask;
// std::vector<AsyncConnectTask> tasks;
// 
// 
// void peer_connection::async_connect_tcp(boost::asio::ip::address addr, unsigned short port)
// {
// 	// 如果当前连接数超出了预设的最大的TCP连接数，则执行失败。
// 	if (out_of_quota())
// 	{
// 		int error_code = err_out_of_quote;
// 
// 	}
// 	boost::asio::ip::tcp::socket s;
// 	boost::asio::ip::tcp::endpoint ep(addr, port);
// 	s.set_option(time_out, 3000);
// 	boost::system::error_code ec;
// 	s.async_connect(ep, boost::bind(&peer_connection::on_connection_complete, self(), _1));
// 	m_connect_start = aux::time_now();
// }
// 
// void new_tasks(CLIENT_REQUEST& cr)
// {
// 	return;
// }
// 
// 
// void thread_async_connect_tcp()
// {
// 	while (!exit_flag)
// 	{
// 		
// 	}
// 	//关闭所有未释放的连接
// 	return;
// }
// // 一个线程，用于处理异步发送UDP请求
// 
// 
// 
// /**
//  * 将客户端的socket连接和客户端的请求绑定一块做个映射。
//  * 当超时或者完成任务断开这个连接的时候，将
//  */
// typedef std::map<boost::asio::ip::tcp::socket, CLIENT_REQUEST> CLIENT_REQUEST_MAP;