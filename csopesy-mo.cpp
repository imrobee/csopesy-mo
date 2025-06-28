#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include "Scheduler.h"

Scheduler scheduler;

void printHeader() {
    std::cout << "  _____   _____   ____   _____   ______   _____ __     __\n";
    std::cout << " / ____| / ____| / __ \\ |  __ \\ |  ____| / ____|\\ \\   / /\n";
    std::cout << "| |     | (___  | |  | || |__) || |__   | (___   \\ \\_/ / \n";
    std::cout << "| |      \\___ \\ | |  | ||  ___/ |  __|   \\___ \\   \\   /  \n";
    std::cout << "| |____  ____) || |__| || |     | |____  ____) |   | |   \n";
    std::cout << " \\_____||_____/  \\____/ |_|     |______||_____/    |_|   \n";
    std::cout << "                                                        \n";
    std::cout << "________________________________________________________\n";
    std::cout << "\n";
    std::cout << "Welcome to CSOPESY Emulator!\n\n";
    std::cout << "Developers:\n";
    std::cout << "Clemente, Daniel Gavrie \n";
    std::cout << "Feliciano, Jan Robee \n";
    std::cout << "Mangawang, Felix Melford\n";
    std::cout << "Roque, Dominic Angelo\n\n";
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen\n";
}

void clearScreen() {
    system("cls");
    printHeader();
}


void initializeEmulator() {
    // Initialize the processor configuration of the application with config.txt
    std::cout << "Initialize command recognized. Configuring OS Emulator...\n";
    scheduler.initialize("config.txt");
}


void startScheduler() {
    // Generates processes and inserts them into ready queue
    std::cout << "Scheduler-start command recognized. Starting scheduler...\n";
    scheduler.start();
}

void stopScheduler() {
    // Stops the scheduler from creating new processes
    std::cout << "Scheduler-stop command recognized. Stopping process generation...\n";
    scheduler.stop();
}

void attachToProcess(const std::string& command) {
    std::string processName = command.substr(10); // skip "screen -s "

    std::shared_ptr<Process> process = nullptr;
    auto itRunning = scheduler.runningProcesses.find(processName);
    if (itRunning != scheduler.runningProcesses.end()) {
        process = itRunning->second;
    }
    else {
        auto itFinished = scheduler.finishedProcesses.find(processName);
        if (itFinished != scheduler.finishedProcesses.end()) {
            process = itFinished->second;
        }
    }

    if (process) {
        system("cls");
        std::cout << "Attached to process: " << processName << "\n";

        // Immediately print "process-smi" info once before accepting input
        auto logs = process->getLogs();
        std::cout << "Process name: " << processName << "\n";
        std::cout << "ID: " << process->getId() << "\n";
        std::cout << "Logs:\n";
        for (const auto& log : logs) {
            std::cout << log << "\n";
        }
        if (process->getCurrentLine() == process->getTotalLines()) {
            std::cout << "Finished!\n";
        }
        else {
            std::cout << "Instruction: " << process->getCurrentLine()
                << " / " << process->getTotalLines() << "\n";
        }

        // Input loop
        while (true) {
            std::cout << "(" << processName << ")> ";
            std::string subCommand;
            std::getline(std::cin, subCommand);

            if (subCommand == "process-smi") {
                auto logs = process->getLogs();
                std::cout << "Process name: " << processName << "\n";
                std::cout << "ID: " << process->getId() << "\n";
                std::cout << "Logs:\n";
                for (const auto& log : logs) {
                    std::cout << log << "\n";
                }
                if (process->getCurrentLine() == process->getTotalLines()) {
                    std::cout << "\nFinished!\n";
                }
                else {
                    std::cout << "Instruction: " << process->getCurrentLine() << " / " << process->getTotalLines() << "\n";
                }
            }
            else if (subCommand == "exit") {
                std::cout << "Returning to main menu...\n";
                break;
            }
            else {
                std::cout << "Unknown command inside process screen.\n";
            }
        }
    }
    else {
        std::cout << "Process " << processName << " not found.\n";
    }
}




