#include "Scheduler.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstdlib>

Scheduler::Scheduler() : numCores(4),
schedulerType("rr"),
quantumCycles(5),
batchFrequency(1),
minInstructions(1000),
maxInstructions(2000),
delayPerExecution(0),
running(false) {
}

void Scheduler::initialize(const std::string& configPath) {
    std::ifstream config(configPath);
    if (!config.is_open()) {
        std::cerr << "Failed to open " << configPath << "\n";
        return;
    }

    std::string line;
    while (std::getline(config, line)) {
        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "num-cpu") {
            iss >> numCores;
        }
        else if (key == "scheduler") {
            iss >> schedulerType;
        }
        else if (key == "quantum-cycles") {
            iss >> quantumCycles;
        }
        else if (key == "batch-process-freq") {
            iss >> batchFrequency;
        }
        else if (key == "min-ins") {
            iss >> minInstructions;
        }
        else if (key == "max-ins") {
            iss >> maxInstructions;
        }
        else if (key == "delay-per-exec") {
            iss >> delayPerExecution;
        }
    }

    // Remove quotes from schedulerType so == can compare properly
    if (!schedulerType.empty() && schedulerType.front() == '"' && schedulerType.back() == '"') {
        schedulerType = schedulerType.substr(1, schedulerType.size() - 2);
    }

}

// Starts the scheduler: spawns core worker threads and the dispatcher thread.
void Scheduler::start(bool withDispatcher) {
    running = true;
    coreAvailable.resize(numCores, true);

    for (int i = 0; i < numCores; ++i) {
        cores.emplace_back(&Scheduler::coreWorker, this, i);
    }

    if (withDispatcher) {
        dispatcherThread = std::thread(&Scheduler::dispatcher, this);
    }

}



void Scheduler::stop() {
    running = false;

    cv.notify_all();

    // Wait for dispatcher to finish
    if (dispatcherThread.joinable()) {
        dispatcherThread.join();
    }

    // Wait for core workers to finish
    for (auto& t : cores) {
        if (t.joinable()) t.join();
    }

    std::cout << "Scheduler stopped.\n";
}



std::vector<Instruction> Scheduler::generateDummyInstructions(int count, int depth) {
    std::vector<Instruction> instructions;
	const int maxDepth = 3; // (depth=the number of nested for loops)

    for (int i = 0; i < count; ++i) {
        int choice = rand() % 6;

        switch (choice) {
        case 0: // PRINT
            instructions.emplace_back(InstructionType::PRINT);
            break;
        case 1: // DECLARE
            instructions.emplace_back(InstructionType::DECLARE, std::vector<std::string>{"var" + std::to_string(i), "10"});
            break;
        case 2: // ADD
            instructions.emplace_back(InstructionType::ADD, std::vector<std::string>{"var" + std::to_string(i), "1", "2"});
            break;
        case 3: // SUBTRACT
            instructions.emplace_back(InstructionType::SUBTRACT, std::vector<std::string>{"var" + std::to_string(i), "5", "3"});
            break;
        case 4: // SLEEP
            instructions.emplace_back(InstructionType::SLEEP, std::vector<std::string>{"1"});
            break;
        case 5: { // FOR loop
            if (depth < maxDepth) {
                int repeats = 2 + rand() % 3;   // repeat 2�4 times (repeats=the number of times for loop will repeat)
                int bodySize = 2 + rand() % 3;  // body has 2�4 instructions (body=the number of instructions in the for loop)
                auto body = generateDummyInstructions(bodySize, depth + 1);
                Instruction forIns(InstructionType::FOR);
                forIns.repeatCount = repeats;
                forIns.body = body;
                instructions.push_back(forIns);
            }
            else {
                // depth limit reached, so just do print
                instructions.emplace_back(InstructionType::PRINT);
            }
            break;
        default:
            instructions.emplace_back(InstructionType::PRINT);
            break;
        }
        }
    }

    return instructions;
}


void Scheduler::dispatcher() {
    int cpuCycles = 0;


    while (running) {
        cpuCycles++;

        if (cpuCycles % batchFrequency == 0) {
            // Add one process on every batchFrequency tick
            int pid = nextProcessId++;
            std::ostringstream name;
            name << "Process_" << std::setw(2) << std::setfill('0') << pid;

            int numInstructions = minInstructions;
            if (maxInstructions > minInstructions) {
                numInstructions += rand() % (maxInstructions - minInstructions + 1);
            }

            auto instructions = generateDummyInstructions(numInstructions);
            auto process = std::make_shared<Process>(pid, name.str(), instructions);

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                readyQueue.push(process);
            }
            cv.notify_all();
        }


    }
}


