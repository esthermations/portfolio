#ifndef PROCESSOR_H
#define PROCESSOR_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2019

   Class for processor

**************************************************************** */

#include <unordered_map>

#include "csr.h"
#include "memory.h"

using namespace std;

// -----------------------------------------------------------------------------
// Phase 2:

enum privelige_level {
   PRIV_USER    = 0,
   PRIV_MACHINE = 3,
};

enum execution_result : uint32_t {
   // All good
   SUCCESS = (uint32_t) -1,

   // Exceptions and interrupts
   EXC_INSTRUCTION_ADDRESS_MISALIGNED = 0x00000000, // PC % 4 != 0
   EXC_ILLEGAL_INSTRUCTION            = 0x00000002,
   EXC_BREAKPOINT                     = 0x00000003, // ebreak
   EXC_LOAD_ADDRESS_MISALIGNED        = 0x00000004, // addr%load_size != 0
   EXC_STORE_ADDRESS_MISALIGNED       = 0x00000006, // ditto for sw, sh
   EXC_ECALL_FROM_USER_MODE           = 0x00000008,
   EXC_ECALL_FROM_MACHINE_MODE        = 0x0000000b,
   INT_USER_SOFTWARE_INTERRUPT        = 0x80000000, // (mip.usip and mie.usie)
   INT_MACH_SOFTWARE_INTERRUPT        = 0x80000003, // (mip.msip and mie.msie)
   INT_USER_TIMER_INTERRUPT           = 0x80000004, // (mip.utip and mie.utie)
   INT_MACH_TIMER_INTERRUPT           = 0x80000007, // (mip.mtip and mie.mtie)
   INT_USER_EXTERNAL_INTERRUPT        = 0x80000008, // (mip.ueip and mie.ueie)
   INT_MACH_EXTERNAL_INTERRUPT        = 0x8000000b, // (mip.meip and mie.meie)
   // All interrupt conditions require mstatus.mie

};

constexpr auto Successful_Execution = execution_result::SUCCESS;

inline bool Is_Exception( execution_result R ) {
   switch ( R ) {
      case EXC_INSTRUCTION_ADDRESS_MISALIGNED:
      case EXC_ILLEGAL_INSTRUCTION:
      case EXC_BREAKPOINT:
      case EXC_LOAD_ADDRESS_MISALIGNED:
      case EXC_STORE_ADDRESS_MISALIGNED:
      case EXC_ECALL_FROM_USER_MODE:
      case EXC_ECALL_FROM_MACHINE_MODE: return true;
      default: return false;
   }
}

inline bool Is_Interrupt( execution_result R ) {
   switch ( R ) {
      case INT_USER_SOFTWARE_INTERRUPT:
      case INT_MACH_SOFTWARE_INTERRUPT:
      case INT_USER_TIMER_INTERRUPT:
      case INT_MACH_TIMER_INTERRUPT:
      case INT_USER_EXTERNAL_INTERRUPT:
      case INT_MACH_EXTERNAL_INTERRUPT: return true;
      default: return false;
   }
}

inline const char *Exec_Result_To_String( execution_result R ) {
   switch ( R ) {
      case EXC_INSTRUCTION_ADDRESS_MISALIGNED:
         return "EXC_INSTRUCTION_ADDRESS_MISALIGNED";
      case EXC_ILLEGAL_INSTRUCTION: return "EXC_ILLEGAL_INSTRUCTION";
      case EXC_BREAKPOINT: return "EXC_BREAKPOINT";
      case EXC_LOAD_ADDRESS_MISALIGNED: return "EXC_LOAD_ADDRESS_MISALIGNED";
      case EXC_STORE_ADDRESS_MISALIGNED: return "EXC_STORE_ADDRESS_MISALIGNED";
      case EXC_ECALL_FROM_USER_MODE: return "EXC_ECALL_FROM_USER_MODE";
      case EXC_ECALL_FROM_MACHINE_MODE: return "EXC_ECALL_FROM_MACHINE_MODE";
      case INT_USER_SOFTWARE_INTERRUPT: return "INT_USER_SOFTWARE_INTERRUPT";
      case INT_MACH_SOFTWARE_INTERRUPT: return "INT_MACH_SOFTWARE_INTERRUPT";
      case INT_USER_TIMER_INTERRUPT: return "INT_USER_TIMER_INTERRUPT";
      case INT_MACH_TIMER_INTERRUPT: return "INT_MACH_TIMER_INTERRUPT";
      case INT_USER_EXTERNAL_INTERRUPT: return "INT_USER_EXTERNAL_INTERRUPT";
      case INT_MACH_EXTERNAL_INTERRUPT: return "INT_MACH_EXTERNAL_INTERRUPT";
      default: return "??? Exec status ???";
   }
}

