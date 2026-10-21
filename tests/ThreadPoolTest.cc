#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

#include "core/ThreadPool.h"

using namespace std::chrono_literals;

// Every task waits until all N tasks are running at the same time. If the pool ran
// tasks one at a time (e.g. while holding its queue lock) the first task would time out.
TEST(ThreadPool, RunsTasksInParallel)
{
    constexpr int kThreads = 4;

    std::mutex mutex;
    std::condition_variable cv;
    int running = 0;
    std::atomic<int> sawAllRunning{0};

    {
        ThreadPool pool(kThreads);
        for (int i = 0; i < kThreads; i++)
        {
            pool.enqueue([&]() {
                std::unique_lock lock(mutex);
                running++;
                cv.notify_all();
                if (cv.wait_for(lock, 2s, [&]() { return running == kThreads; }))
                {
                    sawAllRunning++;
                }
            });
        }
    } // destructor drains and joins

    EXPECT_EQ(sawAllRunning.load(), kThreads);
}

TEST(ThreadPool, DrainsQueuedTasksOnDestruction)
{
    constexpr int kTasks = 1000;
    std::atomic<int> done{0};

    {
        ThreadPool pool(2);
        for (int i = 0; i < kTasks; i++)
        {
            pool.enqueue([&]() { done++; });
        }
    }

    EXPECT_EQ(done.load(), kTasks);
}

TEST(ThreadPool, ThrowingTaskDoesNotKillWorker)
{
    std::atomic<bool> ranAfterThrow{false};

    {
        ThreadPool pool(1);
        pool.enqueue([]() { throw std::runtime_error("boom"); });
        pool.enqueue([]() { throw 42; });
        pool.enqueue([&]() { ranAfterThrow = true; });
    }

    EXPECT_TRUE(ranAfterThrow.load());
}

TEST(ThreadPool, EnqueueAfterStopThrows)
{
    ThreadPool pool(1);
    pool.stop();
    pool.stop(); // idempotent

    EXPECT_THROW(pool.enqueue([]() {}), std::runtime_error);
}

TEST(ThreadPool, ZeroThreadsStillRunsTasks)
{
    std::atomic<bool> ran{false};

    {
        ThreadPool pool(0);
        pool.enqueue([&]() { ran = true; });
    }

    EXPECT_TRUE(ran.load());
}
