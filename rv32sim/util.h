#pragma once

#include <cassert>
#include <cstdint>
#include <string>

#include "csr.h"

using namespace std;

#define COLOUR_ANSI_RED "\033[1;31m"
#define COLOUR_ANSI_GREEN "\033[1;32m"
#define COLOUR_ANSI_YELLOW "\033[1;33m"
#define COLOUR_ANSI_BLUE "\033[1;34m"
#define COLOUR_ANSI_MAGENTA "\033[1;35m"
#define COLOUR_ANSI_CYAN "\033[1;36m"
#define COLOUR_ANSI_DEFAULT "\033[39;49m"

#define YELLOW( str ) COLOUR_ANSI_YELLOW str COLOUR_ANSI_DEFAULT
#define GREEN( str ) COLOUR_ANSI_GREEN str COLOUR_ANSI_DEFAULT
#define RED( str ) COLOUR_ANSI_RED str COLOUR_ANSI_DEFAULT
#define CYAN( str ) COLOUR_ANSI_CYAN str COLOUR_ANSI_DEFAULT
#define BLUE( str ) COLOUR_ANSI_BLUE str COLOUR_ANSI_DEFAULT
#define MAGENTA( str ) COLOUR_ANSI_MAGENTA str COLOUR_ANSI_DEFAULT

