#pragma once

#ifdef _WIN32
#ifdef _DEBUG
#define BTrace OutputDebugStringA
#else
#define BTrace
#endif // _DEBUG
#else
#ifdef _DEBUG
#define BTrace printf
#else
#define BTrace
#endif // _DEBUG
#endif

#ifdef _WIN32
#ifdef LOG_ENABLED
#include "util/logger.h"
#else
#define LOG_ERROR
#define LOG_INFO
#define LOG_DEBUG
#define LOG_FATAL
#define LOG_WARN
#endif // LOG_ENABLED
#else
#define LOG_ERROR
#define LOG_INFO
#define LOG_DEBUG
#define LOG_FATAL
#define LOG_WARN
#endif // _WIN32


#include "boost/asio.hpp"

using namespace boost::asio;

typedef ip::tcp::socket tcp_socket_type;
typedef boost::shared_ptr<tcp_socket_type> tcp_socket_ptr;

#ifdef _DEBUG
#define BASSERT(b) if(!(b)) throw std::logic_error("Assert exception raised.");
#else
#define BASSERT
#endif // _DEBUG
