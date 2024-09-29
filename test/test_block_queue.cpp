/**
 * @file blockqueue_test.cpp
 * @author chenyinjie
 * @date 2024-09-29
 */

#include "../src/log/block_queue.h"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

class TestBlockDeque: public ::testing::Test {
protected:
    void SetUp() override {
        // 
    }

    void TearDown() override {
        // 
    }
};

// 测试基本的 Push 和 Pop 操作
TEST_F(TestBlockDeque, PushPopTest) {
    BlockDeque<int> deque(3);
    int element;

    // 队列应为空
    EXPECT_TRUE(deque.IsEmpty());
    
    // 测试 PushBack
    EXPECT_TRUE(deque.PushBack(1));
    EXPECT_TRUE(deque.PushBack(2));
    EXPECT_TRUE(deque.PushBack(3));

    EXPECT_FALSE(deque.IsEmpty());
    EXPECT_EQ(deque.GetDequeSize(), 3);
    EXPECT_TRUE(deque.IsFull());

    // 测试 PopFront
    EXPECT_TRUE(deque.PopFront(element));
    EXPECT_EQ(element, 1);

    EXPECT_TRUE(deque.PopFront(element));
    EXPECT_EQ(element, 2);

    EXPECT_TRUE(deque.PopFront(element));
    EXPECT_EQ(element, 3);

    EXPECT_TRUE(deque.IsEmpty());
    EXPECT_FALSE(deque.IsFull());
}

// 测试队列是否在达到最大容量时阻塞
TEST_F(TestBlockDeque, PushWhenDequeIsFull) {
    BlockDeque<int> deque(3);

    // 填满队列
    deque.PushBack(1);
    deque.PushBack(2);
    deque.PushBack(3);

    EXPECT_TRUE(deque.IsFull());
    
    // 模拟一个消费者线程，在等待特定时间后才处理满的消息队列
    std::thread t([&deque] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        int temp;
        deque.PopFront(temp);
    });

    // 确保 PushBack 在队列满时会阻塞
    EXPECT_TRUE(deque.PushBack(4));

    if (t.joinable()) t.join();
}



// 测试 Pop
TEST_F(TestBlockDeque, PopFront) {
    BlockDeque<int> deque(3);
    int element;

    // 队列为空时，应超时返回 false
    EXPECT_FALSE(deque.PopFront(element, 1)); // 超时时间为 1 秒
    // EXPECT_FALSE(deque.PopFront(element));

}

// 测试关闭队列
TEST_F(TestBlockDeque, CloseQueue) {
    BlockDeque<int> deque(3);

    // 向队列中 Push 数据
    deque.PushBack(1);
    deque.PushBack(2);

    int element;
    EXPECT_TRUE(deque.PopFront(element));
    EXPECT_EQ(element, 1);

    // 关闭队列
    deque.Close();

    // 队列关闭后，Push 应该失败
    EXPECT_FALSE(deque.PushBack(3));

    // Pop 应该可以继续读取剩下的数据
    EXPECT_TRUE(deque.PopFront(element));
    EXPECT_EQ(element, 2);

    // 队列关闭且为空后，Pop 也应返回 false
    EXPECT_TRUE(deque.IsEmpty());
    EXPECT_FALSE(deque.PopFront(element));
}

// 测试多线程同时 Push 和 Pop 操作
TEST_F(TestBlockDeque, MultiThreadPushPop) {
    BlockDeque<int> deque(50);

    EXPECT_TRUE(deque.IsEmpty());

    // 启动多个线程进行 Push 操作
    std::thread producer1([&deque]() {
        for (int i = 0; i < 50; ++i) {
            deque.PushBack(i);
        }
    });

    std::thread producer2([&deque]() {
        for (int i = 50; i < 100; ++i) {
            deque.PushBack(i);
        }
    });

    // 启动消费者线程
    std::thread consumer([&deque]() {
        int element;
        for (int i = 0; i < 100; ++i) {
            deque.PopFront(element);
        }
    });

    if (producer1.joinable()) producer1.join();
    if (producer2.joinable()) producer2.join();
    if (consumer.joinable()) consumer.join();

    EXPECT_TRUE(deque.IsEmpty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}