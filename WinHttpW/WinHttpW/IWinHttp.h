/**
 * \file IWinHttp.h
 * \brief ����IWinHttp�࣬�����ʹ��ͬ����ʽ�շ�HTTP�������������е��������WinHttpW�÷�������д������ӿڶ���
 * 
 * �ں�����չ����ӽ����ƽ̨���ԡ�����Ƽ�ʹ�����ͷ�ļ����ṩ�Ļ�ȡ�ӿڶ���Ľӿڣ���ò�Ҫֱ��ʹ���������ࡣ
 */
#pragma once

#include <string>
using std::string;

namespace network
{

enum ERequestType
{
	RequestTypeUnkown = -1,
	RequestTypeGET,
	RequestTypePOST
};
/**
 * \file IWinHttp.h
 * \brief ���������WinHttp���ݽӿ�
 */
class IWinHttp
{
public:
	virtual ~IWinHttp() {;}
public:
	/**
	 * ����(��ʼ��)WinHttp,�ͷ���������,����֮ǰ�������ú�Cookies.
	 * \param userAgent �Զ����û������Э��ͷ��Ϣ,User_Agent ������ʹ��Ĭ�ϵ��������Ϣ
	 */
	virtual void rest( const string& userAgent) = 0;
	/**
	 * ��������User-Agent�������Ϣ ��������Open��ʹ�ã�
	 * \param UserAgentValue �ɿ�, ����Ϊ��Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2) 
	 */
	virtual void setUserAgent(const string& userAgentValue = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2)") = 0;
	/**
	 * ����Cookies�Զ�����ģʽ �����ú�һֱ��Ч�� ��������Openǰʹ�ã�
	 * \param Index ����=1
	 * -# 
	 * 0=Winhttp�ڲ��Զ�����Cookies(��ȫ��Winhttpr5.1һ��������)  
	 * 1=���ڲ��Զ��ϲ�����Cookies(��ģʽ�¿�����϶��з���Cookies CookieAdd CookieSet CookieDel ��ȡ/��/���/����/ɾ��)  
	 * 2=���浽����(֧��IES�ؼ��̳� ע��:ģʽ2�����ݶ��߳�)
	 */
	virtual void autoCookies(int isAutoCookies) = 0;
	/** 
	 * ��һ��HTTP����
	 * \param Method, ERequestType֮һ��GET����POST;
	 * \param Url,����http://www.nixsw.com/��֧��HTTPS
	 */
	virtual bool open(ERequestType requestType,const string& URL) = 0;
	/**
	 * ��������,Ĭ������Ϊ�ա������跢������ʱ���󲿷�����µ�GET����������µ�POST��ʹ��Ĭ�ϲ������ɡ�
	 * ����ķ�������Ӧ��Ҫ����getStatus�鿴���������ص�HTTP״ֵ̬��
	 */
	virtual bool send(const string& content = "") = 0;
	/** 
	 * ���û�ȡ������ �����ú�һֱ��Ч��
	 * \param Proxy, �ı���, �ɿ�, ��=ȡ������  HTTP����="http=127.0.0.1:8080"  HTTPS����=��127.0.0.1:8080��
	 * \param User, �ı���, �ɿ�, �����ʺ�
	 * \param Pass, �ı���, �ɿ�, ��������
	 */
	virtual bool setProxy(const string& proxy, const string& user, const string& pass) = 0;
	/**
	 * ָ����ʱ���� �����ú�һֱ��Ч�� ��Ҫ�뱾�η��ʾ���Ч ����Openǰ���ã�
	 * \param ResolveTimeout, ������, �ɿ�, ������ʱ����λ���� ����=10000(10��)
	 * \param ConnectTimeout, ������, �ɿ�, ���ӳ�ʱ����λ���� ����=15000(15��)
	 * \param SendTimeout, ������, �ɿ�, ���ͳ�ʱ����λ���� ����=120000(120��/2����)
	 * \param ReceiveTimeout, ������, �ɿ�, ���ճ�ʱ����λ���� ����=300000(300��/5����) ��ע�⣺������ ���ճ�ʱҲ��Ӱ�쵽POST�ϴ���ʱ��
	 */
	virtual void setTimeouts(int resolveTimeout = 10000, int connectTimeout=15000, int sendTimeout = 120000, int receiveTimeout = 300000) = 0;
	//
	// Headers
	//
	/**
	 * ���ÿͻ����ܽ��ܵ��������ͣ��磺"application/x-ms-application"
	 */
	virtual bool setAccept(const string& acceptType) = 0;
	/**
	 * ��������SetAccept-Language��Ϣ ��������Open��ʹ�ã�
	 * \param AcceptLanguageValue �ɿ�, ����Ϊ��zh-cn 
	 */
	virtual bool setAcceptLanguage(const string& acceptLanguageValue = "zh-cn") = 0;
	/**
	 * ��������Content-Type��Ϣ ��������Open��ʹ�ã�
	 * \param ContentTypeValue �ɿ�, ����Ϊ��application/x-www-form-urlencoded
	 */
	virtual bool setContentType(const string& contentTypeValue = "application/x-www-form-urlencoded") = 0;
	/**
	 * ��������Referer��·��Ϣ ��������Open��ʹ�ã�
	 * \param RefererValue �ɿ�, ����Ϊ�����η��ʵ�Url
	 */
	virtual bool setReferer(const string& refererValue);
	/**
	 * ��ӻ��滻HTTPЭ��ͷ �����η�����Ч�� ��������Open��ʹ�ã�
	 * \param Header �ı���, , Э��ͷ����
	 * \param Value �ı���, , Э��ͷֵ ���ı�Ϊɾ����Э��ͷ
	 */
	virtual bool setRequestHeader(const string& header, const string& value) = 0;
	/**
	 * ��������x-requested-with Ϊ XMLHttpRequest ��������Open��ʹ�ã�
	 */
	virtual bool setXMLHttpRequest() = 0;
	/** 
	 * ɾ��HTTPЭ��ͷ �����η�����Ч�� ��������Open��ʹ�ã�
	 * \param Header Э��ͷ����
	 */
	virtual bool delRequestHeader(const string& header) = 0;
	//
	// Reponds
	//
	/** 
	 * ��ȡHTTP���ص�״ֵ̬ (�磺200 = �ɹ�)
	 */
	virtual int getStatus() = 0;
	/** 
	 * ��ȡ���������ص����ݣ�ԭʼ���ݣ�
	 * \remark function return the byte array in a HTTP response,it can be a utf-8 string (and of course most of time it is), 
	 * but it also can be a binary data array (such as a picture,some file or encryped data).
	 */
	virtual string getRespondBody() = 0;
	/** 
	 * ȡ��HTTP����Э��ͷ ��������Send��ʹ�ã�
	 * \param Name Cookie����
	 */ 
	virtual string getRespondHeader(const string& header) = 0;
	/** 
	 * ȡ��HTTP����Э��ͷ��ָ��Cookie����ֵ ��������Send��ʹ�ã�
	 * \param Name Cookie����
	 */
	virtual string getRespondCookies(const string& name) = 0;
	/**
	 * ȡ������HTTP����Э��ͷ ��������Send��ʹ�ã�
	 * Receives all the headers returned by the server. Each header is separated by a carriage return/line feed (CR/LF) sequence.
	 */
	virtual string GetAllResponseHeaders() = 0;
	/** 
	 * ȡ��HTTP����Э��ͷ������COOKIES ��������Send��ʹ�ã�
	 */
	virtual string GetAllResponseCookies() = 0;
	/**
	 * ȡ�����ı� ���Զ�Utf8תAnsi�� ��������Send��ʹ�ã�
	 */
	virtual string GetResponseTextUtf8ToAnsi() = 0; 
	//
	//cookies
	//
	/**
	 * CookieAdd, ��ӻ��滻Cookie���ڲ�Cookies ��AutoCookiesģʽ1����Ч��
	 * ���� Cookie, ��������Cookie �����磺abc=123 ���� abc=123; def=456;g=nima��
	 */
	virtual bool cookieAdd( const string& cookies ) = 0;
	/**
	 * CookieDel, ɾ���ڲ�Cookies�е�Cookie ��AutoCookiesģʽ1����Ч��
	 * \param Cookie ��������Cookie��Cookie���� �����磺abc ���� abc=123 ���� ���� abc; def;g ���� abc=123; def=456;g=nima;id��
	 */
	virtual bool cookieDel( const string& cookies) = 0;
	/** 
	 * CookieGet, ��ȡ�ڲ�Cookies�е�ĳ��Cookie ��AutoCookiesģʽ1����Ч��
	 * \param Name Cookie����
	 * \param isCarryName �Ƿ��Cookie���Ƹ����Ϸ���
	 */
	virtual string cookieGet(const string& name, bool isCarryName=false) = 0;
	/**
	 * SetCookie, ��������Cookie��Ϣ ��AutoCookiesģʽ0�� ������Open��ʹ�ã�
	 * \parma Cookie cookies��Ϣ
	 */
	virtual string setCookie(const string& Cookie);
protected:
private:
};

	/**
	 * ��ȡIWinHttp��ʵ�ֶ���
	 */
	IWinHttp* getIWInstance();
	/**
	 * ʹ����Ϻ������������
	 */
	void destoryIWInstance(IWinHttp* pInst);
};