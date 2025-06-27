#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include "Instruction.h"

class Process {
public:
    Process(const std::string& name, const std::vector<Instruction>& instructions);

    void run(int coreId, int delayPerExecution, int quantum, std::atomic<bool>& running); // quantum = 0 for fcfs, >0 for round-robin
    std::string getTimestamp() const;
    std::string getName() const;
    int getAssignedCore() const;
    int getCurrentLine() const;
    int getTotalLines() const;

    std::vector<std::string> getLogs() const { return logs; }
private:
    std::vector<std::string> logs;

    std::string name;
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, uint16_t> memory;
    int currentLine;
    int assignedCore;

    void executeInstruction(const Instruction& ins);
    uint16_t getValue(const std::string& arg);
    

};
