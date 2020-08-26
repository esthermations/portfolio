#include "processor.h"
#include "memory.h"
#include "rv32i.h"
#include "util.h"

#include <cassert>
#include <cstdio>

using namespace std;

// ----------------------------------------------------------------------------

processor::processor( memory *Main_Memory, bool Verbose, bool Stage2 ) {
   this->Main_Memory = Main_Memory;
   this->Be_Verbose  = Verbose;
   this->Stage2      = Stage2;

   this->set_prv( PRIV_MACHINE );

   Set_RV32I_Verbosity( Verbose );
}
// ----------------------------------------------------------------------------

void processor::show_pc( void ) const {
   DEBUG_LOG( "Displaying PC (%08x)", this->PC );
   printf( "%08x\n", this->PC );
}

// ----------------------------------------------------------------------------

void processor::set_pc( uint32_t New_Value ) {
   this->PC = New_Value;
   DEBUG_LOG( "PC <- %08x", New_Value );
}

// ----------------------------------------------------------------------------

uint32_t processor::get_pc( void ) const {
   return this->PC;
}

// ----------------------------------------------------------------------------

uint32_t processor::get_reg( unsigned int Reg_Num ) const {
   assert( Reg_Num <= 31 );
   return ( Reg_Num == 0 ? 0 : this->Register_X[Reg_Num] );
}

// ----------------------------------------------------------------------------

void processor::show_reg( unsigned int Reg_Num ) const {
   assert( Reg_Num <= 31 );
   const auto Value = this->Register_X[Reg_Num];
   DEBUG_LOG( "Displaying register x%u (%08x)", Reg_Num, Value );
   printf( "%08x\n", Value );
}

// ----------------------------------------------------------------------------

void processor::set_reg( unsigned int Reg_Num, uint32_t New_Value ) {
   assert( Reg_Num <= 31 );
   if ( Reg_Num == 0 ) {
      // Can't set x0.
      DEBUG_LOG( "Not setting register x0." );
      return;
   } else {
      this->Register_X[Reg_Num] = New_Value;
      DEBUG_LOG( "x%u <- %08x", Reg_Num, New_Value );
      return;
   }
}

execution_result processor::Check_For_Pending_Interrupts( void ) {
   using ex = execution_result;

   // USER_SOFTWARE_INTERRUPT : (mip.usip and mie.usie) and mstatus.mie
   // MACH_SOFTWARE_INTERRUPT : (mip.msip and mie.msie) and mstatus.mie
   // USER_TIMER_INTERRUPT    : (mip.utip and mie.utie) and mstatus.mie
   // MACH_TIMER_INTERRUPT    : (mip.mtip and mie.mtie) and mstatus.mie
   // USER_EXTERNAL_INTERRUPT : (mip.ueip and mie.ueie) and mstatus.mie
   // MACH_EXTERNAL_INTERRUPT : (mip.meip and mie.meie) and mstatus.mie

   const auto Mie       = csr( this->get_csr( CSR_MIE ) ).MIE;
   const auto Mip       = csr( this->get_csr( CSR_MIP ) ).MIP;
   const auto Mstatus   = csr( this->get_csr( CSR_MSTATUS ) ).MSTATUS;
   const auto Privelige = this->get_prv();

   // If interrupts are enabled...
   if ( ( Privelige == PRIV_USER ) or
        ( ( Privelige == PRIV_MACHINE ) and Mstatus.MIE ) ) {
      /*
         From the priveliged spec, p30:

         Multiple simultaneous interrupts and traps at the same privilege
         level are handled in the following decreasing priority order:
         external interrupts, software interrupts, timer interrupts, then
         finally any synchronous traps.
      */

      if ( Mip.MEIP and Mie.MEIE ) {
         return ex( ex::INT_MACH_EXTERNAL_INTERRUPT );
      } else if ( Mip.MSIP and Mie.MSIE ) {
         return ex( ex::INT_MACH_SOFTWARE_INTERRUPT );
      } else if ( Mip.MTIP and Mie.MTIE ) {
         return ex( ex::INT_MACH_TIMER_INTERRUPT );
      } else if ( Mip.UEIP and Mie.UEIE ) {
         return ex( ex::INT_USER_EXTERNAL_INTERRUPT );
      } else if ( Mip.USIP and Mie.USIE ) {
         return ex( ex::INT_USER_SOFTWARE_INTERRUPT );
      } else if ( Mip.UTIP and Mie.UTIE ) {
         return ex( ex::INT_USER_TIMER_INTERRUPT );
      }
   }

   return ex::SUCCESS;
}

