#pragma once
#include "Process.h"
#include <thread>
#include <vector>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <string>
#include <memory>
#include <atomic>

class Scheduler {
public:
    Scheduler();
    void initialize(const std::string& configPath);
    void start();           
    void stop();          
    void dispatcher();       
    void coreWorker(int coreId);
    void printStatus();
    void writeStatusToFile();
    void viewConfig();
    void createManualProcess(const std::string& processName);


    std::map<std::string, std::shared_ptr<Process>> runningProcesses;
    std::map<std::string, std::shared_ptr<Process>> finishedProcesses;

private:
    int numCores;
    std::string schedulerType;
    int quantumCycles;
    int batchFrequency;
    int minInstructions;
    int maxInstructions;
    int delayPerExecution;

    std::atomic<bool> running;

    std::vector<std::thread> cores;   
    std::thread dispatcherThread;         

    std::vector<bool> coreAvailable; 

    std::queue<std::shared_ptr<Process>> readyQueue;   
    

    std::mutex queueMutex;
    std::condition_variable cv;

    std::vector<Instruction> generateDummyInstructions(int count);

};
