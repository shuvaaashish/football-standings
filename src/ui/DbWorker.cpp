#include "DbWorker.h"
#include <iostream>

DbWorker::DbWorker(const std::string& dbPath)
    : running(true), databasePath(dbPath) {
    workerThread = std::thread(&DbWorker::threadMain, this);
}

DbWorker::~DbWorker() {
    shutdown();
}

void DbWorker::postTask(TaskFunc task, Callback onComplete) {
    {
        std::lock_guard<std::mutex> lg(queueMutex);
        taskQueue.emplace(std::move(task), std::move(onComplete));
    }
    queueCv.notify_one();
}

void DbWorker::pollCompleted() {
    std::queue<Callback> local;
    {
        std::lock_guard<std::mutex> lg(completedMutex);
        std::swap(local, completedQueue);
    }

    while (!local.empty()) {
        Callback cb = std::move(local.front());
        local.pop();
        if (cb) {
            try {
                cb();
            } catch (const std::exception& e) {
                std::cerr << "DbWorker: exception in callback: " << e.what() << std::endl;
            }
        }
    }
}

void DbWorker::shutdown() {
    bool expected = true;
    if (!running.compare_exchange_strong(expected, false)) return;

    queueCv.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }

    // Execute any remaining completed callbacks (safe to run here)
    pollCompleted();
}

void DbWorker::threadMain() {
    // Create a Database instance local to this thread to avoid sharing sqlite handles across threads.
    Database db(databasePath);

    while (running) {
        std::pair<TaskFunc, Callback> task;
        {
            std::unique_lock<std::mutex> ul(queueMutex);
            queueCv.wait(ul, [this]() { return !taskQueue.empty() || !running.load(); });
            if (!running && taskQueue.empty()) break;
            task = std::move(taskQueue.front());
            taskQueue.pop();
        }

        try {
            if (task.first) task.first(db);

            if (task.second) {
                std::lock_guard<std::mutex> lg(completedMutex);
                completedQueue.push(task.second);
            }
        } catch (const std::exception& e) {
            std::cerr << "DbWorker: exception in task: " << e.what() << std::endl;
            // push a callback that logs the error (if provided callback exists it won't be called)
            std::lock_guard<std::mutex> lg(completedMutex);
            completedQueue.push([msg = std::string("DbWorker task exception: ") + e.what()](){ std::cerr << msg << std::endl; });
        }
    }
}
