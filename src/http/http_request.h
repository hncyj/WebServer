/**
 * @file http_request.h
 * @author chenyinjie
 * @date 2024-09-12
 */

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../pool/sql_connect_pool.h"
#include "../pool/sql_connect_pool_RAII.h"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <regex>
#include <mysql/mysql.h>
#include <errno.h>

class HTTPRequest{
public:
    enum PARSE_STATE {REQUEST_LINE, HEADERS, BODY, FINISH};
    enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSE_CONNECTION};

    HTTPRequest();
    ~HTTPRequest() = default;

    void Init();
    bool Parse(Buffer& buffer);

    std::string GetPath() const;
    std::string& GetPath();
    std::string GetMethod() const;
    std::string GetVersion() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);
    void ParsePath();
    void ParsePost();
    void ParseFromUrlEncoded();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool is_login);
    static int ConvertHex(char ch);

    PARSE_STATE state_;
    std::string method_;
    std::string path_;
    std::string version_;
    std::string body_;

    bool is_log_open;
    
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};

#endif
