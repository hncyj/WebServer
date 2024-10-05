/**
 * @file http_request.cpp
 * @author chenyinjie
 * @date 2024-09-13
 * @copyright Apache 2.0
 */


// GET /images/logo.png HTTP/1.1    // 请求行：请求资源 /images/logo.png
// Host: www.example.com            // 目标主机 www.example.com
// Connection: keep-alive           // 长连接
// User-Agent: Mozilla/5.0          // 客户端身份信息
// Accept: image/webp,*/*           // 客户端希望接受的资源类型（首选 WebP 格式的图片，但也接受任意格式）
// If-Modified-Since: Wed, 21 Oct 2023 07:28:00 GMT  // 条件请求：自该时间点后文件是否有修改


#include "http_request.h"

#include <openssl/sha.h>

static const size_t SIZE = 256;

const std::unordered_set<std::string> HTTPRequest::DEFAULT_HTML {
    "/index", "/register", "/login", "/welcome", "/video", "/picture"
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
    headers_.clear();
    posts_.clear();
}

bool HTTPRequest::Parse(Buffer& buffer) {
    if (buffer.ReadableLen() <= 0) {
        LOG_ERROR("HTTP Request: Buffer Readable Length Error.");
        return false;
    };

    const char CRLF[] = "\r\n";
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

            case HEADER:
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
        buffer.ReadUntil(end_of_line + 2);
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
    if (posts_.count(key)) {
        return posts_.find(key)->second;
    }
    LOG_ERROR("HTTP Request: Can not find key: %s", key.c_str());
    return "";
}

std::string HTTPRequest::GetPost(const char* key) const {
    if (posts_.count(key)) {
        return posts_.find(key)->second;
    }
    LOG_ERROR("HTTP Request: Can not find key: %s", key);
    return "";
}

bool HTTPRequest::IsKeepAlive() const {
    return headers_.count("Connection") && headers_.find("Connection")->second == "keep-alive" && version_ == "1.1";
}

bool HTTPRequest::ParseRequestLine(const std::string& line) {
    // parse request line
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch parse_result;

    if (std::regex_match(line, parse_result, pattern)) {
        method_ = parse_result[1];
        path_ = parse_result[2];
        version_ = parse_result[3];
        state_ = HEADER;
        return true;
    }

    LOG_ERROR("Request Line Parse Error");
    return false;
}

void HTTPRequest::ParseHeader(const std::string& line) {
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch parse_result;
    if (std::regex_match(line, parse_result, pattern)) {
        headers_[parse_result[1]] = parse_result[2];
    } else {
        state_ = BODY;
    }
}

