/**
 * @file test_buffer.cpp
 * @author chenyinjie
 * @date 2024-10-06
 * @copyright Apache 2.0
 */


#include "../src/buffer/buffer.h"

#include <gtest/gtest.h>

class BufferTest : public ::testing::Test {
protected:
    Buffer buffer;
};

// Test Buffer initialization
TEST_F(BufferTest, InitialState) {
    EXPECT_EQ(buffer.ReadableLen(), 0);
    EXPECT_EQ(buffer.WritePtr() - buffer.ReadPtr(), 0);
}

// Test Append and Read operations
TEST_F(BufferTest, AppendAndRead) {
    const std::string data = "Hello, World!";
    buffer.Append(data.c_str(), data.size());

    EXPECT_EQ(buffer.ReadableLen(), data.size());
    EXPECT_STREQ(buffer.ReadPtr(), data.c_str());

    buffer.ReadLen(5);  // Read "Hello"
    EXPECT_EQ(buffer.ReadableLen(), data.size() - 5);
    EXPECT_STREQ(buffer.ReadPtr(), ", World!");
}

// Test Append and ReadAllToStr
TEST_F(BufferTest, AppendAndReadAllToStr) {
    const std::string data = "Test Buffer";
    buffer.Append(data);

    std::string read_data = buffer.ReadAllToStr();
    EXPECT_EQ(read_data, data);
    EXPECT_EQ(buffer.ReadableLen(), 0);  // All data should be read
}

// Test clearing buffer
TEST_F(BufferTest, ClearBuffer) {
    const std::string data = "Clear Test";
    buffer.Append(data);

    buffer.Clear();
    EXPECT_EQ(buffer.ReadableLen(), 0);
}

// Test buffer expansion
TEST_F(BufferTest, BufferExpansion) {
    std::string long_data(2000, 'a');  // Append 2000 'a' characters
    buffer.Append(long_data);

    EXPECT_EQ(buffer.ReadableLen(), long_data.size());
    EXPECT_EQ(std::string(buffer.ReadPtr(), buffer.ReadableLen()), long_data);
}

// Test ReadFromFd and WriteToFd (requires valid file descriptor)
TEST_F(BufferTest, ReadWriteFd) {
    // Mock file descriptor reading and writing could be done with pipe for real file interaction
    int pipe_fds[2];
    ASSERT_EQ(pipe(pipe_fds), 0);

    const std::string data = "Pipe test data";
    write(pipe_fds[1], data.c_str(), data.size());

    int saveErrno = 0;
    buffer.ReadFromFd(pipe_fds[0], &saveErrno);
    EXPECT_EQ(buffer.ReadableLen(), data.size());
    EXPECT_STREQ(buffer.ReadPtr(), data.c_str());

    close(pipe_fds[0]);
    close(pipe_fds[1]);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}