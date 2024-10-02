/**
 * @file test_thread_poll.cpp
 * @author chenyinjie
 * @date 2024-10-02
 */

#include "../src/pool/thread_pool.h"

#include <gtest/gtest.h>
#include <chrono>
#include <atomic>

// Test case for adding a task to the thread pool
TEST(ThreadPoolTest, AddTask) {
    ThreadPool pool(4, 8);
    std::atomic<int> counter{0};

    // Add 8 tasks to the thread pool
    for (int i = 0; i < 8; ++i) {
        pool.AddTask([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
            ++counter;
            }, std::chrono::milliseconds(100)
        );
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(counter.load(), 8);
}

// Test case for task timeout (queue full)
TEST(ThreadPoolTest, AddTaskTimeout) {
    ThreadPool pool(2, 4);
    std::atomic<int> counter{0};
    // Add 4 tasks that will block the thread pool (simulate long tasks)
    for (int i = 0; i < 6; ++i) {
        pool.AddTask([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            ++counter;
        }, std::chrono::milliseconds(100));
    }

    // Try to add another task, which should timeout because the queue is full
    pool.AddTask([&counter]() {
        ++counter;
    }, std::chrono::milliseconds(10));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(counter.load(), 2);
}

int main(int argc, char **argv) {
    Log::GetLogInstance().Init(10, true, 128, 30);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}