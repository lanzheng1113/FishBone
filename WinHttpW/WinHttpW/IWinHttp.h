/**
 * \file IWinHttp.h
 * \brief 定义IWinHttp类，这个类使用同步方式收发HTTP请求。受易语言中的鱼刺类中WinHttpW用法启发改写了这个接口对象。
 * 
 * 在后期拓展会添加进入多平台特性。因此推荐使用这个头文件中提供的获取接口对象的接口，最好不要直接使用其派生类。
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
 * \brief 定义鱼刺类WinHttp兼容接口
 */
class IWinHttp
{
public:
	virtual ~IWinHttp() {;}
public:
	/**
	 * 重置(初始化)WinHttp,释放所有数据,包括之前所有设置和Cookies.
	 * \param userAgent 自定义用户浏览器协议头信息,User_Agent 留空则使用默认的浏览器信息
	 */
	virtual void rest( const string& userAgent) = 0;
	/**
	 * 快速设置User-Agent浏览器信息 （必须在Open后使用）
	 * \param UserAgentValue 可空, 留空为：Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2) 
	 */
	virtual void setUserAgent(const string& userAgentValue = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2)") = 0;
	/**
	 * 设置Cookies自动管理模式 （设置后一直有效） （必须在Open前使用）
	 * \param Index 留空=1
	 * -# 
	 * 0=Winhttp内部自动处理Cookies(完全和Winhttpr5.1一样的特性)  
	 * 1=类内部自动合并更新Cookies(此模式下可以配合独有方法Cookies CookieAdd CookieSet CookieDel 来取/设/添加/设置/删除)  
	 * 2=缓存到本地(支持IES控件继承 注意:模式2不兼容多线程)
	 */
	virtual void autoCookies(int isAutoCookies) = 0;
	/** 
	 * 打开一个HTTP连接
	 * \param Method, ERequestType之一，GET或者POST;
	 * \param Url,例：http://www.nixsw.com/。支持HTTPS
	 */
	virtual bool open(ERequestType requestType,const string& URL) = 0;
	/**
	 * 发送数据,默认内容为空。当无需发送数据时（大部分情况下的GET，少数情况下的POST）使用默认参数即可。
	 * 具体的服务器响应需要调用getStatus查看服务器返回的HTTP状态值。
	 */
	virtual bool send(const string& content = "") = 0;
	/** 
	 * 设置或取消代理 （设置后一直有效）
	 * \param Proxy, 文本型, 可空, 空=取消代理  HTTP代理="http=127.0.0.1:8080"  HTTPS代理=“127.0.0.1:8080”
	 * \param User, 文本型, 可空, 代理帐号
	 * \param Pass, 文本型, 可空, 代理密码
	 */
	virtual bool setProxy(const string& proxy, const string& user, const string& pass) = 0;
	/**
	 * 指定超时设置 （设置后一直有效） （要想本次访问就生效 请在Open前调用）
	 * \param ResolveTimeout, 整数型, 可空, 解析超时，单位毫秒 留空=10000(10秒)
	 * \param ConnectTimeout, 整数型, 可空, 连接超时，单位毫秒 留空=15000(15秒)
	 * \param SendTimeout, 整数型, 可空, 发送超时，单位毫秒 留空=120000(120秒/2分钟)
	 * \param ReceiveTimeout, 整数型, 可空, 接收超时，单位毫秒 留空=300000(300秒/5分钟) （注意：经测试 接收超时也会影响到POST上传超时）
	 */
	virtual void setTimeouts(int resolveTimeout = 10000, int connectTimeout=15000, int sendTimeout = 120000, int receiveTimeout = 300000) = 0;
	//
	// Headers
	//
	/**
	 * 设置客户端能接受的内容类型，如："application/x-ms-application"
	 */
	virtual bool setAccept(const string& acceptType) = 0;
	/**
	 * 快速设置SetAccept-Language信息 （必须在Open后使用）
	 * \param AcceptLanguageValue 可空, 留空为：zh-cn 
	 */
	virtual bool setAcceptLanguage(const string& acceptLanguageValue = "zh-cn") = 0;
	/**
	 * 快速设置Content-Type信息 （必须在Open后使用）
	 * \param ContentTypeValue 可空, 留空为：application/x-www-form-urlencoded
	 */
	virtual bool setContentType(const string& contentTypeValue = "application/x-www-form-urlencoded") = 0;
	/**
	 * 快速设置Referer来路信息 （必须在Open后使用）
	 * \param RefererValue 可空, 留空为：本次访问的Url
	 */
	virtual bool setReferer(const string& refererValue);
	/**
	 * 添加或替换HTTP协议头 （本次访问有效） （必须在Open后使用）
	 * \param Header 文本型, , 协议头名称
	 * \param Value 文本型, , 协议头值 空文本为删除此协议头
	 */
	virtual bool setRequestHeader(const string& header, const string& value) = 0;
	/**
	 * 快速设置x-requested-with 为 XMLHttpRequest （必须在Open后使用）
	 */
	virtual bool setXMLHttpRequest() = 0;
	/** 
	 * 删除HTTP协议头 （本次访问有效） （必须在Open后使用）
	 * \param Header 协议头名称
	 */
	virtual bool delRequestHeader(const string& header) = 0;
	//
	// Reponds
	//
	/** 
	 * 获取HTTP返回的状态值 (如：200 = 成功)
	 */
	virtual int getStatus() = 0;
	/** 
	 * 获取服务器返回的内容（原始数据）
	 * \remark function return the byte array in a HTTP response,it can be a utf-8 string (and of course most of time it is), 
	 * but it also can be a binary data array (such as a picture,some file or encryped data).
	 */
	virtual string getRespondBody() = 0;
	/** 
	 * 取得HTTP返回协议头 （必须在Send后使用）
	 * \param Name Cookie键名
	 */ 
	virtual string getRespondHeader(const string& header) = 0;
	/** 
	 * 取得HTTP返回协议头中指定Cookie键的值 （必须在Send后使用）
	 * \param Name Cookie键名
	 */
	virtual string getRespondCookies(const string& name) = 0;
	/**
	 * 取得所有HTTP返回协议头 （必须在Send后使用）
	 * Receives all the headers returned by the server. Each header is separated by a carriage return/line feed (CR/LF) sequence.
	 */
	virtual string GetAllResponseHeaders() = 0;
	/** 
	 * 取得HTTP返回协议头中所有COOKIES （必须在Send后使用）
	 */
	virtual string GetAllResponseCookies() = 0;
	/**
	 * 取返回文本 （自动Utf8转Ansi） （必须在Send后使用）
	 */
	virtual string GetResponseTextUtf8ToAnsi() = 0; 
	//
	//cookies
	//
	/**
	 * CookieAdd, 添加或替换Cookie到内部Cookies （AutoCookies模式1下有效）
	 * 参数 Cookie, 单个或多个Cookie （例如：abc=123 或是 abc=123; def=456;g=nima）
	 */
	virtual bool cookieAdd( const string& cookies ) = 0;
	/**
	 * CookieDel, 删除内部Cookies中的Cookie （AutoCookies模式1下有效）
	 * \param Cookie 单个或多个Cookie或Cookie名称 （例如：abc 或是 abc=123 或是 或是 abc; def;g 或是 abc=123; def=456;g=nima;id）
	 */
	virtual bool cookieDel( const string& cookies) = 0;
	/** 
	 * CookieGet, 获取内部Cookies中的某个Cookie （AutoCookies模式1下有效）
	 * \param Name Cookie名称
	 * \param isCarryName 是否把Cookie名称附带上返回
	 */
	virtual string cookieGet(const string& name, bool isCarryName=false) = 0;
	/**
	 * SetCookie, 快速设置Cookie信息 （AutoCookies模式0下 必须在Open后使用）
	 * \parma Cookie cookies信息
	 */
	virtual string setCookie(const string& Cookie);
protected:
private:
};

	/**
	 * 获取IWinHttp的实现对象
	 */
	IWinHttp* getIWInstance();
	/**
	 * 使用完毕后，销毁这个对象。
	 */
	void destoryIWInstance(IWinHttp* pInst);
};