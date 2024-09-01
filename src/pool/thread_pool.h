#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <stdexcept>

#include "db_connection_pool.h"

template <typename F>
class ThreadPool {
private:
    bool m_stop;                                    // 线程池停止标志
    int m_max_thread_nums;                          // 线程池最大线程数
    int m_max_task_nums;                            // 任务请求队列最大请求数
    std::vector<std::thread> m_threads_vector;      // 线程池数组
    std::queue<F*> m_tasks_queue;                   // 任务请求队列
    std::mutex m_tasks_queue_mutex;                 // 请求队列互斥锁
    std::condition_variable m_con_var;              // 请求队列线程同步条件变量
    ConnectionPool& m_db_connection_pool;           // 数据库连接池
    int m_actor_model;                              // 线程池模型

private:
    void work();
    void run();

public:
    ThreadPool(int actor_model, int max_thread_nums, int max_task_nums, ConnectionPool& connection_pool);
    ~ThreadPool();

    bool enqueue_without_state(F* task, int state);
    bool enqueue_with_state(F* task);
};

template <typename F>
void ThreadPool<F>::work() {
    run();
}

template <typename F>
void ThreadPool<F>::run() {
    while (true) {
        F* task;
        {
            std::unique_lock<std::mutex> lock(m_tasks_queue_mutex);
            m_con_var.wait(lock, [this]() {
                return m_stop || !m_tasks_queue.empty();
            });

            if (m_stop && m_tasks_queue.empty()) return;
            task = m_tasks_queue.front();
            m_tasks_queue.pop();
        }

        if (!task) continue;

        // TODO: definetion for task.
        if (m_actor_model == 0) {
            
        } else {

        }
    }
}

template <typename F>
ThreadPool<F>::ThreadPool(int actor_model,
                          int max_thread_nums,
                          int max_task_nums,
                          ConnectionPool& connection_pool) {
  m_stop = false;
  m_actor_model = actor_model;
  m_max_thread_nums = max_thread_nums;
  m_max_task_nums = max_task_nums;
  m_db_connection_pool = connection_pool;

  if (m_max_thread_nums <= 0 || m_max_task_nums <= 0)
    throw std::invalid_argument("Invalid thread number or task number.");

  for (int i = 0; i < m_max_thread_nums; ++i) {
    m_threads_vector.emplace([this]() {

    });
  }
}

template <typename F>
ThreadPool<F>::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_tasks_queue_mutex);
        m_stop = true;
    }
    m_con_var.notify_all();
    for (auto& thread : m_threads_vector) { 
        if (thread.joinable()) thread.join();
    }
}

template <typename F>
bool ThreadPool<F>::enqueue_without_state(F* task, int state) {
}

template <typename F>
bool ThreadPool<F>::enqueue_with_state(F* task) {
}

#endif