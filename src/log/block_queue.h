/**
 * @file block_queue.h
 * @author chenyinjie
 * @date 2024-09-02
 */

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

/**
 * @brief 
 * 单消费者-多生产者模型
 * 异步日志模式下打开
 */

template <typename T>
class BlockDeque {
public:
    explicit BlockDeque(size_t max_capacity = 1024); 
    ~BlockDeque();

    // void clear();
    void Close();

    bool IsEmpty() const noexcept;
    bool IsFull() const noexcept;

    size_t GetDequeSize() const noexcept;
    size_t GetDequeCapacity() const noexcept;

    bool GetFrontElement(T&);
    bool GetLastElement(T&);

    bool PushFront(T&&);
    bool PushFront(const T&);
    bool PushBack(T&&);
    bool PushBack(const T&);
    
    bool PopFront(T&);
    bool PopFront(T&, int);  // 超时版本
    
    void Flush();

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
    if (deque_capacity_ <= 0) {
        throw std::invalid_argument("BlockQueue's capacity must be greater than zero.");
    }
}

template <typename T>
BlockDeque<T>::~BlockDeque() {
    Close();
}

// template <typename T>
// void BlockDeque<T>::clear() {
//     std::lock_guard<std::mutex> locker(deque_mtx_);
//     block_deque_.clear();
//     producer_con_var_.notify_all();
//     consumer_con_var_.notify_one();
// }

template <typename T>
void BlockDeque<T>::Close() {
    {
        std::lock_guard<std::mutex> locker(deque_mtx_);
        is_deque_open_ = false;
    }
    // TODO: 思考通知生产者和消费者的消费顺序是否会有影响
    consumer_con_var_.notify_one();
    producer_con_var_.notify_all();
}

template <typename T>
bool BlockDeque<T>::IsEmpty() const noexcept {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    return block_deque_.empty();
}

template <typename T>
bool BlockDeque<T>::IsFull() const noexcept {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    return block_deque_.size() >= deque_capacity_;
}

template <typename T>
size_t BlockDeque<T>::GetDequeSize() const noexcept {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    return block_deque_.size();
}

template <typename T>
size_t BlockDeque<T>::GetDequeCapacity() const noexcept {
    return deque_capacity_;
}

template <typename T>
bool BlockDeque<T>::GetFrontElement(T& element) {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    if (block_deque_.empty()) return false;
    element = block_deque_.front();
    return true;
}

template <typename T>
bool BlockDeque<T>::GetLastElement(T& element) {
    std::lock_guard<std::mutex> locker(deque_mtx_);
    if (block_deque_.empty()) return false;
    element = block_deque_.back();
    return true;
}

template <typename T>
bool BlockDeque<T>::PushFront(T&& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this]() {
        return block_deque_.size() < deque_capacity_ || !is_deque_open_;
    });
    if (!is_deque_open_) return false;
    block_deque_.push_front(std::forward<T>(element));
    consumer_con_var_.notify_one();

    return true;
}

template <typename T>
bool BlockDeque<T>::PushFront(const T& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this] {
        return block_deque_.size() < deque_capacity_ || !is_deque_open_;
    });
    if (!is_deque_open_) return false;
    block_deque_.push_front(element);
    consumer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::PushBack(T&& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this]() {
        return block_deque_.size() < deque_capacity_ || !is_deque_open_;
    });
    if (!is_deque_open_) return false;
    block_deque_.push_back(std::forward<T>(element));
    consumer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::PushBack(const T& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    producer_con_var_.wait(locker, [this] {
        return block_deque_.size() < deque_capacity_ || !is_deque_open_;
    });
    if (!is_deque_open_) return false;
    block_deque_.push_back(element);
    consumer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::PopFront(T& element) {
    std::unique_lock<std::mutex> locker(deque_mtx_);
    consumer_con_var_.wait(locker, [this] {
        return !block_deque_.empty() || !is_deque_open_;
    });
    if (block_deque_.empty() && !is_deque_open_) return false;
    element = std::move(block_deque_.front());
    block_deque_.pop_front();
    producer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::PopFront(T& element, int timeout) {
    auto secs = std::chrono::seconds(timeout);
    std::unique_lock<std::mutex> locker(deque_mtx_);
    if (!consumer_con_var_.wait_for(locker, secs, [this] {return !block_deque_.empty() || !is_deque_open_;})) {
        return false;
    }
    if (block_deque_.empty() && !is_deque_open_) return false;
    element = std::move(block_deque_.front());
    block_deque_.pop_front();
    producer_con_var_.notify_one();
    return true;
}

template <typename T>
void BlockDeque<T>::Flush() {
    consumer_con_var_.notify_one();
}

#endif
