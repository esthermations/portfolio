#pragma once

#include "csr.h"
#include "processor.h"

#include <cassert>
#include <cstdint>
#include <cstdio>

/*

   This file is called rv32i.h but I suppose that's not technically true for
   Stage 2, since it deals with CSRs and such. In any case, this is just all the
   "make instructions do stuff" code.

   General structure is that each instruction type (e.g. R type) is given an
   associated struct with bitfield members corresponding to Opcode, RS1, etc.,
   and an Execute() function which takes a CPU to execute on, and an instruction
   ID indicating what should be done with the struct's fields.

   This organisation is a bit more copy-pastey than I'd like... I suppose
   inheritance would clean things up a bit, but I'm not aware of a way to add
   members to an inherited union.

   An instruction type (instr, I think it's called) is a union of all the
   different types, and provides an Execute() which just takes an instruction
   ID. All these structs/unions should be the same size as a uint32_t, which is
   enforced by static assertions.

*/

// -----------------------------------------------------------------------------

/// To make DEBUG_LOG() work.
static bool Be_Verbose = true;

static void Set_RV32I_Verbosity( bool Verbose ) {
   Be_Verbose = Verbose;
}

// -----------------------------------------------------------------------------

enum instr_id {
   FIRST_INSTR = 0,
   LUI         = 0,
   AUIPC,
   JAL,
   JALR,
   BEQ,
   BNE,
   BLT,
   BGE,
   BLTU,
   BGEU,
   LB,
   LH,
   LW,
   LBU,
   LHU,
   SB,
   SH,
   SW,
   ADDI,
   SLTI,
   SLTIU,
   XORI,
   ORI,
   ANDI,
   SLLI,
   SRLI,
   SRAI,
   ADD,
   SUB,
   SLL,
   SLT,
   SLTU,
   XOR,
   SRL,
   SRA,
   OR,
   AND,
   FENCE,
   ECALL,
   EBREAK,
   CSRRW,
   CSRRS,
   CSRRC,
   CSRRWI,
   CSRRSI,
   CSRRCI,
   MRET,
   UNKNOWN_INSTR,
   NUM_RV32I_INSTRUCTIONS,
   LAST_INSTR = NUM_RV32I_INSTRUCTIONS,
};

// -----------------------------------------------------------------------------

inline bool Is_Valid_Instruction_ID( instr_id ID ) {
   return ID >= FIRST_INSTR and ID <= LAST_INSTR;
}

// -----------------------------------------------------------------------------

enum instr_type {
   INSTR_TYPE_R = 0,
   INSTR_TYPE_I,
   INSTR_TYPE_S,
   INSTR_TYPE_B,
   INSTR_TYPE_U,
   INSTR_TYPE_J,
   INSTR_TYPE_CSR,
   INSTR_TYPE_UNIMPLEMENTED,
   NUM_INSTR_TYPES = INSTR_TYPE_UNIMPLEMENTED
};

constexpr uint32_t Instr_Type_Mask[NUM_INSTR_TYPES] = {
  [INSTR_TYPE_R]   = 0b11111110000000000111000001111111, // funct7, funct3, op
  [INSTR_TYPE_I]   = 0b00000000000000000111000001111111, // funct3, op
  [INSTR_TYPE_S]   = 0b00000000000000000111000001111111,
  [INSTR_TYPE_B]   = 0b00000000000000000111000001111111,
  [INSTR_TYPE_U]   = 0b00000000000000000000000001111111, // op
  [INSTR_TYPE_J]   = 0b00000000000000000000000001111111,
  [INSTR_TYPE_CSR] = 0b11111111111100000111000001111111, // funct12, funct3, op
};

constexpr instr_type Instr_Type_Mapping[NUM_RV32I_INSTRUCTIONS] = {
  [instr_id::LUI]    = INSTR_TYPE_U,
  [instr_id::AUIPC]  = INSTR_TYPE_U,
  [instr_id::JAL]    = INSTR_TYPE_J,
  [instr_id::JALR]   = INSTR_TYPE_I,
  [instr_id::BEQ]    = INSTR_TYPE_B,
  [instr_id::BNE]    = INSTR_TYPE_B,
  [instr_id::BLT]    = INSTR_TYPE_B,
  [instr_id::BGE]    = INSTR_TYPE_B,
  [instr_id::BLTU]   = INSTR_TYPE_B,
  [instr_id::BGEU]   = INSTR_TYPE_B,
  [instr_id::LB]     = INSTR_TYPE_I,
  [instr_id::LH]     = INSTR_TYPE_I,
  [instr_id::LW]     = INSTR_TYPE_I,
  [instr_id::LBU]    = INSTR_TYPE_I,
  [instr_id::LHU]    = INSTR_TYPE_I,
  [instr_id::SB]     = INSTR_TYPE_S,
  [instr_id::SH]     = INSTR_TYPE_S,
  [instr_id::SW]     = INSTR_TYPE_S,
  [instr_id::ADDI]   = INSTR_TYPE_I,
  [instr_id::SLTI]   = INSTR_TYPE_I,
  [instr_id::SLTIU]  = INSTR_TYPE_I,
  [instr_id::XORI]   = INSTR_TYPE_I,
  [instr_id::ORI]    = INSTR_TYPE_I,
  [instr_id::ANDI]   = INSTR_TYPE_I,
  [instr_id::SLLI]   = INSTR_TYPE_R, // R with rs2 = shamt
  [instr_id::SRLI]   = INSTR_TYPE_R, // R with rs2 = shamt
  [instr_id::SRAI]   = INSTR_TYPE_R, // R with rs2 = shamt
  [instr_id::ADD]    = INSTR_TYPE_R,
  [instr_id::SUB]    = INSTR_TYPE_R,
  [instr_id::SLL]    = INSTR_TYPE_R,
  [instr_id::SLT]    = INSTR_TYPE_R,
  [instr_id::SLTU]   = INSTR_TYPE_R,
  [instr_id::XOR]    = INSTR_TYPE_R,
  [instr_id::SRL]    = INSTR_TYPE_R,
  [instr_id::SRA]    = INSTR_TYPE_R,
  [instr_id::OR]     = INSTR_TYPE_R,
  [instr_id::AND]    = INSTR_TYPE_R,
  [instr_id::FENCE]  = INSTR_TYPE_UNIMPLEMENTED,
  [instr_id::ECALL]  = INSTR_TYPE_I,
  [instr_id::EBREAK] = INSTR_TYPE_I,
  [instr_id::CSRRW]  = INSTR_TYPE_CSR,
  [instr_id::CSRRS]  = INSTR_TYPE_CSR,
  [instr_id::CSRRC]  = INSTR_TYPE_CSR,
  [instr_id::CSRRWI] = INSTR_TYPE_CSR, // CSR with rs1=zimm
  [instr_id::CSRRSI] = INSTR_TYPE_CSR, // CSR with rs1=zimm
  [instr_id::CSRRCI] = INSTR_TYPE_CSR, // CSR with rs1=zimm
  [instr_id::MRET]   = INSTR_TYPE_CSR, // CSR with rs1=0, rd=0, csr=funct12.
  [instr_id::UNKNOWN_INSTR] = INSTR_TYPE_UNIMPLEMENTED,
};

