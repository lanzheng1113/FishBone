// BlockDet.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "BlockDet.h"
#include "tcp_server.h"
#ifdef LOG_ENABLED
#include "util/Path.h"
#include "util/DateTime.h"
#include <sstream>
#endif

#ifdef _WIN32
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

LONG WINAPI DumpMiniDump(PEXCEPTION_POINTERS excpInfo)
{
	HANDLE hFile = CreateFile(_T("CrashDump.dmp"),
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId(); //把需要的信息添进去
	eInfo.ExceptionPointers = excpInfo;
	eInfo.ClientPointers = FALSE;

	// 调用, 生成Dump. 98不支持
	// Dump的类型是小型的, 节省空间. 可以参考MSDN生成更详细的Dump.
	MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		excpInfo ? &eInfo : NULL,
		NULL,
		NULL);

	CloseHandle(hFile);

	return EXCEPTION_EXECUTE_HANDLER;
}
#endif // _WIN32


int main()
{
#ifdef _WIN32
	SetUnhandledExceptionFilter(DumpMiniDump);
#endif
	//
	//监听3999端口
	//
#ifdef LOG_ENABLED
	DateTime dt;
	std::stringstream ss;
	ss << Path::getApplicationDirPath() << "log_" << dt.getYear() << dt.getMonth() << dt.getDay() << dt.getHour() << dt.getMinute() << dt.getSecond() << ".txt";
	Logger::getInstance()->setLogFileName(ss.str());
#endif // #ifdef LOG_ENABLED

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