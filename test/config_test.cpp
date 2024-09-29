/**
 * @file config_test.cpp
 * @author chenyinjie
 * @date 2024-09-29
 */

#include "../src/config/configuration.h"

#include <gtest/gtest.h>

// Test default constructor values
TEST(ConfigurationTest, DefaultConstructor) {
    Configuration config;
    EXPECT_EQ(config.PORT, 8080);
    EXPECT_EQ(config.DB_CONNECT_NUMS, 8);
    EXPECT_EQ(config.THREAD_NUMS, 8);
    EXPECT_EQ(config.ASYNC_MODE, 1);
}

// Test argument parsing
TEST(ConfigurationTest, ParseArgsValid) {
    char* argv[] = {
        (char*)"server", 
        (char*)"-p", (char*)"9090", 
        (char*)"-c", (char*)"16", 
        (char*)"-t", (char*)"4", 
        (char*)"-l", (char*)"0"
    };
    int argc = 9;
    
    Configuration config;
    config.ParseArgs(argc, argv);

    EXPECT_EQ(config.PORT, 9090);
    EXPECT_EQ(config.DB_CONNECT_NUMS, 16);
    EXPECT_EQ(config.THREAD_NUMS, 4);
    EXPECT_EQ(config.ASYNC_MODE, 0);
}

// // Test unknown argument
// TEST(ConfigurationTest, ParseArgsUnknownOption) {
//     char* argv[] = {
//         (char*)"server", 
//         (char*)"-x", 
//         (char*)"-p", (char*)"8080"
//     };
//     int argc = 4;

//     Configuration config;
//     EXPECT_EXIT(config.ParseArgs(argc, argv), 
//                 ::testing::ExitedWithCode(1), 
//                 "[ERROR]: Unknown option: -x. Use -h for help.");
// }



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}