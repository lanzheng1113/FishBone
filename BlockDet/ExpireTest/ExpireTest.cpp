// ExpireTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "boost/asio.hpp"
#include "boost/thread.hpp"

void task_timeout_handler(boost::system::error_code ec)
{
	if (boost::asio::error::operation_aborted == ec.value())
	{
		printf("timer canceled\n");
	}
	else
		printf("timer expired\n");
	return;
}

void tt(boost::asio::deadline_timer& t)
{
	Sleep(5000);
	boost::system::error_code ec;
	t.cancel(ec);
	if (ec)
	{
		printf("%s\n", ec.message().c_str());
	}
	
}

int main()
{
	boost::asio::io_service IOService;
	boost::asio::deadline_timer t(IOService, boost::posix_time::seconds(3));
	t.async_wait(task_timeout_handler);
	boost::thread thd(tt, boost::ref(t));
	IOService.run();
	if (thd.joinable())
	{
		thd.join();
	}
	system("pause");
    return 0;
}

