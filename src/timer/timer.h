/**
 * @file timer.h
 * @author chenyinjie
 * @date 2024-09-12
 * @copyright Apache 2.0
 */

#ifndef TIMER_H
#define TIMER_H

#include "../log/log.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <chrono>

using TimeoutCallBackFunc = std::function<void()>;
using Clock = std::chrono::steady_clock;
using MS = std::chrono::milliseconds;
using TimeStamp = std::chrono::steady_clock::time_point;

// 定时器结构
struct Timer {
    int _id;
    TimeStamp _expire;
    TimeoutCallBackFunc _callback_func;
    Timer(int id, TimeStamp expire, TimeoutCallBackFunc cb_f)
        : _id(id), _expire(expire), _callback_func(cb_f) {};
};

class TimerHeap {
public:
    TimerHeap();
    ~TimerHeap();

    void AddTimer(int id, int timeout, const TimeoutCallBackFunc& cb_f);// 添加定时器
    void UpdateTimer(int id, int new_expire);                           // 重新设置定时器
    void CBWorker(int id);                                              // 执行定时器绑定的回调函数
    void CleanExpiredTimer();                                           // 清理到期计时器
    void RemoveTopTimer();                                              // 移除最早到期定时器
    void ClearAllTimers();                                              // 清空所以定时器
    int GetNextExpireTime();                                            // 获取当前未到期的最早定时器时间点

private:
    void RemoveTimer(size_t idx);                                       // 移除堆中指定索引的定时器
    void HeapifyUp(size_t idx);                                         // 向堆顶更新堆
    void HeapifyDown(size_t idx, size_t n);                             // 向堆底更新堆
    void SwapTimers(size_t i, size_t j);                                // 交换指定堆索引的两个定时器

    std::vector<Timer> timer_heap_;                                     // 按定时器到期时间排序的小顶堆
    std::unordered_map<int, size_t> id_maps_;                           // 记录定时器id到数组索引idx的映射关系
};

#endif