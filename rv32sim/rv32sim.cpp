/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2019

   Main program

**************************************************************** */

#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>

#include "memory.h"
#include "processor.h"
#include "commands.h"

using namespace std;

int main(int argc, char* argv[]) {

    // Values of command line options. 
    string arg;
    bool verbose = false;
    bool cycle_reporting = false;
    bool stage2 = false;

    memory* main_memory;
    processor* cpu;

    unsigned long int cpu_instruction_count;
    
    for (int i = 1; i < argc; i++) {
	// Process the next option
	arg = string(argv[i]);
	if (arg == "-v")  // Verbose output
	    verbose = true;
	else if (arg == "-c")  // Cycle and instruction reporting enabled
	    cycle_reporting = true;
	else if (arg == "-s2")  // Stage 2 functionality enabled
	    stage2 = true;
	else {
	    cout << "Unknown option: " << arg << endl;
	}
    }

    main_memory = new memory (verbose);
    cpu = new processor (main_memory, verbose, stage2);

    interpret_commands(main_memory, cpu, verbose);

    // Report final statistics

    cpu_instruction_count = cpu->get_instruction_count();
    cout << "Instructions executed: " << dec << cpu_instruction_count << endl;

    if (cycle_reporting) {
	// Required for postgraduate Computer Architecture course
	unsigned long int cpu_cycle_count;

	cpu_cycle_count = cpu->get_cycle_count();

	cout << "CPU cycle count: " << dec << cpu_cycle_count << endl;
    }
}