const char *Instr_String_Mapping[NUM_RV32I_INSTRUCTIONS] = {
  [instr_id::LUI] = "lui",       [instr_id::AUIPC] = "auipc",
  [instr_id::JAL] = "jal",       [instr_id::JALR] = "jalr",
  [instr_id::BEQ] = "beq",       [instr_id::BNE] = "bne",
  [instr_id::BLT] = "blt",       [instr_id::BGE] = "bge",
  [instr_id::BLTU] = "bltu",     [instr_id::BGEU] = "bgeu",
  [instr_id::LB] = "lb",         [instr_id::LH] = "lh",
  [instr_id::LW] = "lw",         [instr_id::LBU] = "lbu",
  [instr_id::LHU] = "lhu",       [instr_id::SB] = "sb",
  [instr_id::SH] = "sh",         [instr_id::SW] = "sw",
  [instr_id::ADDI] = "addi",     [instr_id::SLTI] = "slti",
  [instr_id::SLTIU] = "sltiu",   [instr_id::XORI] = "xori",
  [instr_id::ORI] = "ori",       [instr_id::ANDI] = "andi",
  [instr_id::SLLI] = "slli",     [instr_id::SRLI] = "srli",
  [instr_id::SRAI] = "srai",     [instr_id::ADD] = "add",
  [instr_id::SUB] = "sub",       [instr_id::SLL] = "sll",
  [instr_id::SLT] = "slt",       [instr_id::SLTU] = "sltu",
  [instr_id::XOR] = "xor",       [instr_id::SRL] = "srl",
  [instr_id::SRA] = "sra",       [instr_id::OR] = "or",
  [instr_id::AND] = "and",       [instr_id::FENCE] = "fence",
  [instr_id::ECALL] = "ecall",   [instr_id::EBREAK] = "ebreak",
  [instr_id::CSRRW] = "csrrw",   [instr_id::CSRRS] = "csrrs",
  [instr_id::CSRRC] = "csrrc",   [instr_id::CSRRWI] = "csrrwi",
  [instr_id::CSRRSI] = "csrrsi", [instr_id::CSRRCI] = "csrrci",
  [instr_id::MRET] = "mret",
};

// -----------------------------------------------------------------------------

constexpr uint32_t Unique_Mask[NUM_RV32I_INSTRUCTIONS] = {
  // clang-format off
   //        funct7           funct3      opcode
   [LUI]    = 0b00000000000000000000000000110111,
   [AUIPC]  = 0b00000000000000000000000000010111,
   [JAL]    = 0b00000000000000000000000001101111,
   [JALR]   = 0b00000000000000000000000001100111,
   [BEQ]    = 0b00000000000000000000000001100011,
   [BNE]    = 0b00000000000000000001000001100011,
   [BLT]    = 0b00000000000000000100000001100011,
   [BGE]    = 0b00000000000000000101000001100011,
   [BLTU]   = 0b00000000000000000110000001100011,
   [BGEU]   = 0b00000000000000000111000001100011,
   [LB]     = 0b00000000000000000000000000000011,
   [LH]     = 0b00000000000000000001000000000011,
   [LW]     = 0b00000000000000000010000000000011,
   [LBU]    = 0b00000000000000000100000000000011,
   [LHU]    = 0b00000000000000000101000000000011,
   [SB]     = 0b00000000000000000000000000100011,
   [SH]     = 0b00000000000000000001000000100011,
   [SW]     = 0b00000000000000000010000000100011,
   [ADDI]   = 0b00000000000000000000000000010011,
   [SLTI]   = 0b00000000000000000010000000010011,
   [SLTIU]  = 0b00000000000000000011000000010011,
   [XORI]   = 0b00000000000000000100000000010011,
   [ORI]    = 0b00000000000000000110000000010011,
   [ANDI]   = 0b00000000000000000111000000010011,
   [SLLI]   = 0b00000000000000000001000000010011,
   [SRLI]   = 0b00000000000000000101000000010011,
   [SRAI]   = 0b01000000000000000101000000010011,
   [ADD]    = 0b00000000000000000000000000110011,
   [SUB]    = 0b01000000000000000000000000110011,
   [SLL]    = 0b00000000000000000001000000110011,
   [SLT]    = 0b00000000000000000010000000110011,
   [SLTU]   = 0b00000000000000000011000000110011,
   [XOR]    = 0b00000000000000000100000000110011,
   [SRL]    = 0b00000000000000000101000000110011,
   [SRA]    = 0b01000000000000000101000000110011,
   [OR]     = 0b00000000000000000110000000110011,
   [AND]    = 0b00000000000000000111000000110011,
   [FENCE]  = 0b00000000000000000000000000001111,
   // NOTE: ECALL and EBREAK will *only ever* be these exact numbers. There are
   // no fields. So these aren't like the others -- just check for equality. 
   [ECALL]  = 0b00000000000000000000000001110011,
   [EBREAK] = 0b00000000000100000000000001110011,
   [CSRRW]  = 0b00000000000000000001000001110011, 
   [CSRRS]  = 0b00000000000000000010000001110011,
   [CSRRC]  = 0b00000000000000000011000001110011,
   [CSRRWI] = 0b00000000000000000101000001110011,
   [CSRRSI] = 0b00000000000000000110000001110011,
   [CSRRCI] = 0b00000000000000000111000001110011,
   [MRET] = 0 /* TODO Unique_Mask for MRET */,
  // clang-format on
};

