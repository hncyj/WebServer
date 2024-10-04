/**
 * @file http_request.h
 * @author chenyinjie
 * @date 2024-09-12
 * @copyright Apache 2.0
 */

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../pool/db_connect_pool.h"
#include "../pool/db_connect_pool_RAII.h"

#include <unordered_set>
#include <unordered_map>
#include <regex>
#include <errno.h>

class HTTPRequest{
public:
    /**
     * @brief 
     * HTTP Request 解析，包括请求行、首部行、请求体和结束状态。
     */
    enum PARSE_STATE {REQUEST_LINE, HEADER, BODY, FINISH};

    HTTPRequest();
    ~HTTPRequest() = default;

    void Init();
    bool Parse(Buffer& buffer);                                         // 从缓冲区中解析

    std::string& GetPath();                                             // 返回请求路径
    std::string GetPath() const;                                        // 重载版本

    std::string GetMethod() const;                                      // 请求方法
    std::string GetVersion() const;                                     // HTTP协议版本
    std::string GetPost(const std::string& key) const;                  // 返回post方法中指定键的值
    std::string GetPost(const char* key) const;                         // 重载版本    

    bool IsKeepAlive() const;                                           // 是否维持长连接

private:
    bool ParseRequestLine(const std::string& line);                     // 解析请求行
    void ParseHeader(const std::string& line);                          // 解析首部行
    void ParseBody(const std::string& line);                            // 解析请求体
    void ParsePath();                                                   // 解析请求路径
    void ParsePost();                                                   // 解析post请求路径
    void ParseFromUrlEncoded();                                         // 处理url编码

    static bool UserVerify(const std::string& name, const std::string& pwd, bool is_login);
    static int ConvertHex(char ch);                                     // 将一个字符转换为十六进制数

    PARSE_STATE state_;                                                 // 解析状态 
    std::string method_;                                                // 请求方法
    std::string path_;                                                  // 请求路径
    std::string version_;                                               // 协议版本
    std::string body_;                                                  // 请求主体
    
    std::unordered_map<std::string, std::string> headers_;              // 请求头部信息
    std::unordered_map<std::string, std::string> posts_;                // post请求数据
    
    static const std::unordered_set<std::string> DEFAULT_HTML;          // 默认页面路径
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG; // HTML标签信息
};

#endif
