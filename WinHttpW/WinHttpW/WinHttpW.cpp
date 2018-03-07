#include "StdAfx.h"
#include <Windows.h>
#include "WinHttpW.h"
#include "util/stringex.h"
#include "util/Logger.h"

namespace network
{
	CWinHttpW::CWinHttpW(void)
	{
		m_strUserAgent = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2)";
		m_nResolveTimeout = 10000;
		m_nConnectTimeout = 15000;
		m_nSendTimeout = 120000;
		m_nReceiveTimeout = 300000;
		m_hRequest = 0;
	}

	void CWinHttpW::setTimeouts( int resolveTimeout /*= 10000*/, int connectTimeout /*= 15000*/, int sendTimeout /*= 120000*/, int receiveTimeout /*= 300000*/ )
	{
		m_nResolveTimeout = resolveTimeout;
		m_nConnectTimeout = connectTimeout;
		m_nSendTimeout = sendTimeout;
		m_nReceiveTimeout = receiveTimeout;
	}

	CWinHttpW::~CWinHttpW(void)
	{
	}

	void CWinHttpW::rest( const string& userAgent )
	{
		m_strUserAgent = userAgent;
		if (m_hSession)
		{
			WinHttpCloseHandle(m_hSession);
		}
	}

	void CWinHttpW::setUserAgent( const string& userAgentValue )
	{
		m_strUserAgent = userAgentValue;
	}

	void CWinHttpW::autoCookies( int isAutoCookies )
	{
		m_cookieStrategy = isAutoCookies;
	}

	bool CWinHttpW::open( ERequestType requestType,const string& URL )
	{
		memset(&m_URLInfo,0,sizeof(m_URLInfo));
		memset(&m_lpUrlComponents,0,sizeof(m_lpUrlComponents));
		m_lpUrlComponents.dwStructSize = sizeof(m_lpUrlComponents);
		m_lpUrlComponents.lpszExtraInfo = m_URLInfo.szExtraInfo;
		m_lpUrlComponents.lpszHostName = m_URLInfo.szHostName;
		m_lpUrlComponents.lpszPassword = m_URLInfo.szPassword;
		m_lpUrlComponents.lpszScheme = m_URLInfo.szScheme;
		m_lpUrlComponents.lpszUrlPath = m_URLInfo.szUrlPath;
		m_lpUrlComponents.lpszUserName = m_URLInfo.szUserName;

		m_lpUrlComponents.dwExtraInfoLength = 
			m_lpUrlComponents.dwHostNameLength = 
			m_lpUrlComponents.dwPasswordLength = 
			m_lpUrlComponents.dwSchemeLength = 
			m_lpUrlComponents.dwUrlPathLength = 
			m_lpUrlComponents.dwUserNameLength = 512;
		m_eRequestType = requestType;
		if (!WinHttpCrackUrl(String(URL).toStdWString().c_str(), 0, ICU_ESCAPE, &m_lpUrlComponents))
		{
			return false;
		}
		else
		{
			m_hSession = WinHttpOpen(String(m_strUserAgent).toStdWString().c_str(),
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				0);
			if (!m_hSession)
			{
				LOG_ERROR("WinHttpOpen error(%d)\r\n", GetLastError());
				return false;
			}
			
			if (FALSE == WinHttpSetTimeouts(m_hSession, m_nResolveTimeout, m_nConnectTimeout, m_nSendTimeout, m_nReceiveTimeout)){
				LOG_ERROR("WinHttpSetTimeouts error(%d)\r\n", GetLastError());
				return false;
			}

			m_hConnection = WinHttpConnect(m_hSession, m_lpUrlComponents.lpszHostName, m_lpUrlComponents.nPort, 0);
			if (NULL == m_hConnection){
				LOG_ERROR("WinHttpConnect error(%d)\r\n", GetLastError());
				return false;
			}

			std::wstring wstrRequestType;
			if (m_eRequestType == RequestTypePOST)
			{
				//GET时忽略content
				wstrRequestType = L"POST";
			}else{
				wstrRequestType = L"GET";
			}
			DWORD flags = m_lpUrlComponents.nPort == 443 ? WINHTTP_FLAG_SECURE : 0;
			m_hRequest = WinHttpOpenRequest(m_hConnection,wstrRequestType.c_str(), 
				m_lpUrlComponents.lpszUrlPath, 
				TEXT("HTTP/1.1"), 
				WINHTTP_NO_REFERER, 
				WINHTTP_DEFAULT_ACCEPT_TYPES, 
				flags);

			if (NULL == m_hRequest){
				LOG_ERROR("WinHttpOpenRequest error(%d)\r\n", GetLastError());
				return false;
			}
		}
		return true;
	}

	bool CWinHttpW::send( const string& content /*= ""*/ )
	{
		// TODO 通过抓包，在这里测试发送的HTTP请求是否带有请求参数，某些POST请求会带上参数如：?a=1&b=2
		// 很明显这里是不会添加这些参数的。
		// 参照例子http://blog.csdn.net/breaksoftware/article/details/17232483
		if (!WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, content.length(), 0))
		{
			LOG_ERROR("WinHttpSendRequest error(%d)\r\n", GetLastError());
			return false;
		}
		DWORD dwBytesWritten = 0;
		if (!WinHttpWriteData(m_hRequest,content.data(),content.length(),&dwBytesWritten))
		{
			LOG_ERROR("WinHttpWriteData error(%d)\r\n", GetLastError());
			return false;
		}
		if (!WinHttpReceiveResponse( m_hRequest, NULL))
		{
			LOG_ERROR("WinHttpReceiveResponse error(%d)\r\n", GetLastError());
			return false;
		}
		return true;
	}

	bool CWinHttpW::setReferer( const string& refererValue )
	{
		return setRequestHeader("Referer",refererValue);
	}

	bool CWinHttpW::setAccept( const string& acceptType )
	{
		return setRequestHeader("Accept",acceptType);
	}

	bool CWinHttpW::setAcceptLanguage( const string& acceptLanguageValue /*= "zh-cn"*/ )
	{
		return setRequestHeader("Accept-Language",acceptLanguageValue);
	}

	bool CWinHttpW::setContentType( const string& contentTypeValue /*= "application/x-www-form-urlencoded"*/ )
	{
		return setRequestHeader("Content-Type",contentTypeValue);
	}

	bool CWinHttpW::setXMLHttpRequest()
	{
		return setRequestHeader("X-Requested-With","XMLHttpRequest");
	}

	bool CWinHttpW::delRequestHeader( const string& header )
	{
		LOG_FATAL("delRequestHeader no impelitement");
		return true;
	}

	bool CWinHttpW::setRequestHeader( const string& header, const string& value )
	{
		if (value == "" || header == "")
		{
			LOG_ERROR("setRequestHeader error(参数不可以为空)");
			return false;
		}
		string HeadString = header;
		HeadString += ": ";
		HeadString += value;
		HeadString += "\r\n";
		if (!WinHttpAddRequestHeaders(m_hRequest,String(HeadString).toStdWString().c_str(),-1L,WINHTTP_ADDREQ_FLAG_ADD))
		{
			LOG_ERROR("WinHttpOpenRequest error(%d)\r\n", GetLastError());
			return false;
		}
		return true;
	}

}