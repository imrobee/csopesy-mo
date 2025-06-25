#include "Scheduler.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

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

	// Remove quotes from schedulerType if present
    if (!schedulerType.empty() && schedulerType.front() == '"' && schedulerType.back() == '"') {
        schedulerType = schedulerType.substr(1, schedulerType.size() - 2);
    }

}


void Scheduler::start() {
    running = true;

    coreAvailable.resize(numCores, true);
    for (int i = 0; i < numCores; ++i) {
        cores.emplace_back(&Scheduler::coreWorker, this, i);
    }

    // Now push processes
    for (int i = 1; i <= 10; ++i) {
        std::ostringstream name;
        name << "Process_" << std::setw(2) << std::setfill('0') << i;
        auto process = std::make_shared<Process>(name.str(), 100);
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            jobQueue.push(process);
        }
    }

    cv.notify_all();

    std::thread(&Scheduler::dispatcher, this).detach();
}


void Scheduler::stop() {
    // TODO: Stop scheduler.start() from generating processes
}

void Scheduler::dispatcher() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.notify_all();
    }
}

void Scheduler::coreWorker(int coreId) {
    while (running) {
        std::shared_ptr<Process> proc = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [&] {
                return !jobQueue.empty() || !running;
                });

            if (!running) return;

            // Scheduler selection logic goes here
            if (!jobQueue.empty() && coreAvailable[coreId]) {
				if (schedulerType == "fcfs" or schedulerType == "rr") { //Round Robin is just fcfs with quantum cycles*
                    proc = jobQueue.front();
                    jobQueue.pop();
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
                proc->run(coreId, quantumCycles);
            }
            else {
                proc->run(coreId);
            }

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                finishedProcesses[proc->getName()] = proc;
                runningProcesses.erase(proc->getName());
                coreAvailable[coreId] = true;
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