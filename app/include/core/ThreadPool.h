#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

/**
 * @class ThreadPool
 * @brief Simple fixed-size thread pool built on std::jthread.
 */
class ThreadPool
{
public:
    /// @brief Constructs a pool with the given number of worker threads.
    explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());

    /// @brief Stops all workers and joins them.
    ~ThreadPool();

    /// @brief Submits a task for asynchronous execution.
    void enqueue(std::function<void()> task);

    /// @brief Signals workers to exit and joins them.
    void stop();

private:
    /// @brief Worker thread loop; pulls tasks off the queue and runs them.
    void workerLoop(std::stop_token st);

    std::vector<std::jthread> workers_;       ///< Worker threads.
    std::queue<std::function<void()>> tasks_; ///< Pending task queue.
    std::mutex mutex_;                        ///< Guards the task queue.
    std::condition_variable_any cv_;          ///< Wakes workers when work arrives.
    std::atomic<bool> stopping_{false};       ///< Set during shutdown.
};
