#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class TaskScheduler {
public:
    explicit TaskScheduler(size_t threads_amount);
    ~TaskScheduler();
    void Add(std::function<void()> task, std::time_t timestamp);

private:
    struct Task {
        Task(std::function<void()> task, std::time_t timestamp) : task(task), timestamp(timestamp) {}

        bool operator<(const Task &other) const { return timestamp > other.timestamp; }

        std::function<void()> task;
        std::time_t timestamp;
    };

    void SearchTask();

    std::vector<std::thread> workers_;
    std::condition_variable cv_;
    bool is_object_destroyed_ = false;
    std::mutex flag_mutex_;
    std::priority_queue<Task> tasks_;
    std::mutex priority_queue_mutex_;
};
