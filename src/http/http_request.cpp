/**
 * @file http_request.cpp
 * @author chenyinjie
 * @date 2024-09-13
 */


// GET /images/logo.png HTTP/1.1    // 请求行：请求资源 /images/logo.png
// Host: www.example.com            // 目标主机 www.example.com
// Connection: keep-alive           // 保持连接
// User-Agent: Mozilla/5.0          // 客户端身份信息
// Accept: image/webp,*/*           // 客户端希望接受的资源类型（首选 WebP 格式的图片，但也接受任意格式）
// If-Modified-Since: Wed, 21 Oct 2023 07:28:00 GMT  // 条件请求：自该时间点后文件是否有修改


#include "http_request.h"

const std::unordered_set<std::string> HTTPRequest::DEFAULT_HTML {
    "/index", "/register", "login", "/welcome", "/video", "/picture"
};

const std::unordered_map<std::string, int> HTTPRequest::DEFAULT_HTML_TAG {
    {"/register.html", 0}, {"/login.html", 1}
};

HTTPRequest::HTTPRequest() {
    Init();
}

void HTTPRequest::Init() {
    state_ = REQUEST_LINE;
    method_ = path_ = version_ = body_ = "";
    is_log_open = Log::getInstance().isOpen();
    header_.clear();
    post_.clear();
}

bool HTTPRequest::Parse(Buffer& buffer) {
    if (buffer.ReadableLen() <= 0) {
        if (is_log_open) {
            LOG_ERROR("Buffer Readable Len Error.");
        } else {
            std::cerr << "Buffer Readable Len Error." << std::endl;
        }
        return false;
    };

    const char* CRLF = "\r\n";
    while (buffer.ReadableLen() > 0 && state_ != FINISH) {
        const char* end_of_line = std::search(buffer.ReadPtr(), buffer.WritePtr(), CRLF, CRLF + 2);
        std::string line(static_cast<const char*>(buffer.ReadPtr()), end_of_line);
        switch (state_) {
            case REQUEST_LINE:
                if (!ParseRequestLine(line)) {
                    return false;
                }
                ParsePath();
                break;

            case HEADERS:
                ParseHeader(line);
                if (buffer.ReadableLen() <= 2) {
                    state_ = FINISH;
                }
                break;

            case BODY:
                ParseBody(line);
                break;

            default:
                break;
        }
        if (end_of_line == buffer.WritePtr()) break;
        buffer.ReadToPtr(end_of_line);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

std::string HTTPRequest::GetPath() const {
    return path_;
}

std::string& HTTPRequest::GetPath() {
    return path_;
}

std::string HTTPRequest::GetMethod() const {
    return method_;
}

std::string HTTPRequest::GetVersion() const {
    return version_;
}

std::string HTTPRequest::GetPost(const std::string& key) const {
    assert(!key.empty());
    if (post_.count(key)) return post_.find(key)->second;
    return "";
}

std::string HTTPRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if (post_.count(key)) return post_.find(key)->second;
    return "";
}

bool HTTPRequest::IsKeepAlive() {
    if (header_.count("Connection")) return header_.find("Connection")->second == "Keep-alive" && version_ == "1.1";
    return false;
}

bool HTTPRequest::ParseRequestLine(const std::string& line) {
    // parse request line
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch parse_result;

    if (std::regex_match(line, parse_result, pattern)) {
        method_ = parse_result[1];
        path_ = parse_result[2];
        version_ = parse_result[3];
        state_ = HEADERS;
        return true;
    }

    if (is_log_open) {
        LOG_ERROR("Request Line Parse Error");
    } else {
        std::cerr << "Request Line Parse Error" << std::endl;
    }
    
    return false;
}

void HTTPRequest::ParseHeader(const std::string& line) {
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch parse_result;
    if (std::regex_match(line, parse_result, pattern)) {
        header_[parse_result[1]] = parse_result[2];
    } else {
        state_ = BODY;
    }
}

void HTTPRequest::ParseBody(const std::string& line) {
    body_ = line;
    ParsePost();
    state_ = FINISH;
    if (is_log_open) {
        LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
    }
}

void HTTPRequest::ParsePath() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (auto& element : DEFAULT_HTML) {
            if (element == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HTTPRequest::ParsePost() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlEncoded();
        if (DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            if (is_log_open) {
                LOG_DEBUG("Tag:%d", tag);
            }
            if (tag == 0 || tag == 1) {
                bool is_login = (tag == 1);
                if (UserVerify(post_["username"], post_["password"], is_login)) {
                    path_ = "/welcome.html";
                } else {
                    path_ = "/error.html";
                }
            }
        }
    }
}

void HTTPRequest::ParseFromUrlEncoded() {
    if (body_.size() == 0) return;

    std::string key, val;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;
    
    for (; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
        case '=':
            key = body_.substr(j, i - 1);
            j = i + 1;
            break;

        case '+':
            body_[i] = ' ';
            break;

        case '%':
            num = ConvertHex(body_[i + 1]) * 16 + ConvertHex(body_[i + 2]);
            body_[i + 2] = (num % 10 + '0');
            body_[i + 1] = (num / 10 + '0');
            i += 2;
            break;

        case '&':
            val = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = val;
            if (is_log_open) {
                LOG_DEBUG("%s = %s", key.c_str(), val.c_str());
            }
            break;

        default:
            break;
        }
    }

    assert(j <= i);
    if (!post_.count(key) && j < i) {
        val = body_.substr(j, i - j);
        post_[key] = val;
    }
}

bool HTTPRequest::UserVerify(const std::string& name, const std::string& pwd, bool is_login) {
    if(name == "" || pwd == "") return false;

    // TODO: 验证日志系统开启
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());

    MYSQL* sql;
    SQLConnectPoolRAII(&sql, SQLConnectPool::GetInstance());
    assert(sql);

    bool flag = false;
    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;

    if(!is_login) flag = true;
    /* 查询用户及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    // TODO
    LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) { 
        mysql_free_result(res);
        return false; 
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string password(row[1]);
        /* 注册用户 且 用户名未被使用*/
        if(is_login) {
            if (pwd == password) {
                flag = true; 
            }
            else {
                flag = false;
                //TODO
                LOG_DEBUG("pwd error!");
            }
        } 
        else { 
            flag = false; 
            //TODO
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    /* 注册用户 且 用户名未被使用*/
    if(!is_login && flag == true) {
        // TODO
        LOG_DEBUG("regirster!");
        memset(order, 0, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        // TODO
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }

    SQLConnectPool::GetInstance()->FreeConnect(sql);
    //TODO
    LOG_DEBUG( "UserVerify success!!");

    return flag;
}

int HTTPRequest::ConvertHex(char ch) {
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return ch;
}