// Continuously picks processes from the queue and runs them on the assigned core.
void Scheduler::coreWorker(int coreId) {
    while (running) {
        std::shared_ptr<Process> proc = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [&] {
                return (!readyQueue.empty() && coreAvailable[coreId]) || !running;
                });


            if (!running) return;

            // Scheduler selection logic goes here
            if (!readyQueue.empty() && coreAvailable[coreId]) {
                if (schedulerType == "fcfs" or schedulerType == "rr") { //Round Robin is just fcfs with quantum cycles*
                    proc = readyQueue.front();
                    readyQueue.pop();
                    cv.notify_all();
                }
                else {
                    std::cerr << "Unsupported scheduler: " << schedulerType << "\n";
                }

                coreAvailable[coreId] = false;
                runningProcesses[proc->getName()] = proc;
            }
        }

        if (proc) {
            if (schedulerType == "rr") {
                proc->run(coreId, delayPerExecution, quantumCycles, running);
            }
            else {
                proc->run(coreId, delayPerExecution, 0, running);
            }

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                finishedProcesses[proc->getName()] = proc;
                runningProcesses.erase(proc->getName());
                coreAvailable[coreId] = true;
                cv.notify_all();
            }
        }
    }
}

void Scheduler::printStatus() {
    int usedCores = 0;
    for (bool available : coreAvailable) {
        if (!available) ++usedCores;
    }

    std::cout << "CPU Utilization: " << (usedCores * 100 / numCores) << "%\n";
    std::cout << "Cores Used: " << usedCores << "\n";
    std::cout << "Cores Available: " << (numCores - usedCores) << "\n";
    std::cout << "________________________________________________________\n\n";

    std::cout << "Running processes:\n\n";
    for (const auto& [name, proc] : runningProcesses) {
        std::cout << name << " | (" << proc->getTimestamp() << ") | ";
        std::cout << "Core:" << proc->getAssignedCore() << " | ";
        std::cout << proc->getCurrentLine() << " / " << proc->getTotalLines() << "\n\n";
    }

    std::cout << "Finished processes:\n\n";
    for (const auto& [name, proc] : finishedProcesses) {
        std::cout << name << " | (" << proc->getTimestamp() << ") | ";
        std::cout << "Finished | " << proc->getTotalLines() << "/" << proc->getTotalLines() << "\n\n";
    }

    std::cout << "________________________________________________________\n";
}

void Scheduler::writeStatusToFile() {
    std::ofstream outFile("csopesy-log.txt");

    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open csopesy-log.txt for writing.\n";
        return;
    }

    int usedCores = 0;
    for (bool available : coreAvailable) {
        if (!available) ++usedCores;
    }

    outFile << "CPU Utilization: " << (usedCores * 100 / numCores) << "%\n";
    outFile << "Cores Used: " << usedCores << "\n";
    outFile << "Cores Available: " << (numCores - usedCores) << "\n";
    outFile << "________________________________________________________\n\n";

    outFile << "Running processes:\n\n";
    for (const auto& [name, proc] : runningProcesses) {
        outFile << name << " | (" << proc->getTimestamp() << ") | ";
        outFile << "Core:" << proc->getAssignedCore() << " | ";
        outFile << proc->getCurrentLine() << " / " << proc->getTotalLines() << "\n\n";
    }

    outFile << "Finished processes:\n\n";
    for (const auto& [name, proc] : finishedProcesses) {
        outFile << name << " | (" << proc->getTimestamp() << ") | ";
        outFile << "Finished | " << proc->getTotalLines() << "/" << proc->getTotalLines() << "\n\n";
    }

    outFile << "________________________________________________________\n";

    outFile.close();

    std::cout << "Report generated at csopesy-log.txt\n";
}

void Scheduler::viewConfig() {
    std::cout << "Current Scheduler Configuration:\n";
    std::cout << "Number of Cores: " << numCores << "\n";
    std::cout << "Scheduler Type: " << schedulerType << "\n";
    std::cout << "Quantum Cycles: " << quantumCycles << "\n";
    std::cout << "Batch Process Frequency: " << batchFrequency << "\n";
    std::cout << "Min Instructions: " << minInstructions << "\n";
    std::cout << "Max Instructions: " << maxInstructions << "\n";
    std::cout << "Delay Per Execution: " << delayPerExecution << "\n";
}

void Scheduler::createManualProcess(const std::string& processName) {
    int numInstructions = minInstructions;
    if (maxInstructions > minInstructions) {
        numInstructions += rand() % (maxInstructions - minInstructions + 1);
    }

    int pid = nextProcessId++;
    auto instructions = generateDummyInstructions(numInstructions);
    auto process = std::make_shared<Process>(pid, processName, instructions);

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(process);
    }
    cv.notify_all();

    std::cout << "Process " << processName << " created and added to the queue.\n";

    // Start the cores only (without dispatcher) if not already running
    if (!running) {
        start(false);
    }
}

std::shared_ptr<Process> Scheduler::findProcessByName(const std::string& processName) {
    std::lock_guard<std::mutex> lock(queueMutex);
    auto itRunning = runningProcesses.find(processName);
    if (itRunning != runningProcesses.end()) {
        return itRunning->second;
    }
    auto itFinished = finishedProcesses.find(processName);
    if (itFinished != finishedProcesses.end()) {
        return itFinished->second;
    }
    return nullptr;
}
