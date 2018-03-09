#pragma once
#include "IWinHttp.h"
#include "winhttp.h"
#include <map>
using std::map;

namespace network
{

	typedef struct _URL_INFO
	{
		wchar_t szScheme[512];
		wchar_t szHostName[512];
		wchar_t szUserName[512];
		wchar_t szPassword[512];
		wchar_t szUrlPath[512];
		wchar_t szExtraInfo[512];
	}URL_INFO, *PURL_INFO;

/**
 * CWinHttpW
 * 与易语言的鱼刺类接口兼容的C++接口
 */
class CWinHttpW : public IWinHttp
{
public:
	CWinHttpW(void);
	~CWinHttpW(void);
public:
	virtual void rest( const string& userAgent) override;
	virtual void setUserAgent(const string& userAgentValue = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2)") override;
	virtual void autoCookies(int isAutoCookies) override;
	virtual bool open(ERequestType requestType,const string& URL) override;
	virtual bool send(const string& content = "") override;
	virtual bool setProxy(const string& proxy, const string& user, const string& pass) override;
	virtual void setTimeouts(int resolveTimeout = 10000, int connectTimeout = 15000, int sendTimeout = 120000, int receiveTimeout = 300000) override;
	
	virtual bool setAccept(const string& acceptType) override;
	virtual bool setAcceptLanguage(const string& acceptLanguageValue = "zh-cn") override;
	virtual bool setContentType(const string& contentTypeValue = "application/x-www-form-urlencoded") override;
	virtual bool setReferer(const string& refererValue) override;
	virtual bool setRequestHeader(const string& header, const string& value) override;
	virtual bool setXMLHttpRequest() override;
	virtual bool delRequestHeader(const string& header) override;
	
	virtual int getStatus() override;
	virtual string getRespondHeader(const string& header) override;
	virtual string GetAllResponseHeaders() override;
	virtual string getRespondBody() override;
	virtual string getRespondCookies(const string& name) override;
	virtual string GetAllResponseCookies() override;
	virtual string GetResponseTextUtf8ToAnsi() override; 
	
	virtual bool cookieAdd( const string& cookies ) override;
	virtual bool cookieDel( const string& cookies ) override;
	virtual string cookieGet(const string& name, bool isCarryName=false) override;
	virtual string setCookie(const string& Cookie) override;
private:
	string m_strUserAgent;
	int m_cookieStrategy;
	ERequestType m_eRequestType;
	URL_INFO m_URLInfo;
	URL_COMPONENTSW m_lpUrlComponents;
	HINTERNET m_hSession;
	HINTERNET m_hConnection;
	HINTERNET m_hRequest;
	int m_nResolveTimeout;
	int m_nConnectTimeout;
	int m_nSendTimeout;
	int m_nReceiveTimeout;
};

};
