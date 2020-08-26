/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2019

   Class members for memory

**************************************************************** */

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "memory.h"
#include "util.h"
using namespace std;

/// Constructor
memory::memory( bool Verbose ) {
   this->Be_Verbose = Verbose;
}

// ----------------------------------------------------------------------------

/// Read a word of data from a word-aligned address. If the address is not a
/// multiple of 4, it is rounded down to a multiple of 4.
uint32_t memory::read_word( uint32_t Address ) {
   Address = util::Round_Down_To_Word_Aligned( Address );

   return this->Memory_Map[Address];
   // NOTE: The [] operator inserts the value if it doesn't exist. In this
   // particular case, we do actually want that. It's why this function isn't
   // marked const.
}

// ----------------------------------------------------------------------------

uint8_t memory::read_byte( uint32_t Address ) {
   const auto Word_Address = util::Round_Down_To_Word_Aligned( Address );
   const auto Shift        = ( 8 * ( Address - Word_Address ) );
   const auto Mask         = ( 0xFF000000 >> Shift );

   const uint32_t Word   = this->read_word( Word_Address );
   const uint32_t Masked = ( Word & Mask );

   const uint8_t Byte = ( Masked >> ( 24 - Shift ) );
   return Byte;
}

// ----------------------------------------------------------------------------

// void memory::test_read_byte( void ) {
//    DEBUG_LOG( YELLOW( "Running tests." ) );
//
//    this->write_word( 0, 0x11223344, ~0 );
//    this->write_word( 4, 0x55667788, ~0 );
//    assert( this->read_byte( 0 ) == 0x11 );
//    assert( this->read_byte( 1 ) == 0x22 );
//    assert( this->read_byte( 2 ) == 0x33 );
//    assert( this->read_byte( 3 ) == 0x44 );
//    assert( this->read_byte( 4 ) == 0x55 );
//    assert( this->read_byte( 5 ) == 0x66 );
//    assert( this->read_byte( 6 ) == 0x77 );
//    assert( this->read_byte( 7 ) == 0x88 );
//
//    assert( this->read_word_unaligned( 0 ) == 0x11223344 );
//    assert( this->read_word_unaligned( 1 ) == 0x22334455 );
//    assert( this->read_word_unaligned( 2 ) == 0x33445566 );
//    assert( this->read_word_unaligned( 3 ) == 0x44556677 );
//    assert( this->read_word_unaligned( 4 ) == 0x55667788 );
//
//    // TODO: Delete this test once I'm confident I won't need it anymore.
//    this->write_word( 0, 0, ~0 );
//    this->write_word( 4, 0, ~0 );
//
//    DEBUG_LOG( GREEN( "Tests passed." ) );
// }

// ----------------------------------------------------------------------------

uint32_t memory::read_word_unaligned( uint32_t Address ) {
   if ( util::Address_Is_Word_Aligned( Address ) ) {
      return this->read_word( Address );
   }

   DEBUG_LOG( "Reading a genuinely unaligned word. This should be rare." );

   union {
      uint8_t Bytes[4];
      uint32_t Word;
   } Value;

   for ( auto I = 1; I <= 4; ++I ) {
      // Write backwards. Little-endian.
      Value.Bytes[4 - I] = this->read_byte( Address + I - 1 );
   }

   DEBUG_LOG( " Returning %08x.", Value.Word );
   return Value.Word;
}

// ----------------------------------------------------------------------------

/// Write a word of data to a word-aligned address. If the address is not a
/// multiple of 4, it is rounded down to a multipl of 4. The mask contains 1s
/// for bytes to be updated and 0s for bytes that are to be unchanged.
void memory::write_word( uint32_t Address, uint32_t Data, uint32_t Mask ) {
   Address = util::Round_Down_To_Word_Aligned( Address );

   uint32_t const Old_Value = this->Memory_Map[Address];

   // Zero out the bits to be changed
   uint32_t const Masked = ( Old_Value & ~Mask );

   // Set those zeroed bits to whatever Data has set
   uint32_t const New_Value = ( Data | Masked );

   Memory_Map[Address] = New_Value;

   DEBUG_LOG(
     "memory %08x <- word %08x (was %08x)", Address, New_Value, Old_Value );
}

