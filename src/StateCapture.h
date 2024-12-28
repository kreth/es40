#ifndef STATECAPTURE_H
#define STATECAPTURE_H

#include "AlphaCPUState.h" // For SCPU_state
#include "cpu_defs.h" // For instruction related definitions

#include <vector>

struct StateCapture {
      enum class Type {
         Instruction,
         Exception
      };
      Type type;
      SCPU_state state;   // Complete CPU state at the start of the cycle

      // Instruction related
      u32 ins;
      int opcode;

      struct MemoryAccess {
            enum class AccessType { Read, Write, Lock } type;
            u64 address;
            u64 data;
            u32 size;
            u32 cpuid;
      };
      std::vector<MemoryAccess> memory_accesses;

      struct IPRCall {
            enum class MethodType {
               MFPR,
               MTPR
            };
            MethodType type;
            int function;
            u64 argument;
            u64 return_val;
      };
      std::vector<IPRCall> ipr_calls;

      struct FloatingPointCall {
            enum class MethodType {
               ieee_lds,
               ieee_sts,
               ieee_cvtst,
               ieee_cvtts,
               ieee_fcmp,
               ieee_cvtif,
               ieee_cvtfi,
               ieee_fadd,
               ieee_fmul,
               ieee_fdiv,
               ieee_sqrt,
               ieee_unpack,
               ieee_norm,
               ieee_rpack,
               ieee_trap,
               vax_ldf,
               vax_ldg,
               vax_stf,
               vax_stg,
               vax_trap,
               vax_unpack,
               vax_unpack_d,
               vax_norm,
               vax_rpack,
               vax_rpack_d,
               vax_fcmp,
               vax_cvtif,
               vax_cvtfi,
               vax_fadd,
               vax_fmul,
               vax_fdiv,
               vax_sqrt
            };
            MethodType type;
            std::vector<u64> arguments;
            u64 return_val;
      };
      std::vector<FloatingPointCall> fp_calls;

      struct ExternalCall {
            enum class MethodType {
               ReadMem,
               WriteMem,
               PtrToMem,
               cpu_lock,
               interrupt,
               tbia,
               tbiap,
               tbis
            };
            MethodType type;
            u64 address;
            u32 size;
            u64 value;
            int number;
            bool assert;
      };
      std::vector<ExternalCall> external_calls;


      struct TBChange{
            enum class MethodType {
               add_tb,
               tbia,
               tbiap,
               tbis
            };
            MethodType type;
            u64 virt;
            u64 pte_phys;
            u64 pte_flags;
            int flags;

      };
      std::vector<TBChange> tb_changes;


    
      bool  operator == ( const StateCapture& other ) const {
         if (type != other.type) return false;
         if (memcmp(&state, &other.state, sizeof(state)) != 0)
            return false;
         if(ins != other.ins) return false;
         if(opcode != other.opcode) return false;
         if (memory_accesses.size() != other.memory_accesses.size()) return false;
         for(int ii=0; ii < memory_accesses.size(); ii++) {
            if (memory_accesses[ii].type != other.memory_accesses[ii].type) return false;
            if (memory_accesses[ii].address != other.memory_accesses[ii].address) return false;
            if (memory_accesses[ii].data != other.memory_accesses[ii].data) return false;
            if(memory_accesses[ii].size != other.memory_accesses[ii].size) return false;
            if(memory_accesses[ii].cpuid != other.memory_accesses[ii].cpuid) return false;

         }
         if (ipr_calls.size() != other.ipr_calls.size()) return false;
         for(int ii=0; ii < ipr_calls.size(); ii++) {
            if(ipr_calls[ii].type != other.ipr_calls[ii].type) return false;
            if(ipr_calls[ii].function != other.ipr_calls[ii].function) return false;
            if(ipr_calls[ii].argument != other.ipr_calls[ii].argument) return false;
            if(ipr_calls[ii].return_val != other.ipr_calls[ii].return_val) return false;
         }
         if (fp_calls.size() != other.fp_calls.size()) return false;
         for(int ii=0; ii < fp_calls.size(); ii++) {
            if(fp_calls[ii].type != other.fp_calls[ii].type) return false;
            if(fp_calls[ii].arguments.size() != other.fp_calls[ii].arguments.size()) return false;
            for(int kk=0; kk < fp_calls[ii].arguments.size(); kk++)
               if(fp_calls[ii].arguments[kk] != other.fp_calls[ii].arguments[kk]) return false;
            if(fp_calls[ii].return_val != other.fp_calls[ii].return_val) return false;
         }
         if (external_calls.size() != other.external_calls.size()) return false;
         for(int ii=0; ii < external_calls.size(); ii++) {
            if(external_calls[ii].type != other.external_calls[ii].type) return false;
            if(external_calls[ii].address != other.external_calls[ii].address) return false;
            if(external_calls[ii].size != other.external_calls[ii].size) return false;
            if(external_calls[ii].value != other.external_calls[ii].value) return false;
            if(external_calls[ii].number != other.external_calls[ii].number) return false;
            if(external_calls[ii].assert != other.external_calls[ii].assert) return false;
         }
         if (tb_changes.size() != other.tb_changes.size()) return false;
         for(int ii=0; ii < tb_changes.size(); ii++) {
            if(tb_changes[ii].type != other.tb_changes[ii].type) return false;
            if(tb_changes[ii].virt != other.tb_changes[ii].virt) return false;
            if(tb_changes[ii].pte_phys != other.tb_changes[ii].pte_phys) return false;
            if(tb_changes[ii].pte_flags != other.tb_changes[ii].pte_flags) return false;
            if(tb_changes[ii].flags != other.tb_changes[ii].flags) return false;
         }
         return true;
      }
};
#endif
