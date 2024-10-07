/**
 * @file thread_pool.cpp
 * @author chenyinjie
 * @date 2024-10-07
 * @copyright Apache 2.0
 */

#include "thread_pool.h"

ThreadPool::ThreadPool(size_t max_thread_nums = 8, size_t max_task_nums = 16): max_thread_nums_(max_thread_nums), max_task_nums_(max_task_nums), is_stop_(false) {
    if (max_thread_nums_ <= 0) {
        LOG_ERROR("Thread Pool: Invalid thread nums: %zu.", max_thread_nums_);
        throw std::invalid_argument("Invalid number of threads.");
    }
    if (max_task_nums_ <= 0) {
        LOG_ERROR("Thread Pool: Invalid task queue size: %zu.", max_task_nums_);
        throw std::invalid_argument("Invalid task queue size.");
    }
    for (size_t i = 0; i < max_thread_nums_; ++i) {
        try {
            thread_pool_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> locker(thread_pool_mtx_);
                        con_var_.wait(locker, [this]() {return !task_queue_.empty() || is_stop_;});
                        if (is_stop_ && task_queue_.empty()) return;
                        task = std::move(task_queue_.front());
                        task_queue_.pop();
                    }
                    try {
                        task();
                    } catch (const std::exception& e) {
                        LOG_ERROR("Thread Pool: Task threw an exception: %s", e.what());
                    }
                }
            });
        } catch (const std::system_error& e) {
            LOG_ERROR("Thread Pool: Failed to create thread: %s", e.what());
            throw;
        }
    }
    LOG_INFO("Thread Pool: Init thread pool success.");
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> locker(thread_pool_mtx_);
        is_stop_ = true;
    }
    con_var_.notify_all();

    for (auto& thread : thread_pool_) { 
        if (thread.joinable()) thread.join();
    }
}