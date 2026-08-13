#ifndef DBWORKER_H
#define DBWORKER_H

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <string>

#include "Database.h"

class DbWorker {
public:
    DbWorker(const std::string& dbPath = "data/football_standings.db");
    ~DbWorker();

    using TaskFunc = std::function<void(Database&)>;
    using Callback = std::function<void()>;

    // Post a task to run on the worker thread; callback is executed on the main thread
    // when pollCompleted() is called.
    void postTask(TaskFunc task, Callback onComplete = nullptr);

    // Call from the main thread to execute completed callbacks.
    void pollCompleted();

    // Request graceful shutdown.
    void shutdown();

private:
    void threadMain();

    std::thread workerThread;
    std::mutex queueMutex;
    std::condition_variable queueCv;
    std::queue<std::pair<TaskFunc, Callback>> taskQueue;

    std::mutex completedMutex;
    std::queue<Callback> completedQueue;

    std::atomic<bool> running;
    std::string databasePath;
};

#endif
