#ifndef BLOCK_QUEUE_H_
#define BLOCK_QUEUE_H_

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

// 线程安全的阻塞队列
// 利用循环数组实现

template <typename T>
class BlockQueue {
private:
    mutable std::mutex m_queue_mutex;
    std::condition_variable m_queue_condition_var;

    std::vector<T> m_queue_array;
    int m_queue_size;
    int m_queue_max_size;
    int m_queue_first;
    int m_queue_last;

public:
    explicit BlockQueue(int max_size = 1000);
    BlockQueue(const BlockQueue&) = delete;
    BlockQueue& operator=(const BlockQueue&) = delete;
    ~BlockQueue();

    void clear();
    bool isFull() const;
    bool isEmpty() const;
    int get_size() const;
    int get_max_size() const;

    bool get_first_element(T& value) const;
    bool get_last_element(T& value) const;

    bool push(const T& element);
    bool pop(T& element);
    bool pop(T& element, int ms_timeout);
};

template <typename T>
BlockQueue<T>::BlockQueue(int max_size)
    : m_queue_max_size(max_size),
      m_queue_array(max_size),
      m_queue_size(0),
      m_queue_first(0),
      m_queue_last(-1) {
    if (max_size <= 0) {
        std::cerr << "Invalid max size" << std::endl;
        exit(-1);
    }
}

template <typename T>
BlockQueue<T>::~BlockQueue() = default;

template <typename T>
void BlockQueue<T>::clear() {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    m_queue_size = 0;
    m_queue_first = 0;
    m_queue_last = -1;
}

template <typename T>
bool BlockQueue<T>::isFull() const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_queue_size >= m_queue_max_size;
}

template <typename T>
bool BlockQueue<T>::isEmpty() const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_queue_size == 0;
}

template <typename T>
int BlockQueue<T>::get_size() const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_queue_size;
}

template <typename T>
int BlockQueue<T>::get_max_size() const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_queue_max_size;
}

template <typename T>
bool BlockQueue<T>::get_first_element(T& value) const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_queue_size == 0) return false;
    value = m_queue_array[m_queue_first];
    return true;
}

template <typename T>
bool BlockQueue<T>::get_last_element(T& value) const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_queue_size == 0) return false;
    value = m_queue_array[m_queue_last];
    return true;
}

template <typename T>
bool BlockQueue<T>::push(const T& element) {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_queue_size >= m_queue_max_size) {
        m_queue_condition_var.notify_all();
        return false;
    }
    m_queue_last = (m_queue_last + 1) % m_queue_max_size;
    m_queue_array[m_queue_last] = element;
    ++m_queue_size;
    m_queue_condition_var.notify_all();

    return true;
}

template <typename T>
bool BlockQueue<T>::pop(T& element) {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    m_queue_condition_var.wait(lock, [this]{return m_queue_size > 0;});
    element = m_queue_array[m_queue_first];
    m_queue_first = (m_queue_first + 1) % m_queue_max_size;
    --m_queue_size;
    
    return true;
}

template <typename T>
bool BlockQueue<T>::pop(T& element, int ms_timeout) {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    auto timeout = std::chrono::milliseconds(ms_timeout);
    if (!m_queue_condition_var.wait_for(lock, timeout, [this]{return m_queue_size > 0;})) {
        return false;
    }
    element = m_queue_array[m_queue_first];
    m_queue_first = (m_queue_first + 1) % m_queue_max_size;
    --m_queue_size;
    
    return true;
}

#endif // BLOCK_QUEUE_H_