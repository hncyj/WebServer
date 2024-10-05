/**
 * @file http_response.cpp
 * @author chenyinjie
 * @date 2024-09-22 
 * @copyright Apache 2.0
 */

// HTTP/1.1 200 OK                                // 状态行
// Connection: close                              // 首部行
// Date: Tue, 18 Aug 2015 15:44:04 GMT            // 首部行
// Server: Apache/2.2.3 (CentOS)                  // 首部行
// Last-Modified: Tue, 18 Aug 2015 15:11:03 GMT   // 首部行
// Content-Length: 6821                           // 首部行
// Content-Type: text/html                        // 首部哈
// \r\n
// (data data data ...)                           // 实体

#include "http_response.h"

using namespace std;

const unordered_map<string, string> HTTPResponse::SUFFIX_TYPE_ = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css" },
    { ".js",    "text/javascript" },
};

const unordered_map<int, string> HTTPResponse::CODE_STATUS_ = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> HTTPResponse::CODE_PATH_ = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HTTPResponse::HTTPResponse(): code_(-1), 
                              is_keep_alive_(false), 
                              path_(""), 
                              src_dir_(""), 
                              mmfile_ptr_(nullptr), 
                              mmfile_stat_{0} {}

HTTPResponse::~HTTPResponse() {
    UnmapFilePtr();
}

void HTTPResponse::Init(const std::string& src_dir, const std::string& path, bool is_keep_alive, int code) {
    if (src_dir.empty()) {
        LOG_ERROR("HTTP Response: source directroy path error.");
    }
    if (mmfile_ptr_) {
        UnmapFilePtr();
    }
    code_ = code;
    path_ = path;
    src_dir_ = src_dir;
    is_keep_alive_ = is_keep_alive;
    mmfile_ptr_ = nullptr;
    mmfile_stat_ = {0};
}

void HTTPResponse::GenerateResponse(Buffer& buffer) {
    std::string file_path = src_dir_ + path_;

    if (stat(file_path.data(), &mmfile_stat_) < 0 || S_ISDIR(mmfile_stat_.st_mode)) {
        code_ = 404;
    } else if (!(mmfile_stat_.st_mode & S_IROTH)) {
        code_ = 403;
    } else if (code_ == -1) {
        code_ = 200;
    }

    ErrorHtml();

    AddStateLine(buffer);
    AddHeader(buffer);

    if (code_ == 200) {
        AddContent(buffer);
    } else {
        ErrorContent(buffer, CODE_STATUS_.find(code_)->second);
    }
}

void HTTPResponse::UnmapFilePtr() {
    if (mmfile_ptr_) {
        munmap(mmfile_ptr_, mmfile_stat_.st_size);
        mmfile_ptr_ = nullptr;
    }
}

char* HTTPResponse::GetFilePtr() {
    return mmfile_ptr_;
}

size_t HTTPResponse::GetFileLen() const {
    return mmfile_stat_.st_size;
}

void HTTPResponse::ErrorContent(Buffer& buffer, std::string message) {
    std::string body;
    std::string status;

    if (CODE_STATUS_.count(code_)) {
        status = CODE_STATUS_.find(code_)->second;
    } else {
        status = "Bad Request";
    }

    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += to_string(code_) + ": " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>WebServer</em></body></html>";

    buffer.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
    buffer.Append("Content-Type: text/html\r\n");
    buffer.Append("Connection: close\r\n");
    buffer.Append("Content-Length: " + to_string(body.size()) + "\r\n");
    buffer.Append("\r\n");
    buffer.Append(body);
}

int HTTPResponse::GetCode() const {
    return code_;
}

void HTTPResponse::ErrorHtml() {
    if (CODE_PATH_.count(code_)) {
        path_ = CODE_PATH_.find(code_)->second;
        std::string file_path = src_dir_ + path_;
        if (stat(file_path.data(), &mmfile_stat_) < 0) {
            code_ = 404;
            path_ = "/404.html";
            stat((src_dir_ + path_).data(), &mmfile_stat_);
        }
    }
}

void HTTPResponse::AddStateLine(Buffer& buffer) {
    string status;
    if (CODE_STATUS_.count(code_)) {
        status = CODE_STATUS_.find(code_)->second;
    } else {
        code_ = 400;
        status = CODE_STATUS_.find(code_)->second;
    }
    buffer.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

void HTTPResponse::AddHeader(Buffer& buffer) {
    buffer.Append("Connection: ");
    if (is_keep_alive_) {
        buffer.Append("keep-alive\r\n");
        buffer.Append("Keep-Alive: max = 6, timeout=120\r\n");
    } else {
        buffer.Append("close\r\n");
    }
    buffer.Append("Date: " + GetCurrentTime() + "\r\n");
    // buffer.Append("Last-Modified: " + GetLastModifiedTime() + "\r\n");
    buffer.Append("Content-Type: " + GetFileType() + "\r\n");
}

void HTTPResponse::AddContent(Buffer& buffer) {
    // 打开目标文件
    int src_fd = open((src_dir_ + path_).data(), O_RDONLY);
    if (src_fd < 0) {
        // 文件打开失败，返回错误内容并记录日志
        ErrorContent(buffer, "File Not Found.");
        LOG_ERROR("HTTP Response: File path %s not found.", (src_dir_ + path_).data());
        return;
    }
    
    // 将文件映射到内存，提高访问速度
    void* mm_ptr = mmap(0, mmfile_stat_.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    if (mm_ptr == MAP_FAILED) {
        // 如果内存映射失败，关闭文件描述符并返回错误内容
        close(src_fd);
        ErrorContent(buffer, "File Not Found.");
        LOG_ERROR("HTTP Response: Failed to map file %s.", (src_dir_ + path_).data());
        return;
    }

    // 将内存映射结果转换为字符指针，方便处理
    mmfile_ptr_ = static_cast<char*>(mm_ptr);
    
    // 关闭文件描述符，因为文件已被映射到内存
    close(src_fd);

    buffer.Append("Content-Length: " + to_string(mmfile_stat_.st_size) + "\r\n\r\n");
    buffer.Append(mmfile_ptr_, mmfile_stat_.st_size);
    
    LOG_INFO("HTTP Response: File %s successfully mapped, size: %ld bytes.", (src_dir_ + path_).data(), mmfile_stat_.st_size);
}

std::string HTTPResponse::GetFileType() {
    string::size_type idx = path_.find_last_of('.');
    if (idx == string::npos) {
        return "text/plain";
    }
    string suffix = path_.substr(idx);
    if (SUFFIX_TYPE_.count(suffix)) {
        return SUFFIX_TYPE_.find(suffix)->second;
    }
    return "text/plain";
}

std::string HTTPResponse::GetCurrentTime() {
    time_t now = time(0);
    tm* gmt_time = gmtime(&now);
    char buf[128];                     
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt_time);
    
    return std::string(buf);
}
