#include "DebugOutput.h"
#include "AlphaCPU.h"
#include "System.h"
#include "cpu_debug.h"
#include "TraceEngine.h"
#include <string.h>

DebugOutput::DebugOutput(CTraceEngine* trc, CSystem * system) : trc(trc), cSystem(system) {}

DebugOutput::~DebugOutput()
{}

#if defined(IDB)
void DebugOutput::generate_debug_output(CAlphaCPU* this_ptr, SCPU_state& state, u32 ins, char**  dbg_strptrP, bool bListing, char* funcname)
{
   
   char* dbg_strptr = *dbg_strptrP;
   char*   tmp_funcname = 0;
   u64     tmp_pc = state.current_pc;

   if(trc->get_fnc_name(this_ptr, tmp_pc &~U64(0x3), &tmp_funcname))
   {
      if(bListing && !strcmp(tmp_funcname, ""))
      {
         printf("%08" PRIx64 ": \"%s\"\n", tmp_pc,
                cSystem->PtrToMem(tmp_pc));
         u64 tmp_pc_2 = (
                         tmp_pc +
                         strlen(cSystem->PtrToMem(tmp_pc)) +
                         4
                         ) &~U64(0x3);
         while(tmp_pc_2 < 0x600000 && cSystem->ReadMem(tmp_pc_2, 32, this_ptr) == 0)
            tmp_pc_2 += 4;
         state.pc = tmp_pc_2;
         return;
      }
      else if(bListing && !strcmp(tmp_funcname, "!SKIP"))
      {
         while(state.pc < 0x600000 && cSystem->ReadMem(state.pc, 32, this_ptr) == 0)
            state.pc += 4;
         return;
      }
      else if(bListing && !strncmp(tmp_funcname, "!CHAR-", 6))
      {
         u64 xx_upto;
         int xx_result;
         xx_result = sscanf(&(tmp_funcname[6]), "%" PRIx64 "", &xx_upto);
         if(xx_result == 1)
         {
            u64 start_pc = tmp_pc;
            while(start_pc < xx_upto)
            {
               printf("%08" PRIx64 ": \"%s\"\n", start_pc, cSystem->PtrToMem(start_pc));
               start_pc += strlen(cSystem->PtrToMem(start_pc));
               while(start_pc < xx_upto && cSystem->ReadMem(start_pc, 8, this_ptr) == 0)
                  start_pc++;
            }
            return;
         }
      }
      else if(bListing && !strncmp(tmp_funcname, "!LCHAR-", 7))
      {
         char  stringval[300];
         int   stringlen;
         u64   xx_upto;
         int   xx_result;
         xx_result = sscanf(&(tmp_funcname[7]), "%" PRIx64 "", &xx_upto);
         if(xx_result == 1)
         {
            u64 start_pc = tmp_pc;
            while(start_pc < xx_upto)
            {
               stringlen = (int) cSystem->ReadMem(start_pc++, 8, this_ptr);
               memset(stringval, 0, 300);
               strncpy(stringval, cSystem->PtrToMem(start_pc), stringlen);
               printf("%08" PRIx64 ": \"%s\"\n", start_pc - 1, stringval);
               start_pc += stringlen;
               while(start_pc < xx_upto && cSystem->ReadMem(start_pc, 8, this_ptr) == 0)
                  start_pc++;
            }
            return;
         }
      }
      else if(bListing && !strncmp(tmp_funcname, "!X64-", 5))
      {
         printf("\n%s:\n", &(tmp_funcname[5]));
         u64 start_pc = tmp_pc;
         while
            (
             (start_pc == tmp_pc)
             || !trc->get_fnc_name(this_ptr, start_pc, &tmp_funcname)
             )
         {
            printf("%08" PRIx64 ": %016" PRIx64 "\n", start_pc,
                   cSystem->ReadMem(start_pc, 64, this_ptr));
            start_pc += 8;
         }
         return;
      }
      else if(bListing && !strncmp(tmp_funcname, "!X32-", 5))
      {
         printf("\n%s:\n", &(tmp_funcname[5]));
         u64 start_pc = tmp_pc;
         while
            (
             (start_pc == tmp_pc)
             || !trc->get_fnc_name(this_ptr, start_pc, &tmp_funcname)
             )
         {
            printf("%08" PRIx64 ": %08" PRIx64 "\n", start_pc,
                   cSystem->ReadMem(start_pc, 32, this_ptr));
            start_pc += 4;
         }
         return;
      }
      else if(!strncmp(tmp_funcname, ":", 1))
      {
         sprintf(dbg_strptr, "%s:\n", tmp_funcname);
         dbg_strptr += strlen(dbg_strptr);
      }
      else
      {
         sprintf(dbg_strptr, "\n%s:\n", tmp_funcname);
         dbg_strptr += strlen(dbg_strptr);
      }
   }
   sprintf(dbg_strptr, bListing ? "%08" PRIx64 ": " : "%016" PRIx64 "", tmp_pc);
   dbg_strptr += strlen(dbg_strptr);
   if(!bListing)
      sprintf(dbg_strptr, "(%08x): ", ins);
   else
   {
      sprintf(dbg_strptr, "%08x %c%c%c%c: ", ins, printable((char) (ins)),
              printable((char) (ins >> 8)), printable((char) (ins >> 16)),
              printable((char) (ins >> 24)));
   }
   dbg_strptr += strlen(dbg_strptr);
   *dbg_strptrP = dbg_strptr;
}
#endif // defined(IDB)
