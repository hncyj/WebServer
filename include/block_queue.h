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
    std::mutex _queue_mutex;
    std::condition_variable _queue_condition_var;

    std::vector<T> _queue_array;
    int _queue_size;
    int _queue_max_size;
    int _queue_first;
    int _queue_last;

public:
    explicit BlockQueue(int max_size = 1000) {
        if (max_size <= 0) exit(-1);
        _queue_max_size = max_size;
        _queue_array.resize(max_size);
        _queue_size = 0;
        _queue_first = -1;
        _queue_last = -1;
    }

    ~BlockQueue() = default;

    void clear() {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        _queue_size = 0;
        _queue_first = -1;
        _queue_last = -1;
    }

    bool isFull() const {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        return _queue_size >= _queue_max_size;
    }

    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        return _queue_size == 0;
    }

    bool get_first_element(T& value) const {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        if (_queue_size == 0) return false;
        value = _queue_array[_queue_first];
        return true;
    }

    bool get_last_element(T& value) const {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        if (_queue_size == 0) return false;
        value = _queue_array[_queue_last];
        return true;
    }

    int get_size() const {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        return _queue_size;
    }

    int get_max_size() const {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        return _queue_max_size;
    }

    bool push(const T& element) {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        if (_queue_size >= _queue_max_size) {
            _queue_condition_var.notify_all();
            return false;
        }

        _queue_last = (_queue_last + 1) % _queue_max_size;
        _queue_array[_queue_last] = element;
        ++_queue_size;
        _queue_condition_var.notify_all();

        return true;
    }

    bool pop(T& element) {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _queue_condition_var.wait(lock, [this]{return _queue_size > 0;});
        element = _queue_array[_queue_first];
        _queue_first = (_queue_first + 1) % _queue_max_size;
        --_queue_size;
        
        return true;
    }

    bool pop(T& element, int ms_timeout) {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        auto timeout = std::chrono::milliseconds(ms_timeout);
        if (!_queue_condition_var.wait_for(lock, timeout, [this]{return _queue_size > 0;})) {
            return false;
        }
        element = _queue_array[_queue_first];
        _queue_first = (_queue_first + 1) % _queue_max_size;
        --_queue_size;
        
        return true;
    }
};

#endif