/*
  -----------------------------------------------------------------------------

  Structure definitions

  -----------------------------------------------------------------------------
*/

struct r_type {
   unsigned Opcode : 7; // 06..00
   unsigned RD : 5;     // 11..07
   unsigned Funct3 : 3; // 14..12
   unsigned RS1 : 5;    // 19..15
   unsigned RS2 : 5;    // 24..20
   unsigned Funct7 : 7; // 31..25

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );
      assert( ID != UNKNOWN_INSTR );
      assert( Instr_Type_Mapping[ID] == INSTR_TYPE_R );

      const uint32_t A       = CPU->get_reg( RS1 );
      const uint32_t B       = CPU->get_reg( RS2 );
      const int32_t A_Signed = int32_t( A );
      const int32_t B_Signed = int32_t( B );
      uint32_t Result        = 0;

      switch ( ID ) {
         case instr_id::ADD: Result = A + B; break;
         case instr_id::SUB: Result = A - B; break;
         case instr_id::XOR: Result = A ^ B; break;
         case instr_id::OR: Result = A | B; break;
         case instr_id::AND: Result = A & B; break;
         case instr_id::SLT: Result = ( A_Signed < B_Signed ? 1 : 0 ); break;
         case instr_id::SLTU: Result = ( A < B ? 1 : 0 ); break;
         case instr_id::SLL: Result = A << ( B & 0b11111 ); break;
         case instr_id::SRL: Result = A >> ( B & 0b11111 ); break;
         case instr_id::SRA: Result = A_Signed >> ( B & 0b11111 ); break;
         // These ones use RS2 as shift amount, rather than a register number.
         case instr_id::SLLI: Result = A << ( RS2 ); break;
         case instr_id::SRLI: Result = A >> ( RS2 ); break;
         case instr_id::SRAI: Result = A_Signed >> ( RS2 ); break;
         default: assert( util::Unreachable );
      }

      CPU->set_reg( RD, Result );

      return Successful_Execution;
   }
};

// -----------------------------------------------------------------------------

struct i_type {
   unsigned Opcode : 7;             // 01..07
   unsigned RD : 5;                 // 08..12
   unsigned Funct3 : 3;             // 13..15
   unsigned RS1 : 5;                // 16..20
   unsigned Immediate_11_To_0 : 12; // 21..32

