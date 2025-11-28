#include "tsqueue.hpp"
#include <gtest/gtest.h>
#include <thread>

TEST(ThreadSafeQueueTest, PushPop)
{
    ThreadSafeQueue<int> queue;
    queue.push(42);

    int value = 0;
    queue.pop(value);
    EXPECT_EQ(value, 42);
}

TEST(ThreadSafeQueueTest, FIFO)
{
    ThreadSafeQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);

    int value;
    queue.pop(value);
    EXPECT_EQ(value, 1);
    queue.pop(value);
    EXPECT_EQ(value, 2);
    queue.pop(value);
    EXPECT_EQ(value, 3);
}

TEST(ThreadSafeQueueTest, ConcurrentAccess)
{
    ThreadSafeQueue<int>     queue;
    const int                num_threads = 10;
    const int                ops_per_thread = 100;
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::atomic<int>         sum_consumed = 0;

    for (int i = 0; i < num_threads; ++i)
    {
        producers.emplace_back(
          [&queue]
          {
              for (int j = 0; j < ops_per_thread; ++j)
              {
                  queue.push(1);
              }
          });
    }

    for (int i = 0; i < num_threads; ++i)
    {
        consumers.emplace_back(
          [&queue, &sum_consumed]
          {
              for (int j = 0; j < ops_per_thread; ++j)
              {
                  int val;
                  queue.pop(val);
                  sum_consumed += val;
              }
          });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    EXPECT_EQ(sum_consumed, num_threads * ops_per_thread);
}
