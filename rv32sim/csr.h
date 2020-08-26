#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>

/*

   I guess at this early stage I've still time to think about API design.

   Generally speaking we'll get here from an integer in the `csr` command.

   Something like 0x304. That gets looked up in CSR_Code which gives a csr_name.

   Depending on csr_name, we might want to return a struct with bitfields for
   accessing the CSR's bits.

   What I think I want is a discriminated record. You can do this in D and Ada.
   Give a type parameter to the struct and change the bitfields depending on
   that parameter.

   There must be some way to do this in C++, probably using templates. I'll only
   commit to something like that if it's reasonably simple, though.

 */

enum csr_code : uint16_t {
   INVALID_CSR_NAME = 0xFFFF,
   CSR_MVENDORID    = 0xF11,
   CSR_MARCHID      = 0xF12,
   CSR_MIMPID       = 0xF13,
   CSR_MHARTID      = 0xF14,
   CSR_MSTATUS      = 0x300,
   CSR_MISA         = 0x301,
   CSR_MIE          = 0x304,
   CSR_MTVEC        = 0x305,
   CSR_MSCRATCH     = 0x340,
   CSR_MEPC         = 0x341,
   CSR_MCAUSE       = 0x342,
   CSR_MTVAL        = 0x343,
   CSR_MIP          = 0x344,
   NUM_CSR_CODES    = 14,

};

inline const char *CSR_To_String( csr_code CSR ) {
   switch ( CSR ) {
      case INVALID_CSR_NAME: return "INVALID_CSR_NAME";
      case CSR_MVENDORID: return "CSR_MVENDORID";
      case CSR_MARCHID: return "CSR_MARCHID";
      case CSR_MIMPID: return "CSR_MIMPID";
      case CSR_MHARTID: return "CSR_MHARTID";
      case CSR_MSTATUS: return "CSR_MSTATUS";
      case CSR_MISA: return "CSR_MISA";
      case CSR_MIE: return "CSR_MIE";
      case CSR_MTVEC: return "CSR_MTVEC";
      case CSR_MSCRATCH: return "CSR_MSCRATCH";
      case CSR_MEPC: return "CSR_MEPC";
      case CSR_MCAUSE: return "CSR_MCAUSE";
      case CSR_MTVAL: return "CSR_MTVAL";
      case CSR_MIP: return "CSR_MIP";
      case NUM_CSR_CODES: return "NUM_CSR_CODES";
      default: return "??? CSR ???";
   }
}

// -----------------------------------------------------------------------------

struct mstatus {
   unsigned UIE : 1;
   unsigned SIE : 1;
   unsigned WPRI_1 : 1;
   unsigned MIE : 1;
   unsigned UPIE : 1;
   unsigned SPIE : 1;
   unsigned WPRI_2 : 1;
   unsigned MPIE : 1;
   unsigned SPP : 1;
   unsigned WPRI_3 : 2;
   unsigned MPP : 2;
   unsigned FS : 2;
   unsigned XS : 2;
   unsigned MPRV : 1;
   unsigned SUM : 1;
   unsigned MXR : 1;
   unsigned TVM : 1;
   unsigned TW : 1;
   unsigned TSR : 1;
   unsigned WPRI_4 : 8;
   unsigned SD : 1;
};

// -----------------------------------------------------------------------------

struct mip {
   unsigned USIP : 1;
   unsigned SSIP : 1;
   unsigned WIRI_1 : 1;
   unsigned MSIP : 1;
   unsigned UTIP : 1;
   unsigned STIP : 1;
   unsigned WIRI_2 : 1;
   unsigned MTIP : 1;
   unsigned UEIP : 1;
   unsigned SEIP : 1;
   unsigned WIRI_3 : 1;
   unsigned MEIP : 1;
   unsigned WIRI_4 : 20;
};

// -----------------------------------------------------------------------------

struct mie {
   unsigned USIE : 1;
   unsigned SSIE : 1;
   unsigned WPRI_1 : 1;
   unsigned MSIE : 1;
   unsigned UTIE : 1;
   unsigned STIE : 1;
   unsigned WPRI_2 : 1;
   unsigned MTIE : 1;
   unsigned UEIE : 1;
   unsigned SEIE : 1;
   unsigned WPRI_3 : 1;
   unsigned MEIE : 1;
   unsigned WPRI_4 : 20;
};

// -----------------------------------------------------------------------------

struct csr {
   union {
      uint32_t As_Integer;
      mstatus MSTATUS;
      mip MIP;
      mie MIE;
   };

   operator uint32_t() {
      return As_Integer;
   }

   csr()
     : As_Integer( 0 ){};

   csr( uint32_t Integer )
     : As_Integer( Integer ){};
};

// -----------------------------------------------------------------------------

static_assert( sizeof( mstatus ) == sizeof( uint32_t ), "mstatus is borked" );
static_assert( sizeof( mip ) == sizeof( uint32_t ), "mip is borked" );
static_assert( sizeof( mie ) == sizeof( uint32_t ), "mie is borked" );
static_assert( sizeof( csr ) == sizeof( uint32_t ), "csr is borked" );

inline bool CSR_Is_Valid( csr_code CSR ) {
   switch ( CSR ) {
      case INVALID_CSR_NAME: return false;
      case CSR_MVENDORID:
      case CSR_MARCHID:
      case CSR_MIMPID:
      case CSR_MHARTID:
      case CSR_MSTATUS:
      case CSR_MISA:
      case CSR_MIE:
      case CSR_MTVEC:
      case CSR_MSCRATCH:
      case CSR_MEPC:
      case CSR_MCAUSE:
      case CSR_MTVAL:
      case CSR_MIP: return true;
      default: return false;
   }
}

inline bool CSR_Is_Valid( unsigned CSR ) {
   return CSR_Is_Valid( csr_code( CSR ) );
}

inline size_t CSR_To_Index( csr_code CSR ) {
   switch ( CSR ) {
      default: return 0;
      case INVALID_CSR_NAME: return 0;
      case CSR_MVENDORID: return 1;
      case CSR_MARCHID: return 2;
      case CSR_MIMPID: return 3;
      case CSR_MHARTID: return 4;
      case CSR_MSTATUS: return 5;
      case CSR_MISA: return 6;
      case CSR_MIE: return 7;
      case CSR_MTVEC: return 8;
      case CSR_MSCRATCH: return 9;
      case CSR_MEPC: return 10;
      case CSR_MCAUSE: return 11;
      case CSR_MTVAL: return 12;
      case CSR_MIP: return 13;
   }
}

/// Wrapper for convenience
inline size_t CSR_To_Index( unsigned CSR ) {
   return CSR_To_Index( csr_code( CSR ) );
}

inline bool CSR_Is_Writeable( csr_code CSR ) {
   switch ( CSR ) {
      case INVALID_CSR_NAME: return false;

      // Read-only:
      case CSR_MVENDORID: // fall through
      case CSR_MARCHID:
      case CSR_MIMPID:
      case CSR_MHARTID: return false;

      // Read-write:
      case CSR_MSTATUS: // fall through
      case CSR_MISA:
      case CSR_MIE:
      case CSR_MTVEC:
      case CSR_MSCRATCH:
      case CSR_MEPC:
      case CSR_MCAUSE:
      case CSR_MTVAL:
      case CSR_MIP: return true;

      default: return false;
   }
}

inline bool CSR_Is_Writeable( unsigned CSR ) {
   return CSR_Is_Writeable( csr_code( CSR ) );
}
