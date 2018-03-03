#pragma once
#include "IWinHttp.h"


namespace network
{
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
	virtual bool setUserAgent(const string& userAgentValue = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2)") override;
	virtual void autoCookies(int isAutoCookies) override;
	virtual bool open(ERequestType requestType,const string& URL) override;
	virtual bool send(const string& content = "") override;
	virtual bool setProxy(const string& proxy, const string& user, const string& pass) override;
	virtual bool setTimeouts(int resolveTimeout = 10000, int connectTimeout=15000, int sendTimeout = 120000, int receiveTimeout = 300000) override;
	
	virtual void setAccept(const string& acceptType) override;
	virtual bool setAcceptLanguage(const string& acceptLanguageValue = "zh-cn") override;
	virtual bool setContentType(const string& contentTypeValue = "application/x-www-form-urlencoded") override;
	virtual bool setReferer(const string& refererValue) override;
	virtual bool setRequestHeader(const string& header, const string& value) override;
	virtual bool setXMLHttpRequest() override;
	virtual bool delRequestHeader(const string& header) override;
	
	virtual int getStatus() override;
	virtual string getRespondBody() override;
	virtual string getRespondHeader(const string& name) override;
	virtual string getRespondCookies(const string& name) override;
	virtual string GetAllResponseHeaders() override;
	virtual string GetAllResponseCookies() override;
	virtual string GetResponseTextUtf8ToAnsi() override; 
	
	virtual bool cookieAdd( const string& cookies ) override;
	virtual bool cookieDel( const string& cookies ) override;
	virtual string cookieGet(const string& name, bool isCarryName=false) override;
	virtual string setCookie(const string& Cookie) override;
};

};