// ----------------------------------------------------------------------------

// Execute a number of instructions.
void processor::execute( unsigned int Num, bool Check_For_Breakpoints ) {
   while ( Num-- ) {
      execution_result Result = this->Check_For_Pending_Interrupts();

      if ( Is_Interrupt( Result ) ) {
         this->Handle_Exception( Result );
      }

      uint32_t Word = 0;

      if ( not util::Address_Is_Word_Aligned( this->PC ) ) {
         DEBUG_LOG( RED( "PC is misaligned. Not even calling Execute()." ) );
         Result = execution_result::EXC_INSTRUCTION_ADDRESS_MISALIGNED;
      } else {
         Word = Main_Memory->read_word( PC );

         if ( Check_For_Breakpoints and Breakpoint_Is_Active and
              ( PC == Breakpoint_Address ) ) {
            // @Required
            printf( "Breakpoint reached at %08x\n", PC );
            return;
         }

         instr Instruction;
         instr_id ID;
         std::tie( Instruction, ID ) = Integer_To_Instruction( Word );

         DEBUG_LOG( "pc %08x -> memory %08x -> %s",
                    PC,
                    Instruction.As_Integer,
                    Instruction_To_Assembly( Instruction.As_Integer ).c_str() );

         Result = Instruction.Execute( this, ID );
      }

      this->Handle_Exception( Result, Word );

      this->PC += 4;
      // Branch instructions and similar account for this PC+=4 by subtracting 4
      // from their target addresses. This is a bit hacky.
   }
}

// ----------------------------------------------------------------------------

// Clear breakpoint
void processor::clear_breakpoint( void ) {
   this->Breakpoint_Is_Active = false;
}

// ----------------------------------------------------------------------------

// Set breakpoint at an address
void processor::set_breakpoint( uint32_t Address ) {
   this->Breakpoint_Is_Active = true;
   this->Breakpoint_Address   = Address;
   DEBUG_LOG( "Breakpoint set to address %08x", Address );
}

// ----------------------------------------------------------------------------

void processor::show_csr( unsigned CSR_Number ) const {
   if ( not CSR_Is_Valid( CSR_Number ) ) {
      puts( "Illegal CSR number" ); // @Required
      return;
   }

   const auto Value = this->get_csr( CSR_Number );
   DEBUG_LOG( "Displaying CSR 0x%x (%08x)", CSR_Number, Value );
   printf( "%08x\n", Value );
}

// ----------------------------------------------------------------------------

