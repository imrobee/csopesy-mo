#pragma once
#include <string>
#include <vector>

enum class InstructionType {
    PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR
};

struct Instruction {
    InstructionType type;
    std::vector<std::string> args;             // e.g., var names or values
    std::vector<Instruction> body;             // for FOR loops
    int repeatCount = 1;                        // for FOR loops

    Instruction(InstructionType type,
        const std::vector<std::string>& args = {},
        const std::vector<Instruction>& body = {},
        int repeatCount = 1)
        : type(type), args(args), body(body), repeatCount(repeatCount) {
    }
};
