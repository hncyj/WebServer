/**
 * @file test_epoll.cpp
 * @author chenyinjie
 * @date 2024-10-01
 */

#include <gtest/gtest.h>
#include <sys/eventfd.h>
#include "../src/epoll/epoll.h"

class EpollTest : public ::testing::Test {
protected:
    void SetUp() override {
        epoll = new Epoll();
        event_fd = eventfd(0, EFD_NONBLOCK);
        ASSERT_GT(event_fd, 0) << "Failed to create eventfd.";
    }

    void TearDown() override {
        if (event_fd > 0) {
            close(event_fd);
        }
        delete epoll;
    }

    Epoll* epoll;
    int event_fd;
};

// 测试 AddFd 功能
TEST_F(EpollTest, AddFd) {
    bool result = epoll->AddFd(event_fd, EPOLLIN);
    EXPECT_TRUE(result);
}

// 测试 ModifyFd 功能
TEST_F(EpollTest, ModifyFd) {
    ASSERT_TRUE(epoll->AddFd(event_fd, EPOLLIN));

    bool result = epoll->ModifyFd(event_fd, EPOLLOUT);
    EXPECT_TRUE(result);
}

// 测试 DeleteFd 功能
TEST_F(EpollTest, DeleteFd) {
    ASSERT_TRUE(epoll->AddFd(event_fd, EPOLLIN));

    bool result = epoll->DeleteFd(event_fd);
    EXPECT_TRUE(result);
}

// 测试 EpollWait 功能 (使用 eventfd 模拟事件触发)
TEST_F(EpollTest, EpollWait) {
    ASSERT_TRUE(epoll->AddFd(event_fd, EPOLLIN));

    uint64_t u = 1;
    ASSERT_EQ(write(event_fd, &u, sizeof(uint64_t)), sizeof(uint64_t));

    // 调用 epoll_wait，确保能捕获到事件
    int nfds = epoll->EpollWait(1000);  // 超时时间 1000 毫秒
    EXPECT_GT(nfds, 0) << "Expected event but got none.";

    // 检查是否捕获到正确的文件描述符和事件
    EXPECT_EQ(epoll->GetEventFd(0), event_fd);
    EXPECT_EQ(epoll->GetEvents(0) & EPOLLIN, EPOLLIN);
}

TEST_F(EpollTest, ModifyInvalidFd) {
    ASSERT_FALSE(epoll->DeleteFd(event_fd));
    ASSERT_FALSE(epoll->ModifyFd(event_fd, EPOLLOUT));
    ASSERT_TRUE(epoll->AddFd(event_fd, EPOLLIN));
    ASSERT_TRUE(epoll->DeleteFd(event_fd));
}

int main(int argc, char **argv) {
    // 初始化日志系统
    Log::GetLogInstance().Init(10, true, 10, 30);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