   uint32_t Decipher_Immediate( void ) const {
      return Immediate_11_To_0;
   }

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );
      assert( Instr_Type_Mapping[ID] == INSTR_TYPE_I );

      const uint32_t A         = CPU->get_reg( RS1 );
      const int32_t A_Signed   = int32_t( A );
      const uint32_t Imm       = util::Sign_Extend( Immediate_11_To_0, 12 );
      const int32_t Imm_Signed = int32_t( Imm );

      const auto Load_Address = ( Imm + CPU->get_reg( RS1 ) );

      constexpr uint32_t Default_Result = 0xfeedbeef;
      uint32_t Result                   = Default_Result;

      // Stage 2:
      if ( ( ( ID == instr_id::LW ) and ( Load_Address % 4 != 0 ) ) or
           ( ( ID == instr_id::LH ) and ( Load_Address % 2 != 0 ) ) or
           ( ( ID == instr_id::LHU ) and ( Load_Address % 2 != 0 ) ) ) {
         DEBUG_LOG( "Misaligned load from address %08x", Load_Address );
         return execution_result::EXC_LOAD_ADDRESS_MISALIGNED;
      }

      switch ( ID ) {
         case instr_id::ADDI: Result = A + Imm; break;
         case instr_id::XORI: Result = A ^ Imm; break;
         case instr_id::ORI: Result = A | Imm; break;
         case instr_id::ANDI: Result = A & Imm; break;
         case instr_id::SLTI: Result = ( A_Signed < Imm_Signed ); break;
         case instr_id::SLTIU: Result = ( A < Imm ); break;
         case instr_id::SLLI: Result = A << ( Imm & 0b11111 ); break;
         case instr_id::SRLI: Result = A >> ( Imm & 0b11111 ); break;
         case instr_id::SRAI:
            Result = A_Signed >> ( Imm & 0b11111 );
            break;

            /*
               From the spec, page 21:

               ---
               Load and store instructions transfer a value between the
               registers and memory. Loads are encoded in the I-type format and
               stores are S-type. The effective byte address is obtained by
               adding register rs1 to the sign-extended 12-bit offset. Loads
               copy a value from memory to register rd. Stores copy the value in
               register rs2 to memory.

               The LW instruction loads a 32-bit value from memory into rd. LH
               loads a 16-bit value from memory, then sign-extends to 32-bits
               before storing in rd. LHU loads a 16-bit value from memory but
               then zero extends to 32-bits before storing in rd. LB and LBU are
               defined analogously for 8-bit values. The SW, SH, and SB
               instructions store 32-bit, 16-bit, and 8-bit values from the low
               bits of register rs2 to memory.
               ---
            */

         case instr_id::LB: {
            Result            = CPU->Main_Memory->read_word( Load_Address );
            const auto Offset = ( Load_Address % 4 );
            const auto Mask   = 0xFF << ( 8 * Offset );
            Result            = ( Result & Mask );
            Result >>= ( 8 * Offset );
            Result = util::Sign_Extend( Result, 8 );
            break;
         }

         case instr_id::LH: {
            Result            = CPU->Main_Memory->read_word( Load_Address );
            const auto Offset = ( Load_Address % 4 );
            const auto Mask   = 0xFFFF << ( 8 * Offset );
            Result            = ( Result & Mask );
            Result >>= ( 8 * Offset );
            Result = util::Sign_Extend( Result, 16 );
            break;
         }

         case instr_id::LW: {
            Result = CPU->Main_Memory->read_word( Load_Address );
            Result = util::Sign_Extend( Result, 32 );
            break;
         }

         case instr_id::LBU: {
            Result            = CPU->Main_Memory->read_word( Load_Address );
            const auto Offset = ( Load_Address % 4 );
            const auto Mask   = 0xFF << ( 8 * Offset );
            Result            = ( Result & Mask );
            Result >>= ( 8 * Offset );
            Result = util::Zero_Extend( Result, 8 );
            break;
         }

         case instr_id::LHU: {
            Result            = CPU->Main_Memory->read_word( Load_Address );
            const auto Offset = ( Load_Address % 4 );
            const auto Mask   = 0xFFFF << ( 8 * Offset );
            Result            = ( Result & Mask );
            Result >>= ( 8 * Offset );
            Result = util::Zero_Extend( Result, 16 );
            break;
         }

         case instr_id::JALR: {
            /*
               From the spec, page 18:

               ---
               The indirect jump instruction JALR (jump and link register) uses
               the I-type encoding. The target address is obtained by adding the
               sign-extended 12-bit I-immediate to the register rs1, then
               setting the least-significant bit of the result to zero. The
               address of the instruction following the jump (pc+4) is written
               to register rd. Register x0 can be used as the destination if the
               result is not required.
               ---

            */

            auto Jump_Target = ( CPU->get_reg( RS1 ) + Imm - 4 );
            Jump_Target      = util::Set_Bit( Jump_Target, 1, 0 );

            DEBUG_LOG( "JALR jumping to %08x", Jump_Target );

            if ( not util::Address_Is_Word_Aligned( Jump_Target ) ) {
               DEBUG_LOG( RED( "Jump target isn't word aligned!" ) );
            }

            Result = ( CPU->get_pc() + 4 );
            CPU->set_pc( Jump_Target );
            break;
         }

         case instr_id::ECALL: {
            if ( not CPU->Stage2 ) {
               puts( "ecall: not implemented" );
               return Successful_Execution;
            }

            return ( CPU->get_prv() == PRIV_MACHINE
                       ? execution_result::EXC_ECALL_FROM_MACHINE_MODE
                       : execution_result::EXC_ECALL_FROM_USER_MODE );
         }

         case instr_id::EBREAK: {
            if ( not CPU->Stage2 ) {
               puts( "ebreak: not implemented" );
               return Successful_Execution;
            }

            return execution_result::EXC_BREAKPOINT;
         }

         default:
            DEBUG_LOG( "Got an invalid instruction: %s",
                       Instr_String_Mapping[ID] );
            assert( util::Unreachable );
      }

      if ( Be_Verbose and Result == Default_Result ) {
         DEBUG_LOG( "i_type::Execute() result was 0x%x.", Default_Result );
         DEBUG_LOG( "This is the value I use as a default." );
         DEBUG_LOG( RED( "This had better be a freak occurrence!" ) );
      }

      CPU->set_reg( RD, Result );

      return Successful_Execution;
   }
};

// -----------------------------------------------------------------------------

struct s_type {
   unsigned Opcode : 7;            // 06..00
   unsigned Immediate_4_To_0 : 5;  // 11..07
   unsigned Funct3 : 3;            // 14..12
   unsigned RS1 : 5;               // 19..15
   unsigned RS2 : 5;               // 24..20
   unsigned Immediate_11_To_5 : 7; // 31..25

   uint32_t Decipher_Immediate( void ) const {
      return ( Immediate_11_To_5 << 5 | Immediate_4_To_0 << 0 );
   }

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );
      assert( Instr_Type_Mapping[ID] == INSTR_TYPE_S );

      /*
         Per the spec, page 21:

         ----
         Load and store instructions transfer a value between the registers and
         memory. Loads are encoded in the I-type format and stores are S-type.
         The effective byte address is obtained by adding register rs1 to the
         sign-extended 12-bit offset. Loads copy a value from memory to register
         rd. Stores copy the value in register rs2 to memory.
         ----

         Note: "Effective byte address". This means the byte address doesn't
         have to be word-aligned, and we should extract the corresponding byte,
         not just the least-significant byte in the given word.
      */

      const uint32_t Imm = util::Sign_Extend( Decipher_Immediate(), 12 );

      const uint32_t Address = ( CPU->get_reg( this->RS1 ) + Imm );
      uint32_t Data          = ( CPU->get_reg( this->RS2 ) );

      // Stage 2:
      if ( ( ( ID == instr_id::SW ) and ( Address % 4 != 0 ) ) or
           ( ( ID == instr_id::SH ) and ( Address % 2 != 0 ) ) ) {
         DEBUG_LOG( "Misaligned store to address %08x", Address );
         return execution_result::EXC_STORE_ADDRESS_MISALIGNED;
      }

      switch ( ID ) {
         case instr_id::SB: {
            const auto Byte_Offset = ( 8 * ( Address % 4 ) );
            const auto Mask        = 0xFF << Byte_Offset;
            Data                   = ( Data & 0xFF ) << Byte_Offset;
            CPU->Main_Memory->write_word( Address, Data, Mask );
            break;
         }
         case instr_id::SH: {
            const auto Byte_Offset = ( 8 * ( Address % 4 ) );
            const auto Mask        = 0xFFFF << Byte_Offset;
            Data                   = ( Data & 0xFFFF ) << Byte_Offset;
            CPU->Main_Memory->write_word( Address, Data, Mask );
            break;
         }
         case instr_id::SW: {
            CPU->Main_Memory->write_word( Address, Data, 0xFFFFFFFF );
            break;
         }
         default: assert( util::Unreachable );
      }

      return Successful_Execution;
   }
};