// ----------------------------------------------------------------------------

/*
  This is provided. I'll leave it unmodified, but it may have been hit by
  clang-format.
 */

// Load a hex image file and provide the start address for execution from the
// file in start_address. Return true if the file was read without error, or
// false otherwise.
bool memory::load_file( string file_name, uint32_t &start_address ) {
   ifstream input_file( file_name );
   string input;
   unsigned int line_count = 0;
   unsigned int byte_count = 0;
   char record_start;
   char byte_string[3];
   char halfword_string[5];
   unsigned int record_length;
   unsigned int record_address;
   unsigned int record_type;
   unsigned int record_data;
   unsigned int record_checksum;
   bool end_of_file_record = false;
   uint32_t load_address;
   uint32_t load_data;
   uint32_t load_mask;
   uint32_t load_base_address = 0x00000000UL;
   start_address              = 0x00000000UL;
   if ( input_file.is_open() ) {
      while ( true ) {
         line_count++;
         input_file >> record_start;
         if ( record_start != ':' ) {
            cout << "Input line " << dec << line_count
                 << " does not start with colon character" << endl;
            return false;
         }
         input_file.get( byte_string, 3 );
         sscanf( byte_string, "%x", &record_length );
         input_file.get( halfword_string, 5 );
         sscanf( halfword_string, "%x", &record_address );
         input_file.get( byte_string, 3 );
         sscanf( byte_string, "%x", &record_type );
         switch ( record_type ) {
            case 0x00: // Data record
               for ( unsigned int i = 0; i < record_length; i++ ) {
                  input_file.get( byte_string, 3 );
                  sscanf( byte_string, "%x", &record_data );
                  load_address =
                    ( load_base_address | ( uint32_t )( record_address ) ) + i;
                  load_data = ( uint32_t )( record_data )
                              << ( ( load_address % 4 ) * 8 );
                  load_mask = 0x000000ffUL << ( ( load_address % 4 ) * 8 );
                  write_word(
                    load_address & 0xfffffffcUL, load_data, load_mask );
                  byte_count++;
               }
               break;
            case 0x01: // End of file
               end_of_file_record = true;
               break;
            case 0x02: // Extended segment address (set bits 19:4 of load base
                       // address)
               load_base_address = 0x00000000UL;
               for ( unsigned int i = 0; i < record_length; i++ ) {
                  input_file.get( byte_string, 3 );
                  sscanf( byte_string, "%x", &record_data );
                  load_base_address =
                    ( load_base_address << 8 ) | ( record_data << 4 );
               }
               break;
            case 0x03: // Start segment address (ignored)
               for ( unsigned int i = 0; i < record_length; i++ ) {
                  input_file.get( byte_string, 3 );
                  sscanf( byte_string, "%x", &record_data );
               }
               break;
            case 0x04: // Extended linear address (set upper halfword of load
                       // base address)
               load_base_address = 0x00000000UL;
               for ( unsigned int i = 0; i < record_length; i++ ) {
                  input_file.get( byte_string, 3 );
                  sscanf( byte_string, "%x", &record_data );
                  load_base_address =
                    ( load_base_address << 8 ) | ( record_data << 16 );
               }
               break;
            case 0x05: // Start linear address (set execution start address)
               start_address = 0x00000000UL;
               for ( unsigned int i = 0; i < record_length; i++ ) {
                  input_file.get( byte_string, 3 );
                  sscanf( byte_string, "%x", &record_data );
                  start_address = ( start_address << 8 ) | record_data;
               }
               break;
         }
         input_file.get( byte_string, 3 );
         sscanf( byte_string, "%x", &record_checksum );
         input_file.ignore();
         if ( end_of_file_record )
            break;
      }
      input_file.close();
      cout << dec << byte_count
           << " bytes loaded, start address = " << setw( 8 ) << setfill( '0' )
           << hex << start_address << endl;
      return true;
   } else {
      cout << "Failed to open file" << endl;
      return false;
   }
}
