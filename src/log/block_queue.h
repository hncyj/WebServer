/**
 * @file block_queue.h
 * @author chenyinjie
 * @date 2024-09-02
 */

#ifndef BLOCK_QUEUE_H_
#define BLOCK_QUEUE_H_

#include <deque>
#include <mutex>
#include <condition_variable>

/**
 * @brief 单一消费者，多生产者的阻塞双端队列
 */

template <typename T>
class BlockDeque {
public:
    explicit BlockDeque(size_t max_capacity = 1024);
    ~BlockDeque();

    // void clear();
    void close();

    bool isEmpty() const noexcept;
    bool isFull() const noexcept;

    size_t getSize() const noexcept;
    size_t getCapacity() const noexcept;

    bool getFrontElement(T&);
    bool getLastElement(T&);

    bool pushFront(T&&);
    bool pushFront(const T&);
    bool pushBack(T&&);
    bool pushBack(const T&);
    
    bool popFront(T&);
    bool popFront(T&, int);  // 超时版本
    
    void flush();

private:
    std::deque<T> block_deque_;
    bool is_deque_open_;
    size_t deque_capacity_;
    mutable std::mutex deque_mtx_;
    std::condition_variable consumer_con_var_;
    std::condition_variable producer_con_var_;
};

template <typename T>
BlockDeque<T>::BlockDeque(size_t max_capacity)
    : deque_capacity_(max_capacity), is_deque_open_(true) {
        if (deque_capacity_ == 0) {
            throw std::invalid_argument("Block Queue's capacity must be greater than zero.");
        }
    }

template <typename T>
BlockDeque<T>::~BlockDeque() {
    close();
}

// template <typename T>
// void BlockDeque<T>::clear() {
//     std::lock_guard<std::mutex> locker(deque_mtx_);
//     block_deque_.clear();
//     producer_con_var_.notify_all();
//     consumer_con_var_.notify_one();
// }

template <typename T>
void BlockDeque<T>::close() {
    {
        std::lock_guard<std::mutex> locker(deque_mtx_);
        is_deque_open_ = false;
    }
    consumer_con_var_.notify_one();
    producer_con_var_.notify_all();
}

template <typename T>
bool BlockDeque<T>::isEmpty() const noexcept {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    return block_deque_.empty();
}

template <typename T>
bool BlockDeque<T>::isFull() const noexcept {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    return block_deque_.size() >= deque_capacity_;
}

template <typename T>
size_t BlockDeque<T>::getSize() const noexcept {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    return block_deque_.size();
}

template <typename T>
size_t BlockDeque<T>::getCapacity() const noexcept {
    return deque_capacity_;
}

template <typename T>
bool BlockDeque<T>::getFrontElement(T& element) {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    if (block_deque_.empty()) return false;
    element = block_deque_.front();
    return true;
}

template <typename T>
bool BlockDeque<T>::getLastElement(T& element) {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    if (block_deque_.empty()) return false;
    element = block_deque_.back();
    return true;
}

template <typename T>
bool BlockDeque<T>::pushFront(T&& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this]() {return block_deque_.size() < deque_capacity_ || !is_deque_open_;});
    if (!is_deque_open_) return false;
    try {
        block_deque_.push_front(std::move(element));
    } catch (...) {
        throw;
    }
    consumer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::pushFront(const T& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this] {return block_deque_.size() < deque_capacity_ || !is_deque_open_;});
    if (!is_deque_open_) return false;
    try {
        block_deque_.push_front(element);
    } catch (...) { // 偷个懒，可能存在内存申请失败的问题
        throw;
    }
    consumer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::pushBack(T&& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this]() {return block_deque_.size() < deque_capacity_ || !is_deque_open_;});
    if (!is_deque_open_) return false;
    try {
        block_deque_.push_back(std::move(element));
    } catch (...) {
        throw;
    }
    consumer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::pushBack(const T& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this] {return block_deque_.size() < deque_capacity_ || !is_deque_open_;});
    if (!is_deque_open_) return false;
    try {
        block_deque_.push_back(element);
    } catch (...) {
        throw;
    }
    consumer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::popFront(T& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    consumer_con_var_.wait(locker, [this] {return !block_deque_.empty() || !is_deque_open_;});
    if (block_deque_.empty()) return false;
    element = std::move(block_deque_.front());
    block_deque_.pop_front();
    producer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::popFront(T& element, int timeout) {
    auto secs = std::chrono::seconds(timeout);
    std::unique_lock<std::mutex> locker(deque_mtx_);
    if (!consumer_con_var_.wait_for(locker, secs, [this] {return !block_deque_.empty() || !is_deque_open_;})) {
        return false;
    }
    if (block_deque_.empty()) return false;
    element = std::move(block_deque_.front());
    block_deque_.pop_front();
    producer_con_var_.notify_one();
    return true;
}

template <typename T>
void BlockDeque<T>::flush() {
    consumer_con_var_.notify_one();
}

#endif
