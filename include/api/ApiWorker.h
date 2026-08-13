#ifndef APIWORKER_H
#define APIWORKER_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

// Runs blocking API work away from the UI thread. Each task returns a callback
// that is later executed by pollCompleted() on the main/UI thread.
class ApiWorker {
public:
    using Callback = std::function<void()>;
    using TaskFunc = std::function<Callback()>;

    ApiWorker();
    ~ApiWorker();

    void postTask(TaskFunc task);
    void pollCompleted();
    void shutdown();

private:
    void threadMain();

    std::thread workerThread;
    std::mutex queueMutex;
    std::condition_variable queueCv;
    std::queue<TaskFunc> taskQueue;

    std::mutex completedMutex;
    std::queue<Callback> completedQueue;
    std::atomic<bool> running;
};

#endif
