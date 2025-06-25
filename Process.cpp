#include "Process.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <filesystem>

Process::Process(const std::string& name, int totalLines)
    : name(name), totalLines(totalLines), currentLine(0), assignedCore(-1) {
    timestamp = getCurrentTimestamp();
}

void Process::run(int coreId) {
    assignedCore = coreId;

    // Ensure the "processes" folder exists
    std::filesystem::create_directory("processes");

    std::ofstream log(name + ".txt", std::ios::app);

    while (currentLine < totalLines) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // simulate work

        std::string timeStr = getCurrentTimestamp();

        {
            std::lock_guard<std::mutex> lock(mtx);
            ++currentLine;
            if (log.is_open()) {
                log << "(" << timeStr << ") Core: " << coreId
                    << "  \"Hello world from " << name << "!\"\n";
            }
        }
    }

    log.close();
}

void Process::run(int coreId, int quantumCycles) {
    assignedCore = coreId;

    std::ofstream log(name + ".txt", std::ios::app);
    int executed = 0;

    while (currentLine < totalLines && executed < quantumCycles) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Simulate execution
        ++currentLine;
        ++executed;

        if (log.is_open()) {
            log << "Executed line " << currentLine << " on core " << coreId << "\n";
        }
    }

    log.close();
}


const std::string& Process::getName() const {
    return name;
}

int Process::getCurrentLine() const {
    return currentLine;
}

int Process::getTotalLines() const {
    return totalLines;
}

int Process::getAssignedCore() const {
    return assignedCore;
}

std::string Process::getTimestamp() const {
    return timestamp;
}

std::string Process::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm parts;

    localtime_s(&parts, &now_c);

    std::ostringstream oss;
    oss << std::put_time(&parts, "%m/%d/%Y %I:%M:%S%p");
    return oss.str();
}
