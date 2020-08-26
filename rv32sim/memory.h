#ifndef MEMORY_H
#define MEMORY_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2019

   Class for memory

**************************************************************** */

#include <cstdarg>
#include <string>
#include <unordered_map>
#include <vector>

#include "util.h"

using namespace std;

class memory {
private:
   bool Be_Verbose = false;

   /// Round the given address down to a word-aligned address. Tests indicate
   /// this function will be optimised out, so use it for readability.
   static constexpr uint32_t Round_Down_To_Word_Aligned( uint32_t Address );

   /// An address->word map representing the main memory available to the CPU.
   unordered_map<uint32_t, uint32_t> Memory_Map;

public:
   // Constructor
   memory( bool Verbose );

   /// Read a word of data from a word-aligned address. If the address is not a
   /// multiple of 4, it is rounded down to a multiple of 4.
   uint32_t read_word( uint32_t Address );

   /// Read a word of data from a non-word-aligned address. This is slow. Added.
   uint32_t read_word_unaligned( uint32_t Address );

   /// Read a byte from the given address. This is slow. Added.
   uint8_t read_byte( uint32_t Address );

   // void test_read_byte( void );

   /// Write a word of data to a word-aligned address. If the address is not a
   /// multiple of 4, it is rounded down to a multiple of 4. The mask contains
   /// 1s for bytes to be updated and 0s for bytes that are to be unchanged.
   void write_word( uint32_t Address, uint32_t Data, uint32_t Mask = ~0 );

   /// Load a hex image file and provide the start address for execution from
   /// the file in start_address. Return true if the file was read without
   /// error, or false otherwise.
   bool load_file( string File_Name, uint32_t &Start_Address );
};

#endif