// -----------------------------------------------------------------------------

struct b_type {
   unsigned Opcode : 7;            // 06..00
   unsigned Immediate_11 : 1;      // 07
   unsigned Immediate_4_To_1 : 4;  // 11..08
   unsigned Funct3 : 3;            // 14..12
   unsigned RS1 : 5;               // 19..15
   unsigned RS2 : 5;               // 24..20
   unsigned Immediate_10_To_5 : 6; // 30..25
   unsigned Immediate_12 : 1;      // 31

   uint32_t Decipher_Immediate( void ) const {
      static_assert( 0b000001 << 11 == 0b100000000000, "" );
      static_assert( 0b000001 << 10 == 0b010000000000, "" );
      static_assert( 0b111111 << 4 == 0b001111110000, "" );
      static_assert( 0b001111 << 0 == 0b000000001111, "" );
      return ( Immediate_12 << 11 | Immediate_11 << 10 |
               Immediate_10_To_5 << 4 | Immediate_4_To_1 << 0 );
   }

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );
      assert( Instr_Type_Mapping[ID] == INSTR_TYPE_B );

      /*
         Per the spec, page 19:

         ----
         All branch instructions use the B-type instruction format. The 12-bit
         B-immediate encodes signed offsets in multiples of 2, and is added to
         the current pc to give the target address.
         ----
      */

      const uint32_t Offset = 2 * util::Sign_Extend( Decipher_Immediate(), 12 );

      const uint32_t Branch_Target = ( CPU->get_pc() + Offset - 4 );
      // Subtract 4 from Branch_Target to account for processor::execute()
      // adding 4 every time an instruction is executed. Hacky, but it'll do.

      // We're just setting PC here, and not adding +4. The textbook diagrams
      // had a branch control line fed into a multiplexor to either set PC =
      // PC+4 or a branch target. So we're *not* adding 4 here, we're just
      // setting PC. Which I'm pretty sure is correct.

      const int32_t A   = CPU->get_reg( this->RS1 );
      const int32_t B   = CPU->get_reg( this->RS2 );
      const uint32_t AU = uint32_t( A );
      const uint32_t BU = uint32_t( B );

      bool Should_Branch = false;

      switch ( ID ) {
         case instr_id::BEQ: Should_Branch = ( A == B ); break;
         case instr_id::BNE: Should_Branch = ( A != B ); break;
         case instr_id::BLT: Should_Branch = ( A < B ); break;
         case instr_id::BGE: Should_Branch = ( A >= B ); break;
         case instr_id::BLTU: Should_Branch = ( AU < BU ); break;
         case instr_id::BGEU: Should_Branch = ( AU >= BU ); break;
         default: assert( util::Unreachable );
      }

      if ( Should_Branch ) {
         DEBUG_LOG( GREEN( "Taking branch." ) );
         CPU->set_pc( Branch_Target );
      } else {
         DEBUG_LOG( YELLOW( "NOT taking branch." ) );
      }

      return Successful_Execution;
   }
};

// static_assert( (b_type{
// // 0xfe209de3 -> 0b1111'1110'0010'0000'1001'1101'1110'0011
// Opcode                    : 0b1100011, // bne
// Immediate_11              : 0b1,
// Immediate_4_To_1          : 0b1101,
// Funct3                    : 0b001,    // bne
// RS1                       : 0b00001,  // x1
// RS2                       : 0b00010,  // x2
// Immediate_10_To_5         : 0b111111,
// Immediate_12              : 0b1
// }).Decipher_Immediate() == 0b111111111101 );

// -----------------------------------------------------------------------------

struct u_type {
   unsigned Opcode : 7;              // 06..00
   unsigned RD : 5;                  // 11..07
   unsigned Immediate_31_To_12 : 20; // 31..12

   uint32_t Decipher_Immediate( void ) const {
      return Immediate_31_To_12;
   }

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );
      assert( Instr_Type_Mapping[ID] == INSTR_TYPE_U );

      /*
         Per the spec, page 16:

         ----
         LUI (load upper immediate) is used to build 32-bit constants and uses
         the U-type format. LUI places the U-immediate value in the top 20 bits
         of the destination register rd, filling in the lowest 12 bits with
         zeros.

         AUIPC (add upper immediate to pc) is used to build pc-relative
         addresses and uses the U-type format. AUIPC forms a 32-bit offset from
         the 20-bit U-immediate, filling in the lowest 12 bits with zeros, adds
         this offset to the pc of the AUIPC instruction, then places the result
         in register rd.
         ----

      */

      uint32_t Imm = ( Immediate_31_To_12 << 12 );

      assert( util::Get_Bit( Imm, 32 ) ==
              util::Get_Bit( Immediate_31_To_12, 20 ) );

      switch ( ID ) {
         case instr_id::LUI: CPU->set_reg( RD, Imm ); break;
         case instr_id::AUIPC: CPU->set_reg( RD, Imm + CPU->get_pc() ); break;
         default: assert( util::Unreachable );
      }

      return Successful_Execution;
   }
};

// -----------------------------------------------------------------------------

struct j_type {
   // Sigh.
   unsigned Opcode : 7;                                           // 06..00
   unsigned RD : 5;                                               // 11..07
   unsigned Immediate_20_Then_10_To_1_Then_11_Then_19_To_12 : 20; // 31..12