void processor::set_csr( unsigned CSR_Number,
                         uint32_t New_Value,
                         bool From_Instr ) {
   assert( CSR_Number <= 0xFFF );

   assert( CSR_Is_Valid( CSR_Number ) );

   const auto Name = CSR_To_String( csr_code( CSR_Number ) );

   if ( not this->Stage2 ) {
      DEBUG_LOG( "Not setting CSR since we're in stage 1." );
      return;
   }

   if ( not CSR_Is_Writeable( CSR_Number ) ) {
      puts( "Illegal write to read-only CSR" ); // @Required
      DEBUG_LOG( "Not assigining %s -- it's hard-coded.", Name );
      return;
   }

   uint32_t To_Assign = 0xFEEDBEEF;

   switch ( CSR_Number ) {
      case INVALID_CSR_NAME: // Fall through
      case CSR_MVENDORID:
      case CSR_MIMPID:
      case CSR_MARCHID:
      case CSR_MHARTID: assert( util::Unreachable );

      case CSR_MSTATUS: {
         mstatus Given  = csr( New_Value ).MSTATUS;
         mstatus Result = csr( 0 ).MSTATUS;
         Result.MIE     = Given.MIE;
         Result.MPIE    = Given.MPIE;
         Result.MPP     = Given.MPP;
         csr Ret;
         Ret.MSTATUS = Result;
         To_Assign   = Ret;

         DEBUG_LOG( "Assigning new mstatus: %s",
                    util::Mstatus_To_String( Result ).c_str() );

         break;
      }

      case CSR_MISA: To_Assign = 0x40100100; break;

      case CSR_MIE: {
         // Machine interrupt enable register. Only the following bits are
         // implemented: usie, msie, utie, mtie, ueie, meie. Other bits are
         // fixed at 0.
         mie Given   = csr( New_Value ).MIE;
         mie Result  = csr( 0 ).MIE;
         Result.USIE = Given.USIE;
         Result.MSIE = Given.MSIE;
         Result.UTIE = Given.UTIE;
         Result.MTIE = Given.MTIE;
         Result.UEIE = Given.UEIE;
         Result.MEIE = Given.MEIE;
         csr Ret;
         Ret.MIE   = Result;
         To_Assign = Ret;
         break;
      }

      case CSR_MTVEC: {
         // Machine trap handler base address register. The MODE field can only
         // be 0 or 1, so bit 1 of mtvec is fixed at 0. If MODE is 1 (Vectored),
         // then BASE must be 128-byte aligned; in that case, mtvec bits 2 to 6
         // are fixed at 0.
         To_Assign = util::Set_Bit( New_Value, 2, false );
         if ( util::Get_Bit( To_Assign, 1 ) ) {
            DEBUG_LOG( YELLOW( "Wiping bits in MTVEC." ) );
            To_Assign = util::Set_Bits( To_Assign, 2, 7, false );
         }
         break;
      }

      case CSR_MSCRATCH: {
         To_Assign = New_Value;
         break;
      }

      case CSR_MCAUSE: {
         To_Assign = ( New_Value & 0x8000000f );
         break;
      }

      case CSR_MTVAL: {
         // Machine bad address or instruction. For misaligned address
         // exceptions, mtval is written with the address. For illegal
         // instruction exceptions, the least-significant word of mtval is
         // written with the instruction word, and the most-significant word of
         // mtval is set to 0. For other exceptions, mtval is set to 0.
         To_Assign = New_Value;
         break;
      }

      case CSR_MEPC: {
         To_Assign = util::Set_Bits( New_Value, 1, 2, false );
         break;
      }

      case CSR_MIP: {
         // Machine interrupt pending.
         mip Given   = csr( New_Value ).MIP;
         mip Result  = csr( this->get_csr( CSR_MIP ) ).MIP;
         Result.USIP = Given.USIP;
         Result.UTIP = Given.UTIP;
         Result.UEIP = Given.UEIP;

         // These are read-only from instructions. But we want people using the
         // simulator's csr command to be able to set them.
         if ( not From_Instr ) {
            Result.MSIP = Given.MSIP;
            Result.MTIP = Given.MTIP;
            Result.MEIP = Given.MEIP;
         }

         csr Ret;
         Ret.MIP   = Result;
         To_Assign = Ret;
         break;
      }

      default: assert( util::Unreachable );
   }

   DEBUG_LOG( "%s <- %08x (was given %08x).", Name, To_Assign, New_Value );
   this->CSR[CSR_To_Index( CSR_Number )] = To_Assign;
}

// ----------------------------------------------------------------------------

uint32_t processor::get_csr( unsigned CSR_Number ) const {
   assert( CSR_Is_Valid( CSR_Number ) );

   // Switch to handle hard-coded cases
   switch ( csr_code( CSR_Number ) ) {
      case CSR_MVENDORID: return 0;
      case CSR_MARCHID: return 0;
      case CSR_MIMPID: return 0x20190200;
      case CSR_MHARTID: return 0;
      case CSR_MISA: return 0x40100100;
      default: return this->CSR[CSR_To_Index( CSR_Number )];
   }
}

// ----------------------------------------------------------------------------

unsigned processor::get_prv( void ) const {
   return this->Privelige_Level;
}

// ----------------------------------------------------------------------------

void processor::set_prv( unsigned Privelige_Level ) {
   assert( Privelige_Level == 0 or Privelige_Level == 3 );
   DEBUG_LOG( "Setting privelige to %u.", Privelige_Level );
   // TODO: Delete this if not needed.
   //
   // Check whether set_csr() calls set_prv() before uncommenting this. I had
   // infinite recursion once.

   // csr CSR = this->get_csr( CSR_MSTATUS );
   // CSR.MSTATUS.MPP = Privelige_Level;
   // this->set_csr( CSR_MSTATUS, CSR );
   this->Privelige_Level = privelige_level( Privelige_Level );
}

// ----------------------------------------------------------------------------

void processor::show_prv( void ) const {
   const auto Priv = this->get_prv();
   printf( "%u %s\n",
           Priv,
           Priv == PRIV_MACHINE
             ? "(machine)"
             : Priv == PRIV_USER ? "(user)" : "(Invalid privelige!!)" );
}

// ----------------------------------------------------------------------------

