/**
 * @file test_http_response.cpp
 * @author chenyinjie
 * @date 2024-10-05
 * @copyright Apache 2.0
 * 
 */

// http_response_test.cpp

#include "../src/http/http_response.h"

#include <gtest/gtest.h>
#include <sys/types.h>

class HTTPResponseTest: public ::testing::Test {
protected:
    virtual void SetUp() {
        // 设置测试环境
        src_dir_ = "./test_resources";
        // 创建测试文件夹和文件
        mkdir(src_dir_.c_str(), 0755);
        std::ofstream outfile(src_dir_ + "/test.html");
        outfile << "<html><body><h1>Test File</h1></body></html>";
        outfile.close();
    }

    virtual void TearDown() {
        // 清理测试环境
        remove((src_dir_ + "/test.html").c_str());
        rmdir(src_dir_.c_str());
    }

    std::string src_dir_;
};

// 测试200 OK的响应
TEST_F(HTTPResponseTest, Generate200Response) {
    HTTPResponse response;
    Buffer buffer;
    std::string path = "/test.html";
    response.Init(src_dir_, path, true, 200);
    response.GenerateResponse(buffer);

    std::string response_str = buffer.ReadAllToStr();

    // 检查状态行
    EXPECT_NE(response_str.find("HTTP/1.1 200 OK"), std::string::npos);
    // 检查Content-Type
    EXPECT_NE(response_str.find("Content-Type: text/html"), std::string::npos);
    // 检查Content-Length
    EXPECT_NE(response_str.find("Content-Length: 44"), std::string::npos);
    // 检查Connection
    EXPECT_NE(response_str.find("Connection: keep-alive"), std::string::npos);
    // 检查实体内容
    EXPECT_NE(response_str.find("<html><body><h1>Test File</h1></body></html>"), std::string::npos);
}

// 测试404 Not Found的响应
TEST_F(HTTPResponseTest, Generate404Response) {
    HTTPResponse response;
    Buffer buffer;
    std::string path = "/nonexistent.html";
    response.Init(src_dir_, path, false);
    response.GenerateResponse(buffer);

    std::string response_str = buffer.ReadAllToStr();

    // 检查状态行
    EXPECT_NE(response_str.find("HTTP/1.1 404 Not Found"), std::string::npos);
    // 检查Content-Type
    EXPECT_NE(response_str.find("Content-Type: text/html"), std::string::npos);
    // 检查Connection
    EXPECT_NE(response_str.find("Connection: close"), std::string::npos);
    // 检查实体内容中是否包含404信息
    EXPECT_NE(response_str.find("404: Not Found"), std::string::npos);
}

int main(int argc, char **argv) {
    Log::GetLogInstance().Init();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}