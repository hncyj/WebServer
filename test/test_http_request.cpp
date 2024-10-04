/**
 * @file test_http_request.cpp
 * @author chenyinjie
 * @date 2024-10-04
 * @copyright Apache 2.0
 */

// http_request_test.cpp

#include "../src/buffer/buffer.h"
#include "../src/http/http_request.h"
#include "../src/pool/db_connect_pool_RAII.h"

#include <gtest/gtest.h>


// 测试HTTPRequest类的构造函数和初始化函数
TEST(HTTPRequestTest, Initialization) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5);
    HTTPRequest request;
    request.Init();
    Log::GetLogInstance().Init();

    EXPECT_EQ(request.GetMethod(), "");
    EXPECT_EQ(request.GetPath(), "");
    EXPECT_EQ(request.GetVersion(), "");
    EXPECT_FALSE(request.IsKeepAlive());
}

// 测试请求行解析
TEST(HTTPRequestTest, ParseRequestLine) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5);
    HTTPRequest request;
    request.Init();
    Log::GetLogInstance().Init();

    std::string request_line = "GET /index.html HTTP/1.1";
    Buffer buffer;
    buffer.Append(request_line);
    request.Parse(buffer);

    EXPECT_EQ(request.GetMethod(), "GET");
    EXPECT_EQ(request.GetPath(), "/index.html");
    EXPECT_EQ(request.GetVersion(), "1.1");
}

// 测试完整的请求解析过程
TEST(HTTPRequestTest, CompleteRequestParsing) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5);
    HTTPRequest request;
    request.Init();
    Log::GetLogInstance().Init();
    Buffer buffer;

    // 构造HTTP请求
    std::string http_request =
        "GET /index.html HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Connection: Keep-alive\r\n"
        "\r\n";

    buffer.Append(http_request);
    
    EXPECT_TRUE(request.Parse(buffer));
    EXPECT_EQ(request.GetMethod(), "GET");
    EXPECT_EQ(request.GetPath(), "/index.html");
    EXPECT_EQ(request.GetVersion(), "1.1");
    EXPECT_TRUE(request.IsKeepAlive());
}

// 测试POST请求的解析
TEST(HTTPRequestTest, PostRequestParsing) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5);
    HTTPRequest request;
    request.Init();
    Log::GetLogInstance().Init();
    Buffer buffer;

    // // 测试不存在的用户登录
    // std::string http_request =
    //     "POST /login HTTP/1.1\r\n"
    //     "Host: www.example.com\r\n"
    //     "Content-Type: application/x-www-form-urlencoded\r\n"
    //     "Content-Length: 35\r\n"
    //     "\r\n"
    //     "username=chenyinjie&password=123456";

    // buffer.Append(http_request);
    // EXPECT_TRUE(request.Parse(buffer));
    // EXPECT_EQ(request.GetMethod(), "POST");
    // EXPECT_EQ(request.GetPath(), "/error.html");
    // EXPECT_EQ(request.GetVersion(), "1.1");
    // EXPECT_EQ(request.GetPost("username"), "chenyinjie");
    // EXPECT_EQ(request.GetPost("password"), "123456");

    // 注册用户
    // std::string http_request =
    //     "POST /register HTTP/1.1\r\n"
    //     "Host: www.example.com\r\n"
    //     "Content-Type: application/x-www-form-urlencoded\r\n"
    //     "Content-Length: 35\r\n"
    //     "\r\n"
    //     "username=chenyinjie&password=123456";

    // buffer.Append(http_request);
    // EXPECT_TRUE(request.Parse(buffer));
    // EXPECT_EQ(request.GetMethod(), "POST");
    // EXPECT_EQ(request.GetPath(), "/welcome.html");
    // EXPECT_EQ(request.GetVersion(), "1.1");
    // EXPECT_EQ(request.GetPost("username"), "chenyinjie");
    // EXPECT_EQ(request.GetPost("password"), "123456");

    // 测试注册用户登录
    std::string http_request =
        "POST /login HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 35\r\n"
        "\r\n"
        "username=chenyinjie&password=123456";

    buffer.Append(http_request);
    EXPECT_TRUE(request.Parse(buffer));
    EXPECT_EQ(request.GetMethod(), "POST");
    EXPECT_EQ(request.GetPath(), "/welcome.html");
    EXPECT_EQ(request.GetVersion(), "1.1");
    EXPECT_EQ(request.GetPost("username"), "chenyinjie");
    EXPECT_EQ(request.GetPost("password"), "123456");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}