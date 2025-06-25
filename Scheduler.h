#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <map>
#include <memory>
#include "Process.h"

class Scheduler {
public:
    Scheduler();

    void initialize(const std::string& configPath);
    void start();
    void stop();
    void printStatus();
    void writeStatusToFile();
	void viewConfig();

private:
    // configs defined in config.txt
    int numCores;
    std::string schedulerType;
    int quantumCycles;
    int batchFrequency;
    int minInstructions;
    int maxInstructions;
    int delayPerExecution;

    // others
    std::vector<std::thread> cores;
    std::vector<bool> coreAvailable;
    std::atomic<bool> running;

    std::queue<std::shared_ptr<Process>> jobQueue;
    std::map<std::string, std::shared_ptr<Process>> runningProcesses;
    std::map<std::string, std::shared_ptr<Process>> finishedProcesses;

    std::mutex queueMutex;
    std::condition_variable cv;

    void dispatcher();
    void coreWorker(int coreId);
};

#endif