#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <mutex>

class Process {
public:
    Process(const std::string& name, int totalLines);

    void run(int coreId);
	void run(int coreId, int quantumCycles); // For Round Robin scheduling

    const std::string& getName() const;
    int getCurrentLine() const;
    int getTotalLines() const;
    int getAssignedCore() const;
    std::string getTimestamp() const;

private:
    std::string name;
    int totalLines;
    int currentLine;
    int assignedCore;
    std::string timestamp;
    std::mutex mtx;

    std::string getCurrentTimestamp() const;
};

#endif