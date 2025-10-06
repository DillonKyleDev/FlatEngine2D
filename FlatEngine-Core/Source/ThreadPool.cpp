#include "ThreadPool.h"



namespace FlatEngine
{
    ThreadPool::ThreadPool(size_t num_threads)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            m_threads.emplace_back([this]
            {
                while (true) // Thread purgatory
                {
                    std::function<void()> task;
                    // The reason for putting the below code
                    // here is to unlock the queue before
                    // executing the task so that other
                    // threads can perform enqueue tasks
                    {
                        // Locking the queue so that data can be shared safely
                        std::unique_lock<std::mutex> lock(m_queueMutex);

                        // Waiting until there is a task to execute or the pool is stopped
                        m_conditionVariable.wait(lock, [this]
                        {
                            return !m_tasks.empty() || m_b_stop;
                        });

                        // exit the thread in case the pool is stopped and there are no tasks
                        if (m_b_stop && m_tasks.empty())
                        {
                            return;
                        }

                        // Get the next task from the queue
                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    ThreadPool::~ThreadPool()
    {
        {
            // Lock the queue to update the stop flag safely
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_b_stop = true;
        }

        // Notify all threads
        m_conditionVariable.notify_all();

        // Joining all worker threads to ensure they have completed their tasks
        for (auto& thread : m_threads)
        {
            thread.join();
        }
    }


    void ThreadPool::EnqueueTask(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_tasks.emplace(move(task));
        }

        m_conditionVariable.notify_one();
    }

    void ThreadPool::JoinTasks()
    {
        for (auto& thread : m_threads)
        {
            thread.join();
        }
    }
}