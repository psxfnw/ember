#pragma once
#include "../cpu.hpp"
#include <string>
int file_command(const std::string& action, const std::string& path, CPU& cpu, Memory& mem,
                 bool interactive = false);