/// Print a debug message if Be_Verbose is true.
///
/// See https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html for info on how
/// variadic macros work
#define DEBUG_LOG( fmtstr_without_newline, ... )                 \
   do {                                                          \
      if ( Be_Verbose ) {                                        \
         fprintf( stderr,                                        \
                  CYAN( "%s" ) ": " fmtstr_without_newline "\n", \
                  __FUNCTION__,                                  \
                  ##__VA_ARGS__ );                               \
      }                                                          \
   } while ( 0 )

/*
   Utility functions. These are all marked 'inline' because when I originally
   wrote this I was assuming C++14 was available, which allows multi-statement
   constexpr functions. Downgrading to C++11 meant downgrading them from
   'constexpr' to 'inline' so they'd retain the same linkage attributes.

   There are some unit tests in the form of static assertions in util.cpp.
*/
namespace util {

   /// Get a bit from a 32-bit number. Bit_Number should be in the range [1,32].
   inline bool Get_Bit( uint32_t Value, unsigned Bit_Number ) {
      assert( Bit_Number >= 1 and Bit_Number <= 32 );
      const uint32_t Mask = ( 1 << ( Bit_Number - 1 ) );
      const uint32_t Masked = ( Value & Mask );
      return ( Masked != 0 );
   }

   /// Set a bit in a 32-bit number. Bit_Number should be in the range [1,32].
   inline uint32_t Set_Bit( uint32_t Value,
                            unsigned Bit_Number,
                            bool Bit_Value ) {
      assert( Bit_Number >= 1 and Bit_Number <= 32 );
      assert( Bit_Value == 1 or Bit_Value == 0 );

      const auto Mask = ( 1 << ( Bit_Number - 1 ) );

      if ( Bit_Value == 0 ) {
         return ( Value & ~Mask );
      } else {
         return ( Value | Mask );
      }
   }

   /// Set all bits from Low_Bit to High_Bit to the given Bit_Value.
   inline uint32_t Set_Bits( uint32_t Value,
                             unsigned Low_Bit,
                             unsigned High_Bit,
                             bool Bit_Value ) {
      assert( Low_Bit >= 1 and Low_Bit <= 32 );
      assert( High_Bit >= 1 and High_Bit <= 32 );
      assert( High_Bit >= Low_Bit );

      auto Result = Value;

      for ( auto I = Low_Bit; I <= High_Bit; ++I ) {
         Result = Set_Bit( Result, I, Bit_Value );
      }

      return Result;
   }

   // static_assert( Set_Bits( 0xFFFF, 1, 8, false ) == 0xFF00 );

   /// Zero-extend the given value to 32 bits as if it were an integer Num_Bits
   /// in size. This is useful for "cleaning" the higher bits of a uint32_t.
   inline uint32_t Zero_Extend( uint32_t Value, unsigned Num_Bits ) {
      assert( Num_Bits >= 1 and Num_Bits <= 32 );
      if ( Num_Bits == 32 ) {
         return Value;
      }

      const uint32_t Mask = ( 0xFFFFFFFF << Num_Bits );
      return ( Value & ~Mask );
   }

   /// Sign-extend the given value to 32 bits as if it were a signed integer
   /// Num_Bits in size. Num_Bits should be in the range [1,32].
   inline uint32_t Sign_Extend( uint32_t Value, unsigned Num_Bits ) {
      assert( Num_Bits >= 1 and Num_Bits <= 32 );

      if ( Num_Bits == 32 ) {
         return Value;
      }

      const bool Sign_Bit = Get_Bit( Value, Num_Bits );
      const int32_t Mask = ( 0xFFFFFFFF << Num_Bits );

      return Sign_Bit ? ( Value | Mask ) : Value;
   }

   /// Returns true iff all the 1's in the binary representation of Mask are
   /// next to each other. 0b0110 is contiguous, 0b0101 is not.
   inline bool Mask_Is_Contiguous( uint32_t Mask ) {
      // Shift right until we hit a 1
      while ( ( Mask != 0 ) and ( Get_Bit( Mask, 1 ) == 0 ) ) {
         Mask >>= 1;
      }

      if ( Mask == 0 ) {
         return true;
      }

      // Shift right until we hit a 0
      while ( ( Mask != 0 ) and ( Get_Bit( Mask, 1 ) == 1 ) ) {
         Mask >>= 1;
      }

      // Now shift right until we hit a 1 again. If we don't, the mask is
      // contiguous. If we do, it's not.

      while ( ( Mask != 0 ) and ( Get_Bit( Mask, 1 ) == 0 ) ) {
         Mask >>= 1;
      }

      return ( Mask == 0 );
   }

   /// Returns true if the given address is word-aligned.
   inline bool Address_Is_Word_Aligned( uint32_t Address ) {
      return ( Address % 4 == 0 );
   }

   /// Round the given address down to the nearest word-aligned address below.
   inline uint32_t Round_Down_To_Word_Aligned( uint32_t Address ) {
      const uint32_t Remainder = ( Address % 4 );
      if ( Remainder != 0 ) {
         Address -= Remainder;
      }
      return Address;
   }

   /**
      Switch the endianness of the given word.

      Wanna know something bananas? GCC *figures out* that that's what this
      function does, and optimises it into a single bswap instruction.

      https://godbolt.org/z/KovAZZ

      Took me a while to get my jaw off the floor when I saw that one.
   */
   inline uint32_t Switch_Endianness( uint32_t Word ) {
      const auto Byte_1 = ( Word & 0x000000FF ) >> 0;
      const auto Byte_2 = ( Word & 0x0000FF00 ) >> 8;
      const auto Byte_3 = ( Word & 0x00FF0000 ) >> 16;
      const auto Byte_4 = ( Word & 0xFF000000 ) >> 24;

      return ( Byte_1 << 24 | Byte_2 << 16 | Byte_3 << 8 | Byte_4 << 0 );
   }

   /**
      Return a string of the binary representation of the given integer.

      Useful for printing the binary form of numbers. In particular:

         Integer_To_Binary_String( 0b01011001 )

      Should return "0b01011001".
   */
   string Integer_To_Binary_String( uint32_t Integer );

   /// Return a readable description of the contents of the given Mstatus.
   string Mstatus_To_String( mstatus M );

   // Some useful things to assert

   constexpr bool Unreachable = false;
   constexpr bool Unimplemented = false;
   constexpr bool Deliberate_Crash = false;

} // namespace util
