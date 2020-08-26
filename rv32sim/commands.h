#ifndef COMMANDS_H
#define COMMANDS_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2019

   Command interpreter

**************************************************************** */

#include "memory.h"
#include "processor.h"

void interpret_commands(memory* main_memory, processor* cpu, bool verbose);

#endif
