/**
 * @file http_response.h
 * @author chenyinjie
 * @date 2024-09-22
 * @copyright Apache 2.0
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

    void Init(const std::string& src_dir, const std::string& path, bool is_keep_alive = false, int code = -1);
    void GenerateResponse(Buffer& buffer);
    void UnmapFilePtr();
    char* GetFilePtr();
    size_t GetFileLen() const;
    void ErrorContent(Buffer& buffer, std::string message);
    int GetCode() const;

private:
    void ErrorHtml();
    void AddStateLine(Buffer &buffer);
    void AddHeader(Buffer &buffer);
    void AddContent(Buffer &buffer);

    std::string GetFileType();
    std::string GetCurrentTime();

    int code_;                                                               // 响应状态吗
    bool is_keep_alive_;                                                     // 是否保持连接
    std::string path_;                                                       // 请求资源路径
    std::string src_dir_;                                                    // 资源存储目录
    char* mmfile_ptr_;                                                       // 内存映射文件指针
    struct stat mmfile_stat_;                                                // 文件信息结构

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE_;  // 类型后缀映射
    static const std::unordered_map<int, std::string> CODE_STATUS_;          // 状态码映射
    static const std::unordered_map<int, std::string> CODE_PATH_;            // 错误页面路径
};

#endif
