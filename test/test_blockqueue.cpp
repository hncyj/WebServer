#include "block_queue.h"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

TEST(TestBlockQueue, pushNpop) {
    BlockQueue<int> bq(10);

    EXPECT_TRUE(bq.isEmpty());
    EXPECT_FALSE(bq.isFull());

    bq.push(1);
    bq.push(2);
    bq.push(3);

    EXPECT_FALSE(bq.isEmpty());
    EXPECT_EQ(bq.get_size(), 3);

    int val;
    EXPECT_TRUE(bq.pop(val));
    EXPECT_EQ(val, 1);
    
    EXPECT_TRUE(bq.pop(val));
    EXPECT_EQ(val, 2);

    EXPECT_TRUE(bq.pop(val));
    EXPECT_EQ(val, 3);

    EXPECT_TRUE(bq.isEmpty());
}

TEST(TestBlockQueue, popTimeout) {
    BlockQueue<int> bq(3);

    int val;
    auto start_time = std::chrono::steady_clock::now();
    EXPECT_FALSE(bq.pop(val, 100));
    auto end_time = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    EXPECT_GE(duration, 100);
}

TEST(TestBlockQueue, multiThreadPushNPop) {
    BlockQueue<int> bq(10);
    std::thread producer([&bq]() {
        for (int i = 0; i < 5; ++i) {
            bq.push(i);
        }
    });

    std::thread consumer([&bq]() {
        int val;
        for (int i = 0; i < 5; ++i) {
            EXPECT_TRUE(bq.pop(val));
            EXPECT_EQ(val, i);
        }
    });

    producer.join();
    consumer.join();

    EXPECT_TRUE(bq.isEmpty());
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(color) = "yes";  // 启用颜色输出
    return RUN_ALL_TESTS();
}

