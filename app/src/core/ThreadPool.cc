#include "core/ThreadPool.h"

#include <drogon/drogon.h>
#include <stdexcept>

ThreadPool::ThreadPool(size_t threadCount)
{
    if (threadCount == 0) threadCount = 1;

    LOG_INFO << "Starting threadpool with [" << threadCount << "] threads";

    workers_.reserve(threadCount);
    for (size_t i = 0; i < threadCount; i++)
    {
        workers_.emplace_back(std::bind_front(&ThreadPool::workerLoop, this));
    }
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::lock_guard lock(mutex_);
        if (stopping_)
        {
            throw std::runtime_error("ThreadPool::enqueue called after stop()");
        }
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

void ThreadPool::stop()
{
    {
        // Set the flag under the lock: otherwise a worker could check the predicate,
        // miss the update and go to sleep after notify_all() has already fired.
        std::lock_guard lock(mutex_);
        if (stopping_) return;
        stopping_ = true;
    }
    LOG_INFO << "Stopping threadpool";
    cv_.notify_all();
    workers_.clear(); // jthread destructors request stop and join
}

void ThreadPool::workerLoop(std::stop_token st)
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock lock(mutex_);

            // Returns when there is work, on stop(), or when this jthread is asked to stop.
            cv_.wait(lock, st, [&]() { return stopping_ || !tasks_.empty(); });

            // Drain before exiting: only leave once the queue is empty.
            if (tasks_.empty()) break;

            task = std::move(tasks_.front());
            tasks_.pop();
        } // lock released here, so tasks run in parallel

        try
        {
            task();
        }
        catch (const std::exception& error)
        {
            LOG_ERROR << "ThreadPool task exception: " << error.what();
        }
        catch (...)
        {
            LOG_ERROR << "ThreadPool task unknown exception";
        }
    }
}
