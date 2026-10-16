#pragma once
#include "../cpu.hpp"
#include "token.hpp"
#include <string>
#include <vector>
struct Instruction {
    Op op;
    const char* name;
    const char* pattern;
};
const std::vector<Instruction>& instruction_set();
bool assemble(const std::string& source, std::vector<Byte>& bytes, std::string& error);
bool disassemble(const std::vector<Byte>& bytes, std::string& source, std::string& error);
