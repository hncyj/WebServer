/**
 * @file http_response.cpp
 * @author chenyinjie
 * @date 2024-09-22 
 */


#include "http_response.h"

using namespace std;

const unordered_map<string, string> HTTPResponse::SUFFIX_TYPE = {
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
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const unordered_map<int, string> HTTPResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> HTTPResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HTTPResponse::HTTPResponse(): code_(-1), 
                              is_keep_alive_(false), 
                              path_(""), 
                              src_dir_(""), 
                              mapped_file_ptr_(nullptr), 
                              mapped_file_stat_{0} {}

HTTPResponse::~HTTPResponse() {
    UnmapMemoryFilePtr();
}

void HTTPResponse::Init(const std::string& src_dir, std::string& path, bool is_keep_alive, int code) {
    assert(!src_dir_.empty());
    if (mapped_file_ptr_) {
        UnmapMemoryFilePtr();
    }
    code_ = code;
    path_ = path;
    is_keep_alive_ = is_keep_alive;
    mapped_file_ptr_ = nullptr;
    mapped_file_stat_ = {0};
    is_log_open_ = Log::getInstance().isOpen();
}

void HTTPResponse::GenerateResponse(Buffer& buffer) {
    if (stat((src_dir_ + path_).data(), &mapped_file_stat_) < 0 || S_ISDIR(mapped_file_stat_.st_mode)) {
        code_ = 404;
    } else if (!(mapped_file_stat_.st_mode & S_IROTH)) {
        code_ = 403;
    } else if (code_ == -1) {
        code_ = 200;
    }
    ErrorHtml();
    AddStateLine(buffer);
    AddHeader(buffer);
    AddContent(buffer);
}

void HTTPResponse::UnmapMemoryFilePtr() {
    if (mapped_file_ptr_) {
        munmap(mapped_file_ptr_, mapped_file_stat_.st_size);
        mapped_file_ptr_ = nullptr;
    }
}

char* HTTPResponse::GetMapFilePtr() {
    return mapped_file_ptr_;
}

size_t HTTPResponse::GetFileLen() const {
    return mapped_file_stat_.st_size;
}

void HTTPResponse::ErrorContent(Buffer& buffer, std::string message) {
    // 生成错误资源获取错误信息，并添加至缓冲区
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";

    if (CODE_STATUS.count(code_)) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }

    body += to_string(code_) + ":" + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>WebServer</em></body></html>";

    buffer.Append("Content-lenth:" + to_string(body.size()) + "\r\n\r\n");
    buffer.Append(body);
}

int HTTPResponse::Code() const {
  return 0;
}

void HTTPResponse::AddStateLine(Buffer& buffer) {
    string status;
    if (CODE_STATUS.count(code_)) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        code_ = 400;
        status = CODE_STATUS.find(code_)->second;
    }
    buffer.Append("HTTP/1.1" + to_string(code_) + " " + status + "\r\n");
}

void HTTPResponse::AddHeader(Buffer& buffer) {
    buffer.Append("Connection: ");
    if (is_keep_alive_) {
        buffer.Append("keep-alive\r\n");
        buffer.Append("keep-alive: max = 10, timeout=120\r\n");
    } else {
        buffer.Append("close\r\n");
    }
    buffer.Append("Date: " + GetCurDateTime() + "\r\n");
    // TODO:
    // Last-Modified..
    buffer.Append("Content-Type: " + GetFileType() + "\r\n");
}

void HTTPResponse::AddContent(Buffer& buffer) {
    int src_fd = open((src_dir_ + path_).data(), O_RDONLY);
    if (src_fd < 0) {
        ErrorContent(buffer, "File Not Found.");
    }
    if (is_log_open_) {
        LOG_ERROR("file path %s", (src_dir_ + path_).data());
    }

    void* map_mem_ptr = static_cast<int*>(mmap(0, mapped_file_stat_.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
    if (map_mem_ptr == MAP_FAILED) {
        ErrorContent(buffer, "File Not Found.");
        return;
    }
    mapped_file_ptr_ = static_cast<char*>(map_mem_ptr);
    close(src_fd);
    buffer.Append("Content-Length: " + to_string(mapped_file_stat_.st_size) + "\r\n\r\n");
} 

void HTTPResponse::ErrorHtml() {
    if (CODE_PATH.count(code_)) {
        path_ = CODE_PATH.find(code_)->second;
        stat((src_dir_ + path_).data(), &mapped_file_stat_);
    }
}

std::string HTTPResponse::GetFileType() {
    string::size_type idx = path_.find_last_of('.');
    if (idx == string::npos) {
        return "text/plain";
    }
    string suffix = path_.substr(idx);
    if (SUFFIX_TYPE.count(suffix)) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

std::string HTTPResponse::GetCurDateTime() {
    time_t now = time(0);
    tm* gmt_time = gmtime(&now);
    char buf[128];                     
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt_time);
    
    return std::string(buf);
}
