/**
 * @file timer.cpp
 * @author chenyinjie
 * @date 2024-09-12
 * @copyright Apache 2.0
 */

#include "timer.h"

TimerHeap::TimerHeap() {
    timer_heap_.reserve(64);
    LOG_INFO("Timer: Init Timer Heap Success.");
}

TimerHeap::~TimerHeap() {
    ClearAllTimers();
}

void TimerHeap::AddTimer(int id, int timeout, const TimeoutCallBackFunc& cb_f) {
    if (id_maps_.contains(id)) {
        LOG_ERROR("Add Timer Failed: Timer With id: %d Already Exists.", id);
        return;
    } 
    TimeStamp exipres = Clock::now() + MS(timeout);
    timer_heap_.emplace_back(id, exipres, cb_f);
    id_maps_[id] = timer_heap_.size() - 1;
    HeapifyUp(timer_heap_.size() - 1);
}

void TimerHeap::UpdateTimer(int id, int new_expire) {
    if (!id_maps_.contains(id)) {
        LOG_ERROR("Update Timer Failed: Timer With id: %d Not Exists.", id);
        return;
    }
    int idx = id_maps_[id];
    TimeStamp old_expires = timer_heap_[idx]._expire;
    // 更新到期时间
    timer_heap_[idx]._expire = Clock::now() + MS(new_expire);
    if (timer_heap_[idx]._expire > old_expires) {
        HeapifyDown(idx, timer_heap_.size());
    } else {
        HeapifyUp(idx);
    }
}

void TimerHeap::CBWorker(int id) {
    if (timer_heap_.empty() || !id_maps_.contains(id)) {
        LOG_WARN("Timer CallBack Worker Failed: CallBack Worker Called Invalid.");
        return;
    }
    size_t idx = id_maps_[id];
    Timer timer = timer_heap_[idx];
    timer._callback_func();
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
        if (std::chrono::duration_cast<MS>(timer._expire - Clock::now()).count() > 0) break;
        timer._callback_func();
        RemoveTopTimer();
    }
}

void TimerHeap::RemoveTopTimer() {
    if (timer_heap_.empty()) {
        LOG_ERROR("Timer: Attempted To Temove Top Timer, But Timer Heap is Empty.");
        return;
    }
    RemoveTimer(0);
}

int TimerHeap::GetNextExpireTime() {
    CleanExpiredTimer();
    if (timer_heap_.empty()) return -1;
    int t = std::chrono::duration_cast<MS>(timer_heap_.front()._expire - Clock::now()).count();

    return t < 0 ? 0 : t;
}

void TimerHeap::RemoveTimer(size_t idx) {
    // 删除指定堆索引的timer
    if (idx >= timer_heap_.size()) {
        LOG_ERROR("Timer: Attempt To Remove Invalid Timer Heap idx.");
        return;
    }
    size_t last_idx = timer_heap_.size() - 1;
    SwapTimers(idx, last_idx);
    id_maps_.erase(timer_heap_[last_idx]._id);
    timer_heap_.pop_back();
    if (idx < last_idx) {
        if (idx > 0 && (timer_heap_[idx]._expire < timer_heap_[(idx - 1) / 2]._expire)) {
            HeapifyUp(idx);
        } else {
            HeapifyDown(idx, timer_heap_.size());
        }
    }
}

void TimerHeap::HeapifyUp(size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (timer_heap_[parent]._expire <= timer_heap_[idx]._expire) break;
        SwapTimers(parent, idx);
        idx = parent;
    }
}

void TimerHeap::HeapifyDown(size_t idx, size_t n) {
    while (idx < n) {
        size_t min_idx = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        if (left < n && timer_heap_[left]._expire < timer_heap_[min_idx]._expire) min_idx = left;
        if (right < n && timer_heap_[right]._expire < timer_heap_[min_idx]._expire) min_idx = right;
        if (min_idx == idx) break; 
        SwapTimers(idx, min_idx);
        idx = min_idx;
    }
}

void TimerHeap::SwapTimers(size_t i, size_t j) {
    // 交换对应索引下的两个Timer
    std::swap(timer_heap_[i], timer_heap_[j]);
    // 更新映射关系
    id_maps_[timer_heap_[i]._id] = i;
    id_maps_[timer_heap_[j]._id] = j;
}
