#pragma once
#ifdef _WIN32
#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#define sleep_seconds(A) Sleep((A)*1000)
#else
#include <stdio.h>
#include <unistd.h>
#define sleep_seconds(A) sleep(A)
#endif
