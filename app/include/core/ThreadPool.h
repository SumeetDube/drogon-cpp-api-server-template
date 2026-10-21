#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <vector>

/**
 * @class ThreadPool
 * @brief Simple fixed-size thread pool built on std::jthread.
 *
 * Tasks run in parallel on the worker threads. On shutdown the pool drains:
 * every task enqueued before stop() still runs before the workers exit.
 */
class ThreadPool
{
public:
    /// @brief Constructs a pool with the given number of worker threads.
    explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());

    /// @brief Drains the queue, then stops and joins all workers.
    ~ThreadPool();

    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /// @brief Submits a task for asynchronous execution. Throws if the pool is stopped.
    void enqueue(std::function<void()> task);

    /// @brief Rejects new tasks, runs the queued ones, then joins the workers. Idempotent.
    void stop();

private:
    /// @brief Worker thread loop; pulls tasks off the queue and runs them outside the lock.
    void workerLoop(std::stop_token st);

    std::queue<std::function<void()>> tasks_; ///< Pending task queue.
    std::mutex mutex_;                        ///< Guards tasks_ and stopping_.
    std::condition_variable_any cv_;          ///< Wakes workers when work arrives.
    bool stopping_ = false;                   ///< Set during shutdown (guarded by mutex_).

    // Declared last so the workers are joined before the queue, mutex and cv are destroyed.
    std::vector<std::jthread> workers_;       ///< Worker threads.
};
