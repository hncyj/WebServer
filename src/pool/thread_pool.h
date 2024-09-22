/**
 * @file thread_pool.h
 * @author chenyinjie
 * @date 2024-09-11
 */

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include "../log/log.h"

#include <iostream>
#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <cassert>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    static ThreadPool& GetInstance(size_t max_thread_nums = 8, size_t max_task_nums = 16);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename F>
    void AddTask(F&& task);

private:
    explicit ThreadPool(size_t max_thread_nums = 8, size_t max_task_nums = 16);
    ~ThreadPool();

    bool is_stop_;
    std::mutex thread_pool_mtx_;
    std::condition_variable con_var_;
    size_t max_task_nums_;
    std::vector<std::thread> thread_pool_;
    std::queue<std::function<void()>> task_queue_;
};

ThreadPool& ThreadPool::GetInstance(size_t max_thread_nums = 8, size_t max_task_nums = 16) {
    static ThreadPool thread_pool_instance(max_thread_nums, max_task_nums);
    return thread_pool_instance;
}

ThreadPool::ThreadPool(size_t max_thread_nums = 4, size_t max_task_nums = 16): max_task_nums_(max_task_nums), is_stop_(false) {
    assert(max_thread_nums > 0);
    for (size_t i = 0; i < max_thread_nums; ++i) {
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

                bool is_log_open = Log::getInstance().isOpen();
                try {
                    if (is_log_open) {
                        LOG_INFO("Thread is executing a task.");
                    }
                    task();
                } catch(const std::exception& e) {
                    if (is_log_open) {
                        LOG_ERROR("Exception in task: ", e.what());
                    } else {
                        std::cerr << "Exception in task: " << e.what() << std::endl;
                    }
                } catch(...) {
                    if (is_log_open) {
                        LOG_INFO("Something happend.");
                    } else {
                        std::cerr << "Unkown exception in task." << std::endl;
                    }
                }
            }
        });
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
void ThreadPool::AddTask(F&& task) {
    if (is_stop_) {
        throw std::runtime_error("Thread Pool has stopped, can not add new task.");
    }

    {
        std::unique_lock<std::mutex> locker(thread_pool_mtx_);
        con_var_.wait(locker, [this]() {return task_queue_.size() < max_task_nums_;});
        task_queue_.emplace(std::forward<F>(task));
    }
    con_var_.notify_one();
}

#endif