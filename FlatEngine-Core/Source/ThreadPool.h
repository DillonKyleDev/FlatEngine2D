#pragma once
// Resource used for thread pooling: https://www.geeksforgeeks.org/cpp/thread-pool-in-cpp/

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>


namespace FlatEngine
{
	class ThreadPool
	{
    public:
        ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
        ~ThreadPool();

        void EnqueueTask(std::function<void()> task);
        void JoinTasks();

    private:
        std::vector<std::thread> m_threads;
        std::queue<std::function<void()>> m_tasks;
        std::mutex m_queueMutex;                      // Mutex to synchronize access to shared data
        std::condition_variable m_conditionVariable;  // Condition variable to signal changes in the state of the tasks queue
        bool m_b_stop = false;
	};
}

