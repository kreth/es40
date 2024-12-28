
#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include "StdAfx.h"

class CTraceEngine;
class CSystem;
class CAlphaCPU;
class SCPU_state;

class DebugOutput
{
   public:
      DebugOutput(CTraceEngine* trc, CSystem * system);
      virtual ~DebugOutput();
#if defined(IDB)      
      void generate_debug_output(CAlphaCPU* cpu, SCPU_state& state, u32 ins, char** dbg_strptrP, bool bListing, char* funcname);
#endif
   private:
      CTraceEngine* trc;
      CSystem* cSystem;
};
#endif // DEBUG_OUTPUT_H

