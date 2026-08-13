#include "api/ApiWorker.h"

#include <iostream>

ApiWorker::ApiWorker() : running(true) {
    workerThread = std::thread(&ApiWorker::threadMain, this);
}

ApiWorker::~ApiWorker() {
    shutdown();
}

void ApiWorker::postTask(TaskFunc task) {
    {
        std::lock_guard<std::mutex> lg(queueMutex);
        taskQueue.push(std::move(task));
    }
    queueCv.notify_one();
}

void ApiWorker::pollCompleted() {
    std::queue<Callback> local;
    {
        std::lock_guard<std::mutex> lg(completedMutex);
        std::swap(local, completedQueue);
    }

    while (!local.empty()) {
        Callback callback = std::move(local.front());
        local.pop();
        if (callback) callback();
    }
}

void ApiWorker::shutdown() {
    bool expected = true;
    if (!running.compare_exchange_strong(expected, false)) return;

    queueCv.notify_all();
    if (workerThread.joinable()) workerThread.join();
    pollCompleted();
}

void ApiWorker::threadMain() {
    while (running) {
        TaskFunc task;
        {
            std::unique_lock<std::mutex> ul(queueMutex);
            queueCv.wait(ul, [this]() { return !taskQueue.empty() || !running.load(); });
            if (!running && taskQueue.empty()) break;
            task = std::move(taskQueue.front());
            taskQueue.pop();
        }

        try {
            Callback callback = task();
            if (callback) {
                std::lock_guard<std::mutex> lg(completedMutex);
                completedQueue.push(std::move(callback));
            }
        } catch (const std::exception& e) {
            std::cerr << "ApiWorker task exception: " << e.what() << std::endl;
        }
    }
}