using from_instr = bool;

// -----------------------------------------------------------------------------

class processor {
private:
   bool Be_Verbose = false;

   /// How many instructions the CPU has executed.
   uint32_t Executed_Instruction_Count = 0;

   /// The program counter.
   uint32_t PC = 0;

   /// The values of the registers. Register_X[1] is equivalent to x1. x0 is
   /// present in this array, but must always return zero. These values should
   /// only be accessed or modified via the get_reg() and set_reg() procedures.
   uint32_t Register_X[32] = {0};

   bool Breakpoint_Is_Active   = false;
   uint32_t Breakpoint_Address = 0;

   //
   // Stage 2:
   //

   // Contents of the Control and Status Registers (CSRs). Most of these have
   // special properties that must be guaranteed, so they should only be set and
   // retrieved by the get_ and set_csr() functions.
   uint32_t CSR[NUM_CSR_CODES] = {0};

   /// Current operating privelige level. This interacts heavily with the
   /// MSTATUS CSR.
   privelige_level Privelige_Level = PRIV_MACHINE;

   /// Handle the given execution result.
   void Handle_Exception( execution_result Result, uint32_t Instruction = 0 );

   execution_result Check_For_Pending_Interrupts( void );

public:
   // Memory is public so the Execute() functions in my instruction types in
   // rv32i.h can access it. Isn't object-oriented programming fun!
   memory *Main_Memory;

   // Are we in stage 2?
   bool Stage2 = false;

   // Consructor
   processor( memory *Main_Memory, bool Verbose, bool Stage2 );

   // Display PC value
   void show_pc( void ) const;

   // Set PC to new value
   void set_pc( uint32_t New_Value );

   // Get the current PC
   uint32_t get_pc( void ) const;

   // Display register value
   void show_reg( unsigned int Reg_Num ) const;

   // Set register to new value
   void set_reg( unsigned int Reg_Num, uint32_t New_Value );

   // Get register value -- Added
   uint32_t get_reg( unsigned int Reg_Num ) const;

   // Execute a number of instructions
   void execute( unsigned int Num, bool Check_For_Breakpoints );

   // Clear breakpoint
   void clear_breakpoint( void );

   // Set breakpoint at an address
   void set_breakpoint( uint32_t Address );

   // Show privilege level
   void show_prv() const;

   // Set privilege level
   void set_prv( unsigned int Privelige_Level );

   // Display CSR value
   void show_csr( unsigned int CSR_Number ) const;

   // Get the current privelige level -- Added
   unsigned get_prv( void ) const;

   // Set CSR to new value
   // void set_csr( unsigned int CSR_Number, uint32_t New_Value );

   // Set CSR to new value from an instruction. This will ignore read-only bits.
   void set_csr( unsigned int CSR_Number,
                 uint32_t New_Value,
                 from_instr From_Instr = from_instr( false ) );

   // Get the value of the given CSR -- Added
   uint32_t get_csr( unsigned CSR_Number ) const;

   // Get the number of instructions executed. Excludes instructions that
   // were interrupted or raised an exception.
   uint32_t get_instruction_count() const {
      return this->Executed_Instruction_Count;
   };

   // Used for Postgraduate assignment. Undergraduate assignment can return
   // 0.
   uint32_t get_cycle_count() const {
      return 0;
   }
};

#endif // PROCESSOR_H
