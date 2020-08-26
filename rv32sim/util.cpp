/*
  Unit tests for utility functions. The bodies were in here but since they're
  all constexpr they're implicitly inline, which means they have to go in a
  header to be usable. So only the unit tests live in here.
 */

#include <cstdlib>
#include <string>

#include "csr.h"
#include "util.h"

using namespace util;

// clang-format off

// ----------------------------------------------------------------------------

// Get_Bit()

// static_assert(     Get_Bit( 0b00000001, 1  ) );
// static_assert(     Get_Bit( 0b00000010, 2  ) );
// static_assert(     Get_Bit( 0b00000100, 3  ) );
// static_assert(     Get_Bit( 0b00001000, 4  ) );
// static_assert( not Get_Bit( 0b11111110, 1  ) );
// static_assert(     Get_Bit( 0x80000000, 32 ) );
// static_assert(     Get_Bit( 0x70000000, 30 ) );

// ----------------------------------------------------------------------------

// Sign_Extend()

// static_assert( Sign_Extend( 0b1000'0000'0000, 12 ) == 0xFFFF'F800 );
// static_assert( Sign_Extend( 0b1000'1000'1000, 12 ) == 0xFFFF'F888 );
// static_assert( Sign_Extend( 0x00000080,  8 ) == 0xFFFFFF80 );
// static_assert( Sign_Extend( 0x00008000, 16 ) == 0xFFFF8000 );
// static_assert( Sign_Extend( 0x80000000, 32 ) == 0x80000000 );
// static_assert( Sign_Extend( 0xFFFFFFFF, 32 ) == 0xFFFFFFFF );

// ----------------------------------------------------------------------------

// Clean_Bits()

// static_assert( Zero_Extend( 0b10101010, 3 ) == 0b00000010 );
// static_assert( Zero_Extend( 0xFFFFFFFF, 1 ) == 0x00000001 );
// static_assert( Zero_Extend( 0x00000080,  8 ) == 0x00000080 );
// static_assert( Zero_Extend( 0x00008000, 16 ) == 0x00008000 );
// static_assert( Zero_Extend( 0x80000000, 32 ) == 0x80000000 );
// static_assert( Zero_Extend( 0xFFFFFFFF, 32 ) == 0xFFFFFFFF );

// ----------------------------------------------------------------------------

// Set_Bit()

// static_assert( Set_Bit( 0b00001000, 1, 1 ) == 0b00001001 );
// static_assert( Set_Bit( 0b00001001, 1, 1 ) == 0b00001001 );
// static_assert( Set_Bit( 0b00000000, 8, 1 ) == 0b10000000 );
// static_assert( Set_Bit( 0b11110111, 1, 0 ) == 0b11110110 );
// static_assert( Set_Bit( 0b11110110, 1, 0 ) == 0b11110110 );
// static_assert( Set_Bit( 0b11111111, 8, 0 ) == 0b01111111 );

// ----------------------------------------------------------------------------

// Mask_Is_Contiguous()

// static_assert(     Mask_Is_Contiguous( 0          ) );
// static_assert(     Mask_Is_Contiguous( 0b1100     ) );
// static_assert( not Mask_Is_Contiguous( 0b0101     ) );
// static_assert( not Mask_Is_Contiguous( 0xF0F0F0F0 ) );
// static_assert(     Mask_Is_Contiguous( 0xFFFFFFFF ) );
// static_assert(     Mask_Is_Contiguous( 0x70000000 ) );

// ----------------------------------------------------------------------------

// Switch_Endianness()

// static_assert( Switch_Endianness( 0xAA00AA00 ) == 0x00AA00AA );
// static_assert( Switch_Endianness( 0x12345678 ) == 0x78563412 );
// static_assert( Switch_Endianness( 0x00000001 ) == 0x01000000 );
// static_assert( Switch_Endianness( 0x10000000 ) == 0x00000010 );

// ----------------------------------------------------------------------------

