/**
 * @file timer.cpp
 * @author chenyinjie
 * @date 2024-09-12
 */

#include "timer.h"

TimerHeap::TimerHeap() {
    timer_heap_.reserve(64);
    is_log_open = Log::getInstance().isOpen();
}

TimerHeap::~TimerHeap() {
    ClearAllTimers();
}

void TimerHeap::AddTimer(int id, int timeout, const TimeoutCallBack& cb_f) {
    if (id_maps_.contains(id)) {
        if (is_log_open) {
            LOG_ERROR("Timer with id: %d already exists!", id);
        } else {
            std::cerr << "Timer with id: " << id << " already exists!" << std::endl;
        }
        return;
    } 
    TimeStamp exipres = Clock::now() + MS(timeout);
    timer_heap_.emplace_back(id, exipres, cb_f);
    id_maps_[id] = timer_heap_.size() - 1;
    HeapifyUp(timer_heap_.size() - 1);
}

void TimerHeap::UpdateTimer(int id, int new_period) {
    if (!id_maps_.contains(id)) {
        if (is_log_open) {
            LOG_ERROR("Timer with id: %d not found!", id);
        } else {
            std::cerr << "Timer with id: " << id << " not found!" << std::endl;
        }
        return;
    }
    int idx = id_maps_[id];
    TimeStamp old_expires = timer_heap_[idx].expires_;
    // 更新到期时间
    timer_heap_[idx].expires_ = Clock::now() + MS(new_period);
    if (timer_heap_[idx].expires_ > old_expires) {
        // 时间延长
        HeapifyDown(idx, timer_heap_.size());
    } else {
        HeapifyUp(idx);
    }
}

void TimerHeap::Worker(int id) {
    if (timer_heap_.empty() || !id_maps_.contains(id)) {
        if (is_log_open) {
            LOG_WARN("Worker called invalid");
        } else {
            std::cerr << "Worker called invalid" << std::endl;
        }
        return;
    }
    size_t idx = id_maps_[id];
    Timer timer = timer_heap_[idx];
    timer.callback_f_();
    RemoveTimer(idx);
}

void TimerHeap::ClearAllTimers() {
    timer_heap_.clear();
    id_maps_.clear();
}

void TimerHeap::CleanExpiredTimer() {
    if (timer_heap_.empty()) return;
    while (!timer_heap_.empty()) {
        Timer timer = timer_heap_.front();
        if (std::chrono::duration_cast<MS>(timer.expires_ - Clock::now()).count() > 0) break;
        timer.callback_f_();
        RemoveTopTimer();
    }
}

void TimerHeap::RemoveTopTimer() {
    if (timer_heap_.empty()) {
        if (is_log_open) {
            LOG_ERROR("Attempted to remove top timer, but timer heap is empty.");
        } else {
            std::cerr << "Attempted to remove top timer, but timer heap is empty." << std::endl;
        }
        return;
    }
    RemoveTimer(0);
}

int TimerHeap::GetNextTimerExpireTime() {
    CleanExpiredTimer();
    if (timer_heap_.empty()) return -1;
    int t = std::chrono::duration_cast<MS>(timer_heap_.front().expires_ - Clock::now()).count();

    return t < 0 ? 0 : t;
}

void TimerHeap::RemoveTimer(size_t idx) {
    // 删除指定堆索引的timer
    if (idx >= timer_heap_.size()) {
        if (is_log_open) {
            LOG_ERROR("Remove idx invalid.");
        } else {
            std::cerr << "Remove idx invalid." << std::endl;
        }
        return;
    }
    size_t last_idx = timer_heap_.size() - 1;
    if (idx < last_idx) {
        SwapTimers(idx, last_idx);
        id_maps_.erase(timer_heap_[last_idx].id_);
        timer_heap_.pop_back();
        HeapifyDown(idx, timer_heap_.size());
    } else {
        id_maps_.erase(timer_heap_[last_idx].id_);
        timer_heap_.pop_back();
    }
}

void TimerHeap::HeapifyUp(size_t idx) {
  while (idx > 0) {
    size_t parent = (idx - 1) / 2;
    if (timer_heap_[parent].expires_ <= timer_heap_[idx].expires_)
      break;
    SwapTimers(parent, idx);
    idx = parent;
  }
}

void TimerHeap::HeapifyDown(size_t idx, size_t n) {
    while (idx < n) {
        size_t min_idx = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        if (left < n && timer_heap_[left].expires_ < timer_heap_[min_idx].expires_) min_idx = left;
        if (right < n && timer_heap_[right].expires_ < timer_heap_[min_idx].expires_) min_idx = right;
        if (min_idx == idx) break; 
        SwapTimers(idx, min_idx);
        idx = min_idx;
    }
}

void TimerHeap::SwapTimers(size_t i, size_t j) {
    // 交换对应索引下的两个Timer
    std::swap(timer_heap_[i], timer_heap_[j]);
    // 更新映射关系
    id_maps_[timer_heap_[i].id_] = i;
    id_maps_[timer_heap_[j].id_] = j;
}
