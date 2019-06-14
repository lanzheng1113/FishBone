#pragma once

#ifdef _WIN32
#ifdef _DEBUG
#define BTrace OutputDebugStringA
#endif // _DEBUG
#else
#ifdef _DEBUG
#define BTrace printf
#endif // _DEBUG
#endif

#include "boost/asio.hpp"

using namespace boost::asio;

typedef ip::tcp::socket tcp_socket_type;
typedef boost::shared_ptr<tcp_socket_type> tcp_socket_ptr;

