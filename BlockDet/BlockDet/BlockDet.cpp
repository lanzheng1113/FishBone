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
	eInfo.ThreadId = GetCurrentThreadId(); //����Ҫ����Ϣ���ȥ
	eInfo.ExceptionPointers = excpInfo;
	eInfo.ClientPointers = FALSE;

	// ����, ����Dump. 98��֧��
	// Dump��������С�͵�, ��ʡ�ռ�. ���Բο�MSDN���ɸ���ϸ��Dump.
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
	//����3999�˿�
	//
#ifdef LOG_ENABLED
	DateTime dt;
	std::stringstream ss;
	ss << Path::getApplicationDirPath() << "log_" << dt.getYear() << dt.getMonth() << dt.getDay() << dt.getHour() << dt.getMinute() << dt.getSecond() << ".txt";
	Logger::getInstance()->setLogFileName(ss.str());
#endif // #ifdef LOG_ENABLED

	tcp_server s;
        try{
            s.run();
        }
        catch(std::exception& e)
        {
            printf("error: %s",e.what());
        }
	
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