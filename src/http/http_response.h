/**
 * @file http_response.h
 * @author chenyinjie
 * @date 2024-09-22
 */

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "../buffer/buffer.h"
#include "../log/log.h"

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string>
#include <fstream>
#include <ctime>

class HTTPResponse {
public:
    HTTPResponse();
    ~HTTPResponse();

    void Init(const std::string& src_dir, std::string& path, bool is_keep_alive = false, int code = -1);
    void GenerateResponse(Buffer& buffer);
    void UnmapMemoryFilePtr();
    char* GetMapFilePtr();
    size_t GetFileLen() const;
    void ErrorContent(Buffer& buffer, std::string message);
    int Code() const;

private:
    void AddStateLine(Buffer &buffer);
    void AddHeader(Buffer &buffer);
    void AddContent(Buffer &buffer);

    void ErrorHtml();
    std::string GetFileType();
    std::string GetCurDateTime();

    int code_;
    bool is_keep_alive_;

    std::string path_;
    std::string src_dir_;
    
    char* mapped_file_ptr_; 
    struct stat mapped_file_stat_;

    bool is_log_open_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif
