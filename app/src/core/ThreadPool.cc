#include "core/ThreadPool.h"

#include <drogon/drogon.h>

ThreadPool::ThreadPool(size_t threadCount)
{
    if (threadCount == 0) threadCount = 1;

    LOG_INFO << "Starting threadpool with [" << threadCount << "] threads";

    for (size_t i = 0; i < threadCount; i++)
    {
        workers_.emplace_back(std::bind_front(&ThreadPool::workerLoop, this));
    }
}

ThreadPool::~ThreadPool()
{
    LOG_INFO << "Killing threadpool";
    stop();
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::lock_guard lock(mutex_);
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

void ThreadPool::stop()
{
    LOG_INFO << "Stopping threadpool";
    stopping_ = true;
    cv_.notify_all();
    workers_.clear();
}

void ThreadPool::workerLoop(std::stop_token st)
{
    while (!st.stop_requested())
    {
        std::function<void()> task;
        {
            std::unique_lock lock(mutex_);

            cv_.wait(lock, [&]() { return stopping_ || !tasks_.empty() || st.stop_requested(); });

            if ((stopping_ || st.stop_requested()) && tasks_.empty()) break;

            if (!tasks_.empty())
            {
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            else
            {
                continue;
            }

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
}
