#include "TaskScheduler.hpp"

TaskScheduler::TaskScheduler(size_t threads_amount) {
    for (size_t i = 0; i < threads_amount; ++i) {
        workers_.emplace_back(&TaskScheduler::SearchTask, this);
    }
}

TaskScheduler::~TaskScheduler() {
    {
        std::lock_guard<std::mutex> lock(flag_mutex_);
        is_object_destroyed_ = true;
    }
    cv_.notify_all();

    for (std::thread &worker: workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void TaskScheduler::Add(std::function<void()> task, std::time_t timestamp) {
    {
        std::lock_guard<std::mutex> lock(priority_queue_mutex_);
        tasks_.emplace(task, timestamp);
    }
    cv_.notify_one();
}

void TaskScheduler::SearchTask() {
    while (true) {
        std::unique_lock<std::mutex> queue_lock(priority_queue_mutex_);
        cv_.wait(queue_lock, [this]() { return is_object_destroyed_ || !tasks_.empty(); });

        if (is_object_destroyed_ && tasks_.empty()) {
            return;
        }

        while (!tasks_.empty()) {
            auto now = std::time(nullptr);
            Task earliest_task = tasks_.top();

            if (earliest_task.timestamp <= now) {
                tasks_.pop();
                queue_lock.unlock();
                earliest_task.task();
                queue_lock.lock();
            } else {
                cv_.wait_until(queue_lock, std::chrono::system_clock::from_time_t(earliest_task.timestamp));
            }
        }
    }
}