void processor::Handle_Exception( execution_result Result,
                                  uint32_t Instruction ) {
   using ex = execution_result;

   const bool Exception_Occurred = Is_Exception( Result );
   const bool Interrupt_Occurred = Is_Interrupt( Result );

   if ( not Exception_Occurred and not Interrupt_Occurred ) {
      this->Executed_Instruction_Count += 1;
      return;
   }

   DEBUG_LOG( YELLOW( "trapped: %s" ), Exec_Result_To_String( Result ) );

   // Update Mstatus privelige stack
   {
      csr Mstatus          = this->get_csr( CSR_MSTATUS );
      Mstatus.MSTATUS.MPIE = Mstatus.MSTATUS.MIE;
      Mstatus.MSTATUS.MIE  = 0;
      Mstatus.MSTATUS.MPP  = ( this->Privelige_Level );
      this->set_csr( CSR_MSTATUS, Mstatus );

      if ( this->get_prv() == PRIV_USER ) {
         DEBUG_LOG( "Elevating privelige from USER to MACHINE." );
         this->set_prv( PRIV_MACHINE );
      }
   }

   // Update Mcause and MEPC
   this->set_csr( CSR_MCAUSE, Result );
   this->set_csr( CSR_MEPC, this->PC );

   // Handle exceptions
   if ( Exception_Occurred ) {
      switch ( Result ) {
         case ex::EXC_ILLEGAL_INSTRUCTION: {
            const auto Instr =
              this->Main_Memory->read_word_unaligned( this->PC );

            DEBUG_LOG( "Illegal instruction. Setting MTVAL <- %08x", Instr );
            this->set_csr( CSR_MTVAL, Instr );
            break;
         }
         case ex::EXC_LOAD_ADDRESS_MISALIGNED: {
            const auto Instr = instr( Instruction );
            const auto Base  = this->get_reg( Instr.I_Type.RS1 );
            const auto Imm =
              util::Sign_Extend( Instr.I_Type.Immediate_11_To_0, 12 );
            const auto Bad_Address = ( Imm + Base );

            DEBUG_LOG( "Misaligned load. Setting MTVAL <- %08x", Bad_Address );
            this->set_csr( CSR_MTVAL, Bad_Address );
            break;
         }
         case ex::EXC_STORE_ADDRESS_MISALIGNED: {
            const auto Instr = instr( Instruction );
            const auto Imm =
              util::Sign_Extend( Instr.S_Type.Decipher_Immediate(), 12 );
            const auto Base        = this->get_reg( Instr.S_Type.RS1 );
            const auto Bad_Address = ( Base + Imm );

            DEBUG_LOG( "Misaligned store. Setting MTVAL <- %08x", Bad_Address );
            this->set_csr( CSR_MTVAL, Bad_Address );
            break;
         }
         case ex::EXC_INSTRUCTION_ADDRESS_MISALIGNED: {
            DEBUG_LOG( "Misaligned instruction. Setting MTVAL <- %08x", PC );
            this->set_csr( CSR_MTVAL, this->PC );
            break;
         }
         default: break;
      }
   }

   // Jump to exception handler. This should be the last thing we do.
   // FIXME: What if the exception handler is 0?

   const auto Mtvec     = this->get_csr( CSR_MTVEC );
   auto Handler_Address = ( Mtvec & ~0b11 );
   auto Mode            = ( Mtvec & 0b11 );
   enum { MTVEC_MODE_DIRECT = 0b00, MTVEC_MODE_VECTORED = 0b01 };

   if ( Is_Exception( Result ) ) {
      DEBUG_LOG( "Ignoring MTVEC mode since we got here from an exception." );
      DEBUG_LOG( "Jumping to %08x.", Handler_Address - 4 );
      this->set_pc( Handler_Address - 4 );
      return;
   }

   if ( Mode == 0b11 ) {
      DEBUG_LOG(
        RED( "MTVEC mode is RESERVED. This is invalid! Doing nothing." ) );
      return;
   }

   if ( Mode == MTVEC_MODE_VECTORED ) {
      Handler_Address = util::Set_Bits( Handler_Address, 2, 7, false );
   }

   switch ( Mode ) {
      case MTVEC_MODE_DIRECT: {
         DEBUG_LOG( "MTVEC mode is DIRECT. Jumping to %08x.", Handler_Address );
         this->set_pc( Handler_Address );
         break;
      }
      case MTVEC_MODE_VECTORED: {
         const uint32_t Target = ( Handler_Address + ( 4 * Result ) );
         DEBUG_LOG( "MTVEC mode is VECTORED. Jumping to %08x + %08x = %08x.",
                    Handler_Address,
                    ( 4 * Result ),
                    Target );
         this->set_pc( Target );
         break;
      }
   }
}
