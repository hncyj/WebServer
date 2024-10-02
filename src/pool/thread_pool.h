/**
 * @file thread_pool.h
 * @author chenyinjie
 * @date 2024-09-11
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "../log/log.h"

#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <functional>
#include <condition_variable>

using MS = std::chrono::milliseconds;

class ThreadPool {
public:
    explicit ThreadPool(size_t max_thread_nums, size_t max_task_nums);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename F>
    bool AddTask(F&& task, MS timeout = MS(100));

private:
    bool is_stop_;                                              // 线程停止标识符
    std::mutex thread_pool_mtx_;                                // 线程互斥锁
    std::condition_variable con_var_;                           // 条件变量
    size_t max_task_nums_;                                      // 任务队列最大任务数
    size_t max_thread_nums_;                                    // 线程池线程线程数
    std::vector<std::thread> thread_pool_;                      // 线程池
    std::queue<std::function<void()>> task_queue_;              // 任务队列
};

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

template <typename F>
bool ThreadPool::AddTask(F&& task, MS timeout) {
    if (is_stop_) {
        LOG_ERROR("Thread Pool: Thread Pool has stopped, can not add new task");
        throw std::runtime_error("Thread Pool has stopped, can not add new task.");
    }

    {
        std::unique_lock<std::mutex> locker(thread_pool_mtx_);
        if (!con_var_.wait_for(locker, timeout, [this]() {return task_queue_.size() < max_task_nums_;})) {
            LOG_WARN("Thread Pool: Task queue is full, failed to add new task within timeout.");
            return false;
        }
        task_queue_.emplace(std::forward<F>(task));
    }
    con_var_.notify_one();
    return true;
}

#endif