   //
   uint32_t Decipher_Immediate( void ) const {
      // Whole bunch of tests here to ensure I'm not making any mistakes.

      // clang-format off
      constexpr uint32_t Mask_20         = 0b10000000000000000000;
      constexpr uint32_t Mask_19_To_12   = 0b00000000000011111111;
      constexpr uint32_t Mask_11         = 0b00000000000100000000;
      constexpr uint32_t Mask_10_To_1    = 0b01111111111000000000;

      constexpr uint32_t Target_20       = 0b10000000000000000000;
      constexpr uint32_t Target_19_To_12 = 0b01111111100000000000;
      constexpr uint32_t Target_11       = 0b00000000010000000000;
      constexpr uint32_t Target_10_To_1  = 0b00000000001111111111;
      // clang-format on

      {
         // A 1 should appear exactly once for each bit above, which is what XOR
         // will tell us. I can see it visually up there, but best to be sure.
         constexpr auto Xor_All =
           ( Target_20 ^ Target_19_To_12 ^ Target_11 ^ Target_10_To_1 );

         static_assert( Xor_All == 0b11111111111111111111, "" );
      }

      constexpr auto Shift_20       = 0;
      constexpr auto Shift_19_To_12 = 11; // Left
      constexpr auto Shift_11       = 2;  // Left
      constexpr auto Shift_10_To_1  = 9;  // Right

      static_assert( Mask_20 >> Shift_20 == Target_20, "" );
      static_assert( Mask_19_To_12 << Shift_19_To_12 == Target_19_To_12, "" );
      static_assert( Mask_11 << Shift_11 == Target_11, "" );
      static_assert( Mask_10_To_1 >> Shift_10_To_1 == Target_10_To_1, "" );

      // End tests

      const uint32_t Imm = Immediate_20_Then_10_To_1_Then_11_Then_19_To_12;

      const uint32_t Bit_20       = ( Imm & Mask_20 ) >> Shift_20;
      const uint32_t Bit_19_To_12 = ( Imm & Mask_19_To_12 ) << Shift_19_To_12;
      const uint32_t Bit_11       = ( Imm & Mask_11 ) << Shift_11;
      const uint32_t Bit_10_To_1  = ( Imm & Mask_10_To_1 ) >> Shift_10_To_1;

      return ( 0 | Bit_20 | Bit_19_To_12 | Bit_11 | Bit_10_To_1 );
   }

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );
      assert( Instr_Type_Mapping[ID] == INSTR_TYPE_J );
      assert( ID == instr_id::JAL );

      /*

         Per the spec, page 18:

         ----
         The jump and link (JAL) instruction uses the J-type format, where the
         J-immediate encodes a signed offset in multiples of 2 bytes. The offset
         is sign-extended and added to the pc to form the jump target address.
         Jumps can therefore target a ±1MiB range. JAL stores the address of the
         instruction following the jump (pc+4) into register rd.
         ----

         So this instruction is guaranteed to be JAL.
      */

      const uint32_t Imm =
        ( util::Sign_Extend( 2 * this->Decipher_Immediate(), 20 ) );

      CPU->set_reg( this->RD, CPU->get_pc() + 4 );
      CPU->set_pc( CPU->get_pc() + Imm - 4 );
      // Subtract 4 to account for processor::execute() adding 4 when it
      // executes an instruction.

      return Successful_Execution;
   }
};

// ----------------------------------------------------------------------------

struct csr_type {
   unsigned Opcode : 7;
   unsigned RD : 5;
   unsigned Funct3 : 3;
   unsigned RS1 : 5;
   unsigned CSR : 12;

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );
      assert( Instr_Type_Mapping[ID] == INSTR_TYPE_CSR );
      // TODO: csr_type::Execute().

      if ( not CPU->Stage2 ) {
         return Successful_Execution; // @Required for stage 1
      }

      // MRET uses the CSR field as Funct12, so it won't look like a valid CSR.
      if ( ID != MRET and not CSR_Is_Valid( CSR ) ) {
         DEBUG_LOG( "Illegal instruction: invalid CSR." );
         return execution_result::EXC_ILLEGAL_INSTRUCTION;
      }

      const auto Privelige = CPU->get_prv();

      const auto Can_Write_CSR =
        ( Privelige == PRIV_MACHINE and CSR_Is_Writeable( CSR ) );

      switch ( ID ) {
         case CSRRWI: // Fall through
         case CSRRW: {
            if ( not Can_Write_CSR ) {
               return execution_result::EXC_ILLEGAL_INSTRUCTION;
            }

            if ( RD != 0 ) {
               // Only read the CSR if RD isn't x0
               uint32_t Old_CSR = CPU->get_csr( this->CSR );
               Old_CSR          = util::Zero_Extend( Old_CSR, 32 );
               CPU->set_reg( this->RD, Old_CSR );
            }

            CPU->set_csr( this->CSR,
                          ( ID == CSRRWI ? util::Zero_Extend( this->RS1, 32 )
                                         : CPU->get_reg( this->RS1 ) ) );
            break;
         }

         case CSRRSI:
         case CSRRS: {
            if ( RS1 != 0 and not Can_Write_CSR ) {
               return execution_result::EXC_ILLEGAL_INSTRUCTION;
            }

            uint32_t Old_CSR = CPU->get_csr( this->CSR );
            Old_CSR          = util::Zero_Extend( Old_CSR, 32 );

            const uint32_t Mask =
              ( ID == CSRRSI ? util::Zero_Extend( this->RS1, 32 )
                             : CPU->get_reg( this->RS1 ) );

            CPU->set_reg( this->RD, Old_CSR );

            if ( this->RS1 != 0 ) {
               CPU->set_csr( this->CSR, Old_CSR | Mask, from_instr( true ) );
            }
            break;
         }

         case CSRRCI:
         case CSRRC: {
            if ( RS1 != 0 and not Can_Write_CSR ) {
               return execution_result::EXC_ILLEGAL_INSTRUCTION;
            }

            uint32_t Old_CSR = CPU->get_csr( this->CSR );
            Old_CSR          = util::Zero_Extend( Old_CSR, 32 );

            const uint32_t Mask =
              ( ID == CSRRCI ? util::Zero_Extend( this->RS1, 32 )
                             : CPU->get_reg( this->RS1 ) );

            CPU->set_reg( this->RD, Old_CSR );

            if ( this->RS1 != 0 ) {
               CPU->set_csr( this->CSR, Old_CSR & ~Mask );
            }
            break;
         }

         case MRET: {
            if ( Privelige != PRIV_MACHINE ) {
               DEBUG_LOG( "Privelige isn't high enough for MRET." );
               return execution_result::EXC_ILLEGAL_INSTRUCTION;
            }

            DEBUG_LOG( "MRET instruction. Setting PC to MEPC." );
            CPU->set_pc( CPU->get_csr( CSR_MEPC ) - 4 );

            /*
               From the spec, with x substituted for M:

               When executing an MRET instruction, supposing MPP holds the value
               y, MIE is set to MPIE; the privilege mode is changed to y; MPIE
               is set to 1; and MPP is set to U.
             */

            DEBUG_LOG( "MRET popping privelige stack." );
            csr CSR         = CPU->get_csr( CSR_MSTATUS );
            CSR.MSTATUS.MIE = CSR.MSTATUS.MPIE;
            CPU->set_prv( CSR.MSTATUS.MPP );
            CSR.MSTATUS.MPIE = 1;
            CSR.MSTATUS.MPP  = PRIV_USER;
            CPU->set_csr( CSR_MSTATUS, CSR );
            break;
         }
         default: assert( util::Unreachable );
      }

      return Successful_Execution;
   }
};

