/**
 * @file test_timer.cpp
 * @author chenyinjie
 * @date 2024-10-01
 */

#include "../src/timer/timer.h"

#include <gtest/gtest.h>
#include <thread>
#include <atomic>

std::atomic<int> callback_counter(0);


void TestCallback() {
    callback_counter++;
}


TEST(TimerHeapTest, AddTimerTest) {
    TimerHeap timer_heap;

    timer_heap.AddTimer(1, 100, TestCallback);

    EXPECT_LE(timer_heap.GetNextExpireTime(), 100);
    EXPECT_GE(timer_heap.GetNextExpireTime(), 95);
}

TEST(TimerHeapTest, UpdateTimerTest) {
    TimerHeap timer_heap;

    timer_heap.AddTimer(1, 100, TestCallback);
    timer_heap.UpdateTimer(1, 200);

    EXPECT_LE(timer_heap.GetNextExpireTime(), 200);
    EXPECT_GE(timer_heap.GetNextExpireTime(), 195);
}

TEST(TimerHeapTest, RemoveTimerTest) {
    TimerHeap timer_heap;

    timer_heap.AddTimer(1, 100, TestCallback);
    timer_heap.AddTimer(2, 200, TestCallback);

    EXPECT_LE(timer_heap.GetNextExpireTime(), 100);
    EXPECT_GE(timer_heap.GetNextExpireTime(), 95);

    timer_heap.RemoveTopTimer();

    EXPECT_LE(timer_heap.GetNextExpireTime(), 200);
    EXPECT_GE(timer_heap.GetNextExpireTime(), 195);
}

TEST(TimerHeapTest, CleanExpiredTimerTest) {
    TimerHeap timer_heap;

    callback_counter = 0;

    timer_heap.AddTimer(1, 100, TestCallback);
    timer_heap.AddTimer(2, 200, TestCallback);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    timer_heap.CleanExpiredTimer();

    EXPECT_EQ(callback_counter.load(), 1);

    int next_expire = timer_heap.GetNextExpireTime();

    EXPECT_LE(next_expire, 50);
    EXPECT_GE(next_expire, 0);
}

TEST(TimerHeapTest, MultipleTimersTest) {
    TimerHeap timer_heap;

    callback_counter = 0;

    timer_heap.AddTimer(1, 100, TestCallback);
    timer_heap.AddTimer(2, 150, TestCallback);
    timer_heap.AddTimer(3, 200, TestCallback);

    // 等待 250 毫秒，确保所有定时器都过期
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    timer_heap.CleanExpiredTimer();

    // 检查回调函数是否被调用三次
    EXPECT_EQ(callback_counter.load(), 3);

    EXPECT_EQ(timer_heap.GetNextExpireTime(), -1);
}

TEST(TimerHeapTest, UpdateNonexistentTimerTest) {

    TimerHeap timer_heap;

    timer_heap.UpdateTimer(1, 100);

    EXPECT_EQ(timer_heap.GetNextExpireTime(), -1);
}

TEST(TimerHeapTest, AddDuplicateTimerTest) {
    TimerHeap timer_heap;

    timer_heap.AddTimer(1, 100, TestCallback);

    timer_heap.AddTimer(1, 200, TestCallback);

    EXPECT_LE(timer_heap.GetNextExpireTime(), 100);
    EXPECT_GE(timer_heap.GetNextExpireTime(), 95);
}

TEST(TimerHeapTest, CallbackExecutionTest) {
    TimerHeap timer_heap;

    callback_counter = 0;

    timer_heap.AddTimer(1, 50, []() {
        callback_counter += 2;
    });

    // 等待 100 毫秒，确保定时器过期
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    timer_heap.CleanExpiredTimer();

    // 检查回调函数是否被调用并正确修改了计数器
    EXPECT_EQ(callback_counter.load(), 2);
}

TEST(TimerHeapTest, ClearAllTimersTest) {
    TimerHeap timer_heap;

    timer_heap.AddTimer(1, 100, TestCallback);
    timer_heap.AddTimer(2, 200, TestCallback);

    timer_heap.ClearAllTimers();

    EXPECT_EQ(timer_heap.GetNextExpireTime(), -1);
}

TEST(TimerHeapTest, GetNextExpireTimeTest) {
    TimerHeap timer_heap;

    timer_heap.AddTimer(1, 100, TestCallback);
    timer_heap.AddTimer(2, 50, TestCallback);
    timer_heap.AddTimer(3, 150, TestCallback);

    // 下一个过期时间应接近 50 毫秒
    int next_expire = timer_heap.GetNextExpireTime();
    EXPECT_LE(next_expire, 50);
    EXPECT_GE(next_expire, 0);
}

int main(int argc, char **argv) {
    Log::GetLogInstance().Init(3, true, 10, 30);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}