/**
 * @file thread_pool.h
 * @author chenyinjie
 * @date 2024-09-11
 * @copyright Apache 2.0
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