// ----------------------------------------------------------------------------

struct instr {
   union {
      uint32_t As_Integer;
      r_type R_Type;
      i_type I_Type;
      s_type S_Type;
      b_type B_Type;
      u_type U_Type;
      j_type J_Type;
      csr_type CSR_Type;
   };

   instr()
     : As_Integer( 0 ) {
   }

   instr( uint32_t Integer )
     : As_Integer( Integer ) {
   }

   operator uint32_t() {
      return As_Integer;
   }

   execution_result Execute( processor *CPU, instr_id ID ) const {
      assert( CPU );

      switch ( Instr_Type_Mapping[ID] ) {
         case INSTR_TYPE_R: return this->R_Type.Execute( CPU, ID );
         case INSTR_TYPE_I: return this->I_Type.Execute( CPU, ID );
         case INSTR_TYPE_S: return this->S_Type.Execute( CPU, ID );
         case INSTR_TYPE_B: return this->B_Type.Execute( CPU, ID );
         case INSTR_TYPE_U: return this->U_Type.Execute( CPU, ID );
         case INSTR_TYPE_J: return this->J_Type.Execute( CPU, ID );

         case INSTR_TYPE_CSR: {
            if ( not CPU->Stage2 ) {
               puts( "Error: illegal instruction" );
               DEBUG_LOG(
                 BLUE( "(This is a CSR instruction. "
                       "It's illegal because Stage2 isn't enabled.)" ) );
               break;
            }

            return this->CSR_Type.Execute( CPU, ID );
         }

         default: {
            if ( ID == FENCE ) {
               // @Required: Do nothing.
               DEBUG_LOG( "Fence instruction. Do nothing." );
               return Successful_Execution;
            }

            if ( CPU->Stage2 ) {
               return execution_result::EXC_ILLEGAL_INSTRUCTION;
            } else {
               // @Required
               puts( "Error: illegal instruction" );
               DEBUG_LOG( "Illegal (unknown) instruction, but not crashing." );
               return Successful_Execution;
            }
         }
      }

      assert( util::Unreachable );
      return Successful_Execution;
   }
};

static_assert( sizeof( r_type ) == sizeof( uint32_t ), "r_type is busted." );
static_assert( sizeof( i_type ) == sizeof( uint32_t ), "i_type is busted." );
static_assert( sizeof( s_type ) == sizeof( uint32_t ), "s_type is busted." );
static_assert( sizeof( b_type ) == sizeof( uint32_t ), "b_type is busted." );
static_assert( sizeof( u_type ) == sizeof( uint32_t ), "u_type is busted." );
static_assert( sizeof( j_type ) == sizeof( uint32_t ), "j_type is busted." );
static_assert( sizeof( csr_type ) == sizeof( uint32_t ),
               "csr_type is busted." );
static_assert( sizeof( instr ) == sizeof( uint32_t ), "instr  is busted." );

// ----------------------------------------------------------------------------

inline instr_id Determine_Instruction_ID( uint32_t Integer ) {
   for ( int I = FIRST_INSTR; I <= LAST_INSTR; ++I ) {
      const auto ID   = instr_id( I );
      const auto Type = Instr_Type_Mapping[ID];

      if ( Type == INSTR_TYPE_UNIMPLEMENTED ) {
         break;
      }

      // Zero out the unimportant fields
      const auto Masked = ( Integer & Instr_Type_Mask[size_t( Type )] );

      if ( Masked == Unique_Mask[ID] ) {
         return ID;
      }
   }

   // At this point, it might still be FENCE or ECALL, EBREAK, etc.

   constexpr uint32_t ECALL_Integer  = 0b00000000000000000000000001110011;
   constexpr uint32_t EBREAK_Integer = 0b00000000000100000000000001110011;
   constexpr uint32_t MRET_Integer   = 0b00110000001000000000000001110011;

   if ( Integer == ECALL_Integer ) {
      return ECALL;
   }

   if ( Integer == EBREAK_Integer ) {
      return EBREAK;
   }

   if ( Integer == MRET_Integer ) {
      return MRET;
   }

   // Check if it's a CSR instruction

   {
      const auto Instr = instr( Integer ).CSR_Type;

      if ( Instr.Opcode == 0b1110011 and Instr.Funct3 != 0b100 ) {
         switch ( Instr.Funct3 ) {
            case 0b001: return CSRRW;
            case 0b010: return CSRRS;
            case 0b011: return CSRRC;
            case 0b101: return CSRRWI;
            case 0b110: return CSRRSI;
            case 0b111: return CSRRCI;
            default: assert( util::Unreachable );
         }
      }
   }

   // Check if it's FENCE

   if ( ( ( Integer & Unique_Mask[FENCE] ) == Unique_Mask[FENCE] ) ) {
      return FENCE;
   }

   return UNKNOWN_INSTR;
}

