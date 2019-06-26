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

LONG WINAPI DumpMiniDump(PEXCEPTION_POINTERS excpInfo) {
    HANDLE hFile = CreateFile(_T("CrashDump.dmp"),
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = excpInfo;
	eInfo.ClientPointers = FALSE;
	MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		excpInfo ? &eInfo : NULL,
		NULL,
		NULL);

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif // _WIN32

int main() {
#ifdef _WIN32
    SetUnhandledExceptionFilter(DumpMiniDump);
#endif

#ifdef LOG_ENABLED
    DateTime dt;
    std::stringstream ss;
    ss << Path::getApplicationDirPath() << "log_" << dt.getYear() << dt.getMonth() << dt.getDay() << dt.getHour() << dt.getMinute() << dt.getSecond() << ".txt";
    Logger::getInstance()->setLogFileName(ss.str());
#endif // #ifdef LOG_ENABLED

    tcp_server s;
    try {
        s.run();
    } catch (std::exception& e) {
        printf("error: %s", e.what());
    }

    return 0;
}
