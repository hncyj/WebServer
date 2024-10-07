cmake_minimum_required(VERSION 3.10)

# ==================== test config module ================ #
# add_executable(test_config test_config.cpp)
# target_sources(test_config PRIVATE ${PROJECT_SOURCE_DIR}/src/config/configuration.cpp)
# target_link_libraries(test_config gtest gtest_main pthread)
# add_test(NAME TestConfig COMMAND test_config)



# =================== test block queue =================== #
# add_executable(test_block_queue test_block_queue.cpp)
# target_link_libraries(test_block_queue gtest gtest_main pthread)
# add_test(NAME TestBlockQueue COMMAND test_block_queue)



# =================== test Log module ==================== #
# add_executable(test_log test_log.cpp ${PROJECT_SOURCE_DIR}/src/log/log.cpp)
# target_link_libraries(test_log gtest gtest_main pthread)
# target_compile_options(test_log PRIVATE -g -O0)
# add_test(NAME TestLogSystem COMMAND test_log)




# ================== test timer Heap ===================== #
# add_executable(
#     test_timer test_timer.cpp 
#     ${PROJECT_SOURCE_DIR}/src/timer/timer.cpp
#     ${PROJECT_SOURCE_DIR}/src/log/log.cpp
# )

# target_link_libraries(test_timer gtest gtest_main pthread)
# target_compile_options(test_timer PRIVATE -g -O0)
# add_test(NAME TestTimer COMMAND test_timer)




# ================== test Epoll Module ===================== #
# add_executable(
#     test_epoll test_epoll.cpp
#     ${PROJECT_SOURCE_DIR}/src/log/log.cpp
#     ${PROJECT_SOURCE_DIR}/src/epoll/epoll.cpp
# )

# target_link_libraries(test_epoll gtest gtest_main pthread)
# target_compile_options(test_epoll PRIVATE -g -O0)
# add_test(NAME TestEpoll COMMAND test_epoll)




# ================== test thread pool ===================== #
# add_executable(
#     test_thread_pool test_thread_pool.cpp
#     ${PROJECT_SOURCE_DIR}/src/log/log.cpp
# )

# target_link_libraries(test_thread_pool gtest gtest_main pthread)
# target_compile_options(test_thread_pool PRIVATE -g -O0)
# add_test(NAME TestThreadPool COMMAND test_thread_pool)




# ================== test connect pool ===================== #
# add_executable(
#     test_connect_pool test_connect_pool.cpp
#     ${PROJECT_SOURCE_DIR}/src/log/log.cpp
#     ${PROJECT_SOURCE_DIR}/src/pool/db_connect_pool.cpp
# )

# target_link_libraries(test_connect_pool gtest gtest_main pthread)
# target_link_libraries(test_connect_pool ${MYSQL_LIBRARIES} ${MYSQL_EXTRA_LIBS})

# target_compile_options(test_connect_pool PRIVATE -g -O0)
# add_test(NAME TestConnectPool COMMAND test_connect_pool)




# ================= test connect pool raii ================= #
# add_executable(
#     test_connect_pool_raii test_connect_pool_raii.cpp
#     ${PROJECT_SOURCE_DIR}/src/log/log.cpp
#     ${PROJECT_SOURCE_DIR}/src/pool/db_connect_pool.cpp
# )

# target_link_libraries(test_connect_pool_raii gtest gtest_main pthread)
# target_link_libraries(test_connect_pool_raii ${MYSQL_LIBRARIES} ${MYSQL_EXTRA_LIBS})

# target_compile_options(test_connect_pool_raii PRIVATE -g -O0)
# add_test(NAME TestConnectPoolRAII COMMAND test_connect_pool_raii)



# # ================= test http request ================= #
# add_executable(
#     test_http_request test_http_request.cpp
#     ${PROJECT_SOURCE_DIR}/src/log/log.cpp
#     ${PROJECT_SOURCE_DIR}/src/buffer/buffer.cpp
#     ${PROJECT_SOURCE_DIR}/src/http/http_request.cpp
#     ${PROJECT_SOURCE_DIR}/src/pool/db_connect_pool.cpp
# )

# target_link_libraries(test_http_request gtest gtest_main pthread)
# target_link_libraries(test_http_request ${MYSQL_LIBRARIES} ${MYSQL_EXTRA_LIBS})

# target_compile_options(test_http_request PRIVATE -g -O0)
# add_test(NAME TestHTTPRequest COMMAND test_http_request)



# ================= test http response ================= #
# add_executable(
#     test_http_response test_http_response.cpp
#     ${PROJECT_SOURCE_DIR}/src/log/log.cpp
#     ${PROJECT_SOURCE_DIR}/src/buffer/buffer.cpp
#     ${PROJECT_SOURCE_DIR}/src/http/http_response.cpp
# )

# target_link_libraries(test_http_response gtest gtest_main pthread)
# target_link_libraries(test_http_response ${MYSQL_LIBRARIES} ${MYSQL_EXTRA_LIBS})

# target_compile_options(test_http_response PRIVATE -g -O0)
# add_test(NAME TestHTTPResponse COMMAND test_http_response)




# =================== test buffer ===================== #
# add_executable(
#     test_buffer test_buffer.cpp
#     ${PROJECT_SOURCE_DIR}/src/buffer/buffer.cpp
# )

# target_link_libraries(test_buffer gtest gtest_main pthread)
# target_link_options(test_buffer PRIVATE -g -O0)
# add_test(NAME TestBuffer COMMAND test_buffer)