// clang-format off
static_assert( (    0xa5a58593 
                 &  0b00000000000000000111000000000000 ) 
                 == 0b00000000000000000000000000000000, "" );

static_assert( (    0xa5a58593 
                 &  0b00000000000000000000000001111111 ) 
                 == 0b00000000000000000000000000010011, "" );
// clang-format on

// static_assert( Determine_Instruction_ID( 0xa5a58593 ) != instr_id::LB );
// static_assert( Determine_Instruction_ID( 0xa5a58593 ) == instr_id::ADDI );
// static_assert( Determine_Instruction_ID( 0x0051e933 ) == instr_id::OR );

// ----------------------------------------------------------------------------

tuple<instr, instr_id> Integer_To_Instruction( uint32_t Integer ) {
   const instr Instruction = instr( Integer );
   const auto ID           = Determine_Instruction_ID( Integer );

   if ( ID == UNKNOWN_INSTR ) {
      DEBUG_LOG( "Unrecognised instruction: %08x", Integer );
   }

   return make_tuple( Instruction, ID );
}

// ----------------------------------------------------------------------------

string Instruction_To_Assembly( uint32_t Integer ) {
   instr Instr;
   instr_id ID;
   std::tie( Instr, ID ) = Integer_To_Instruction( Integer );

   if ( ID == UNKNOWN_INSTR ) {
      DEBUG_LOG( "called on an unknown instruction." );
   }

   const auto Name = Instr_String_Mapping[ID];

   const auto R   = Instr.R_Type;
   const auto I   = Instr.I_Type;
   const auto S   = Instr.S_Type;
   const auto B   = Instr.B_Type;
   const auto U   = Instr.U_Type;
   const auto J   = Instr.J_Type;
   const auto CSR = Instr.CSR_Type;

   using namespace util;

   constexpr size_t Length  = 50;
   char Temp_String[Length] = {0};

   // Clang-format has really weird ideas about how this switch should be
   // formatted. I think it doesn't recognise the [[fallthrough]]; attribute, or
   // maybe it does and is just very opinionated about how fallthrough cases
   // should look. I dunno.

   // clang-format off
   switch ( ID ) {

      // U-type:
      case LUI: [[fallthrough]];
      case AUIPC:
         snprintf( Temp_String, Length, "%-6s x%u, %05x", // 20-bit, so %05x
                  Name, U.RD, U.Immediate_31_To_12 );
         break;

      // J-type:
      case JAL:
         snprintf( Temp_String, Length, "%-6s x%u, %d",
                   Name, J.RD,
                   2 * util::Sign_Extend( J.Decipher_Immediate(), 20 ) );
         break;

      // B-type:
      case BEQ: [[fallthrough]];
      case BNE:
      case BLT:
      case BGE:
      case BLTU:
      case BGEU:
         snprintf( Temp_String, Length, "%-6s x%u, x%u, %d",
                  Name, B.RS1, B.RS2,
                  2 * util::Sign_Extend( B.Decipher_Immediate(), 12 ) );
         break;

      // I-type, special case: 
      case JALR: 
         snprintf( Temp_String, Length, "%-6s x%u, x%u, %d",
                   Name, I.RD, I.RS1,
                   util::Sign_Extend( I.Decipher_Immediate(), 12 ) );
         break;

      // I-type, run-of-the-mill:
      case ADDI: [[fallthrough]];
      case SLTI:
      case SLTIU:
      case LB:
      case LH:
      case LW:
      case LBU:
      case LHU:
      case XORI:
      case ORI:
      case ANDI:
      case SRAI:
         snprintf( Temp_String, Length, "%-6s x%u, x%u, %d",
                   Name, I.RD, I.RS1, 
                   util::Sign_Extend( I.Immediate_11_To_0, 12 ) );
         break;

      // S-type:
      case SB: [[fallthrough]];
      case SH:
      case SW: 
         snprintf( Temp_String, Length, "%-6s x%u, x%u, %d",
                   Name, S.RS1, S.RS2, S.Decipher_Immediate() );
         break;

      // R-type but with rs2=shamt:
      case SLLI: [[fallthrough]];
      case SRLI:
         snprintf( Temp_String, Length, "%-6s x%u, x%u, x%u",
                   Name, R.RD, R.RS1, R.RS2 );
         break;

      // R-type:
      case ADD: [[fallthrough]];
      case SUB:
      case SLL:
      case SLT:
      case SLTU:
      case XOR:
      case SRL:
      case SRA:
      case OR:
      case AND:
         snprintf( Temp_String, Length, "%-6s x%u, x%u, x%u",
                   Name, R.RD, R.RS1, R.RS2 );
         break;

      case FENCE:  return "fence";
      case ECALL:  return "ecall";
      case EBREAK: return "ebreak";
      case CSRRW: 
      case CSRRS: 
      case CSRRC: 
         snprintf( Temp_String, Length, "%s x%u, x%u, 0x%x", 
                   Name, CSR.RD, CSR.RS1, CSR.CSR ); 
         break;
      case CSRRWI:
      case CSRRSI:
      case CSRRCI:
         snprintf( Temp_String, Length, "%s x%u, %u, 0x%x", 
                   Name, CSR.RD, CSR.RS1, CSR.CSR ); 
         break;
      case MRET: 
         snprintf( Temp_String, Length, "%s", Name ); 
         break;
 
      case UNKNOWN_INSTR: return "[unknown instruction]";
      default: assert( util::Unreachable );
   }
   // clang-format on

   return std::string( Temp_String );
}
