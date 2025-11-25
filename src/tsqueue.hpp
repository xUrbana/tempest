#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class ThreadSafeQueue
{
  public:
    void push(T&& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        condition_.notify_one();
    }

    void pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // Wait until the queue is not empty. If it is, the thread blocks here.
        condition_.wait(lock, [this] { return !queue_.empty(); });

        value = std::move(queue_.front());
        queue_.pop();
    }

  private:
    std::queue<T>           queue_;
    std::mutex              mutex_;
    std::condition_variable condition_;
};