void HTTPRequest::ParseBody(const std::string& line) {
    body_ = line;
    ParsePost();
    state_ = FINISH;
    LOG_DEBUG("Parse HTTP Request Body, len size: %d", line.size());
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
    if (method_ == "POST" && headers_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlEncoded();
        if (DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("HTTP Request Path Tag:%d", tag);
            if (tag == 0 || tag == 1) {
                bool is_login = (tag == 1);
                if (UserVerify(posts_["username"], posts_["password"], is_login)) {
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
    int left = 0, right = 0;
    
    for (; right < n; right++) {
        char ch = body_[right];
        int low = -1, high = -1;
        switch (ch) {
            case '=':
                key = body_.substr(left, right - left);
                left = right + 1;
                break;

            case '+':
                body_[right] = ' ';
                break;

            case '%':
                if (right + 2 >= n) {
                    LOG_ERROR("HTTP Request: Percent decoding index out of range.");
                    return; // 或者采取其他适当的错误处理措施   
                }
                high = ConvertHex(body_[right + 1]);
                low = ConvertHex(body_[right + 2]);
                if (high == -1 || low == -1) {
                    LOG_ERROR("HTTP Request: Invalid percent encoding.");
                    return; // 或者采取其他适当的错误处理措施
                }
                num = high * 16 + low;
                body_[right] = static_cast<char>(num);
                right += 2;
                break;

            case '&':
                val = body_.substr(left, right - left);
                left = right + 1;
                posts_[key] = val;
                LOG_DEBUG("ParseFromUrlEncoded: Key: %s", key.c_str());
                break;

            default:
                break;
        }
    }

    if (left <= right) {
        val = body_.substr(left, right - left);
        posts_[key] = val;
        LOG_DEBUG("ParseFromUrlEncoded: Key: %s", key.c_str());
    }
}

/**
 * @brief 
 * 用户验证逻辑
 * - 登录逻辑
 * - 注册逻辑
 * 
 * @param name 
 * @param pwd 
 * @param is_login 
 * @return true 
 * @return false 
 */
bool HTTPRequest::UserVerify(const std::string& name, const std::string& pwd, bool is_login) {
    if (name == "" || pwd == "") return false;
    LOG_INFO("UserVerify: name: %s", name.c_str());

    MYSQL* sql;
    SQLConnectPoolRAII(&sql, SQLConnectPool::GetSQLConnectPoolInstance());
    if (sql == nullptr) {
        LOG_ERROR("Get SQL connection failed.");
        return false;
    }

    bool flag = false;
    MYSQL_STMT* stmt = nullptr;
    MYSQL_BIND bind[2];
    MYSQL_BIND result_bind[1];
    char hashed_pwd[65] = {0};    // 存储密码的哈希结果

    // 对输入密码进行哈希处理
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)pwd.c_str(), pwd.length(), hash);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(hashed_pwd + (i * 2), "%02x", hash[i]);
    }

    if (is_login) {
        const char* query = "SELECT password FROM user WHERE username=? LIMIT 1";
        stmt = mysql_stmt_init(sql);
        if (!stmt) {
            LOG_ERROR("mysql_stmt_init failed.");
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            LOG_ERROR("mysql_stmt_prepare failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }
        
        // 绑定用户名参数
        memset(bind, 0, sizeof(bind));
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(name.c_str());
        bind[0].buffer_length = name.length();

        if (mysql_stmt_bind_param(stmt, bind)) {
            LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt)) {
            LOG_ERROR("mysql_stmt_execute failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 绑定查询结果
        memset(result_bind, 0, sizeof(result_bind));
        char db_pwd[256] = {0};
        unsigned long length = 0;
        result_bind[0].buffer_type = MYSQL_TYPE_STRING;
        result_bind[0].buffer = db_pwd;
        result_bind[0].buffer_length = sizeof(db_pwd);
        result_bind[0].length = &length;

        if (mysql_stmt_bind_result(stmt, result_bind)) {
            LOG_ERROR("mysql_stmt_bind_result failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_fetch(stmt) == 0) {
            if (strcmp(hashed_pwd, db_pwd) == 0) {
                flag = true;
                LOG_INFO("User Login sucessful.");
            } else {
                LOG_WARN("User password incorrect.");
            }
        } else {
            LOG_WARN("User not found.");
        }
        mysql_stmt_close(stmt);
    } else {
        // 注册用户
        const char* check_query = "SELECT username FROM user WHERE username=? LIMIT 1";
        stmt = mysql_stmt_init(sql);
        if (!stmt) {
            LOG_ERROR("mysql_stmt_init failed!");
            return false;
        }

        if (mysql_stmt_prepare(stmt, check_query, strlen(check_query))) {
            LOG_ERROR("mysql_stmt_prepare failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        memset(bind, 0, sizeof(bind));
        // 绑定用户名参数
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(name.c_str());
        bind[0].buffer_length = name.length();

        if (mysql_stmt_bind_param(stmt, bind)) {
            LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 执行查询
        if (mysql_stmt_execute(stmt)) {
            LOG_ERROR("mysql_stmt_execute failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 绑定结果
        memset(result_bind, 0, sizeof(result_bind));
        char db_username[50] = {0};
        unsigned long length = 0;

        result_bind[0].buffer_type = MYSQL_TYPE_STRING;
        result_bind[0].buffer = db_username;
        result_bind[0].buffer_length = sizeof(db_username);
        result_bind[0].length = &length;

        if (mysql_stmt_bind_result(stmt, result_bind)) {
            LOG_ERROR("mysql_stmt_bind_result failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 检查是否有结果
        if (mysql_stmt_fetch(stmt) == 0) {
            // 用户名已存在，不能注册
            LOG_DEBUG("Username: %s already exists!", name);
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);

        // 插入新用户
        const char* insert_query = "INSERT INTO user(username, password) VALUES(?, ?)";
        stmt = mysql_stmt_init(sql);
        if (!stmt) {
            LOG_ERROR("mysql_stmt_init failed!");
            return false;
        }

        if (mysql_stmt_prepare(stmt, insert_query, strlen(insert_query))) {
            LOG_ERROR("mysql_stmt_prepare failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        memset(bind, 0, sizeof(bind));
        // 绑定用户名参数
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)name.c_str();
        bind[0].buffer_length = name.length();

        // 绑定密码参数
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = hashed_pwd;
        bind[1].buffer_length = strlen(hashed_pwd);

        if (mysql_stmt_bind_param(stmt, bind)) {
            LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 执行插入
        if (mysql_stmt_execute(stmt)) {
            LOG_ERROR("mysql_stmt_execute failed: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        flag = true;
        LOG_DEBUG("User registered successfully!");
    }

    return flag;
}

int HTTPRequest::ConvertHex(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}