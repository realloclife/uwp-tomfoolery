#include "execution.hpp"

std::mutex exec::pending_bytecode_mutex{};
std::queue<std::string> exec::pending_bytecode{};
uintptr_t exec::whsj_step_original{};