void reattachToProcess(const std::string& command) {
    std::string processName = command.substr(10); // skip "screen -r "
    auto it = scheduler.runningProcesses.find(processName);
    if (it != scheduler.runningProcesses.end()) {
        auto process = it->second;
        system("cls");
        std::cout << "Re-attached to process: " << processName << "\n";

        // Print the same details as "process-smi"
        auto logs = process->getLogs();
        std::cout << "Process name: " << processName << "\n";
        std::cout << "ID: " << process->getId() << "\n";
        std::cout << "Logs:\n";
        for (const auto& log : logs) {
            std::cout << log << "\n";
        }
        if (process->getCurrentLine() == process->getTotalLines()) {
            std::cout <<"\nFinished!\n";
        }
        else {
            std::cout << "Instruction: " << process->getCurrentLine() << " / " << process->getTotalLines() << "\n";
        }

        // Immediately return to main menu
    }
    else {
        std::cout << "Process " << processName << " not found or has finished.\n";
    }
}





//A main menu console for recognizing the following commands : 
//“initialize” – initialize the processor configuration of the application. This must be called before any other command could be recognized, aside from “exit”.
//“exit” – terminates the console.
//“screen” – note: does not support "screen" by itself, only the following commands:
//          “screen -s <process name>" - clears the screen and creates a new process with the given name.
//                      “process-smi” – Prints a simple information about the process (lines 9 – 13). The process contains dummy instructions that the CPU executes in the background. Whenever the user types “process-smi”, it provides the updated details and accompanying logs from the print instructions. (e.g., lines 162 – 170). If the process has finished, simply print “Finished!” after the process name, ID, and logs have been printed (e.g., lines 17 – 20).
//                      “exit” – Returns the user to the main menu.
//  	    “screen -r <process name>” – clears the screen then accesses the process with the given name. Does not support process-smi
// 			“screen -ls” – lists all processes that are currently running. 
//“scheduler - start”(formerly scheduler - test) – continuously generates a batch of dummy processes for the CPU scheduler. Each process is accessible via the “screen” command.
//“scheduler - stop” – stops generating dummy processes.
//“report - util” – for generating CPU utilization report.See additional details.

//BIG NOTE: User should only see the header in the main menu (i.e., not in screen -s).
void enterMainLoop() {
    std::string command;
    printHeader();
    bool initialized = false;

    while (true) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        // if not initialized, only allow "initialize" and "exit"
        if (!initialized) {
            if (command == "initialize") {
                initializeEmulator();
                initialized = true;
            }
            else if (command == "exit") {
                std::cout << "Exit command recognized. Closing application.\n";
                break;
            }
            else {
                std::cout << "Please initialize the emulator first by typing 'initialize'.\n";
            }
        }
        else {
            if (command == "view-config") {
                scheduler.viewConfig();
            }
            else if (command == "initialize") {
                initializeEmulator();
            }
            else if (command == "exit") {
                std::cout << "Exit command recognized. Closing application.\n";
                scheduler.stop(); // Ensure the scheduler is stopped before exiting
                break;
            }
            else if (command.rfind("screen -s ", 0) == 0) {
                attachToProcess(command);
				clearScreen();
            }
            else if (command.rfind("screen -r ", 0) == 0) { 
                reattachToProcess(command);
            }
            else if (command == "screen -ls") { // "List all processes"
                scheduler.printStatus();
            }
            else if (command == "scheduler-start") {
                startScheduler();
            }
            else if (command == "scheduler-stop") {
                stopScheduler();
            }
            else if (command == "report-util") { // Saves "screen -ls" in csopesy-log.txt
                scheduler.writeStatusToFile();
            }
            else if (command == "clear") { // Clears the screen
                clearScreen();
            }
            else if (command.empty()) {
                continue;
            }
            else {
                std::cout << "Unrecognized command.\n";
            }
        }
    }
}

int main() {
    enterMainLoop();
    return 0;
}