// BlockDet.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "BlockDet.h"
#include "tcp_server.h"

int main()
{
	//
	//����3999�˿�
	//
	tcp_server s;
	s.run();
    return 0;
}
// 
// /**
// * �ͻ��˵�����
// */
// struct CLIENT_REQUEST
// {
// 	boost::asio::ip::address a;
// 	boost::asio::ip::tcp::socket sock_tcp;	//TCP������
// 	unsigned short tcp_port;
// 	boost::asio::ip::udp::socket sock_udp;
// 	unsigned short udp_port;
// };
// 
// #include "boost/function.hpp"
// #include <vector>
// //
// // һ���̣߳����ڴ����첽����tcp
// //
// bool exit_flag = false;
// typedef boost::function<void()> AsyncConnectTask;
// std::vector<AsyncConnectTask> tasks;
// 
// 
// void peer_connection::async_connect_tcp(boost::asio::ip::address addr, unsigned short port)
// {
// 	// �����ǰ������������Ԥ�������TCP����������ִ��ʧ�ܡ�
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
// 	//�ر�����δ�ͷŵ�����
// 	return;
// }
// // һ���̣߳����ڴ����첽����UDP����
// 
// 
// 
// /**
//  * ���ͻ��˵�socket���ӺͿͻ��˵������һ������ӳ�䡣
//  * ����ʱ�����������Ͽ�������ӵ�ʱ�򣬽�
//  */
// typedef std::map<boost::asio::ip::tcp::socket, CLIENT_REQUEST> CLIENT_REQUEST_MAP;