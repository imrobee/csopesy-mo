#include "Process.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <algorithm>

Process::Process(int id, const std::string& name, const std::vector<Instruction>& instructions)
    : id(id), name(name), instructions(instructions), currentLine(0), assignedCore(-1)
{
    creationTimestamp = generateTimestamp();
}

// Generate timestamp at creation
std::string Process::generateTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm buf{};
#ifdef _WIN32
    localtime_s(&buf, &t);
#else
    localtime_r(&t, &buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&buf, "%m/%d/%Y %H:%M:%S");
    return oss.str();
}

// Return the saved creation timestamp
std::string Process::getTimestamp() const {
    return creationTimestamp;
}

// Runs the process; ends when stops running so scheduler.stop() doesn't need to wait for it
void Process::run(int coreId, int delayPerExecution, int quantum, std::atomic<bool>& running) {
    assignedCore = coreId;
    int cycles = 0;

    while (currentLine < instructions.size()) {
        if (!running) break;

        executeInstruction(instructions[currentLine]);
        ++currentLine;
        ++cycles;

        if (delayPerExecution > 0) {
            for (uint32_t d = 0; d < delayPerExecution; ++d) {
                // busy-wait
            }
        }

        if (quantum > 0 && cycles >= quantum) break;
    }
}

// Executes a single instruction and logs the action
void Process::executeInstruction(const Instruction& ins) {
    std::ostringstream entry;
    entry << "[" << getTimestamp() << "] ";

    switch (ins.type) {
    case InstructionType::PRINT:
        entry << "PRINT: Hello world from " << name << "!";
        break;
    case InstructionType::DECLARE:
        if (ins.args.size() >= 2) {
            memory[ins.args[0]] = static_cast<uint16_t>(std::stoi(ins.args[1]));
            entry << "DECLARE: " << ins.args[0] << " = " << ins.args[1];
        }
        break;
    case InstructionType::ADD:
        if (ins.args.size() >= 3) {
            int sum = getValue(ins.args[1]) + getValue(ins.args[2]);
            sum = std::clamp(sum, 0, static_cast<int>(UINT16_MAX));
            memory[ins.args[0]] = static_cast<uint16_t>(sum);
            entry << "ADD: " << ins.args[0] << " = " << ins.args[1] << " + " << ins.args[2]
                << " -> " << sum;
        }
        break;
    case InstructionType::SUBTRACT:
        if (ins.args.size() >= 3) {
            int diff = getValue(ins.args[1]) - getValue(ins.args[2]);
            diff = std::clamp(diff, 0, static_cast<int>(UINT16_MAX));
            memory[ins.args[0]] = static_cast<uint16_t>(diff);
            entry << "SUBTRACT: " << ins.args[0] << " = " << ins.args[1] << " - " << ins.args[2]
                << " -> " << diff;
        }
        break;
    case InstructionType::SLEEP:
        if (!ins.args.empty()) {
            entry << "SLEEP: " << ins.args[0] << " ticks";
            std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(ins.args[0]) * 10));
        }
        break;
    case InstructionType::FOR:
        entry << "FOR: repeat " << ins.repeatCount << " times {";
        logs.push_back(entry.str());
        for (int i = 0; i < ins.repeatCount; ++i) {
            logs.push_back("  [FOR iteration " + std::to_string(i + 1) + "]");
            for (const auto& subIns : ins.body) {
                executeInstruction(subIns);
            }
        }
        logs.push_back("  [FOR loop ended] };");
        return;


    default:
        break;
    }

    logs.push_back(entry.str());
}

uint16_t Process::getValue(const std::string& arg) {
    if (memory.count(arg)) return memory[arg];
    return static_cast<uint16_t>(std::stoi(arg));
}

std::string Process::getName() const { return name; }
int Process::getAssignedCore() const { return assignedCore; }
int Process::getCurrentLine() const { return currentLine; }
int Process::getTotalLines() const { return static_cast<int>(instructions.size()); }
int Process::getId() const { return id; }