std::string util::Integer_To_Binary_String( uint32_t Integer ) {

   uint8_t Num_Digits_To_Print = 0;
   if ( Integer <= UINT8_MAX ) {
      Num_Digits_To_Print = 8;
   } else if ( Integer <= UINT16_MAX ) {
      Num_Digits_To_Print = 16;
   } else if ( Integer <= UINT32_MAX ) {
      Num_Digits_To_Print = 32;
   } else {
      Num_Digits_To_Print = 64;
   }

   uint8_t As_Binary[64] = {0};

   // This loop assumes an unsigned right shift.
   for ( int I = 0; I < 64; ++I ) {
      As_Binary[64 - I - 1] = ( uint8_t )( Integer & 1 );
      Integer >>= 1;
   }


   char *Str = new char [64 + 2 + 1];
   Str[0] = '0';
   Str[1] = 'b';
   size_t Str_Idx = 2;

   const uint8_t Starting_Digit = ( 64 - Num_Digits_To_Print );
   for ( uint8_t I = Starting_Digit; I < 64; ++I ) {
      char C = '!';
      if ( As_Binary[I] == 0 ) {
         C = '0';
      } else if ( As_Binary[I] == 1 ) {
         C = '1';
      } else {
         C = 'X';
      }

      Str[Str_Idx++] = C;
   }

   return std::string( Str ); 
}

// ----------------------------------------------------------------------------

std::string util::Mstatus_To_String( mstatus M ) {
   enum { LENGTH = 300 };
   char Base_Str[LENGTH] = {0};
   char *Str = Base_Str;

   // clang-format off
   Str += snprintf( Str, LENGTH, "mstatus{ " );
   if ( M.SD )     Str += snprintf( Str, LENGTH, "SD: %u, ", M.SD );
   if ( M.WPRI_4 ) Str += snprintf( Str, LENGTH, "WPRI_4: %u, ", M.WPRI_4 );
   if ( M.TW )     Str += snprintf( Str, LENGTH, "TW: %u, ", M.TW );
   if ( M.TVM )    Str += snprintf( Str, LENGTH, "TVM: %u, ", M.TVM );
   if ( M.MXR )    Str += snprintf( Str, LENGTH, "MXR: %u, ", M.MXR );
   if ( M.SUM )    Str += snprintf( Str, LENGTH, "SUM: %u, ", M.SUM );
   if ( M.MPRV )   Str += snprintf( Str, LENGTH, "MPRV: %u, ", M.MPRV );
   if ( M.XS )     Str += snprintf( Str, LENGTH, "XS: %u, ", M.XS );
   if ( M.FS )     Str += snprintf( Str, LENGTH, "FS: %u, ", M.FS );
   if ( M.MPP )    Str += snprintf( Str, LENGTH, "MPP: %u, ", M.MPP );
   if ( M.WPRI_3 ) Str += snprintf( Str, LENGTH, "WPRI_3: %u, ", M.WPRI_3 );
   if ( M.SPP )    Str += snprintf( Str, LENGTH, "SPP: %u, ", M.SPP );
   if ( M.MPIE )   Str += snprintf( Str, LENGTH, "MPIE: %u, ", M.MPIE );
   if ( M.WPRI_2 ) Str += snprintf( Str, LENGTH, "WPRI_2: %u, ", M.WPRI_2 );
   if ( M.SPIE )   Str += snprintf( Str, LENGTH, "SPIE: %u, ", M.SPIE );
   if ( M.UPIE )   Str += snprintf( Str, LENGTH, "UPIE: %u, ", M.UPIE );
   if ( M.MIE )    Str += snprintf( Str, LENGTH, "MIE: %u, ", M.MIE );
   if ( M.WPRI_1 ) Str += snprintf( Str, LENGTH, "WPRI_1: %u, ", M.WPRI_1 );
   if ( M.SIE )    Str += snprintf( Str, LENGTH, "SIE: %u, ", M.SIE );
   if ( M.UIE )    Str += snprintf( Str, LENGTH, "UIE: %u, ", M.UIE );
   Str += snprintf( Str, LENGTH, " }" );
   // clang-format on

   return std::string( Base_Str );
}
