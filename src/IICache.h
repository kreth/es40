/**
 * This class defines the abstract interface for the Instruction Cache
 **/      
#ifndef IICACHE_H
#define IICACHE_H

#include "StdAfx.h"

class CSystem;
class SCPU_state;

class IICache
{
   public:
      virtual ~IICache() {}
      virtual int get_icache(u64 address, u32* data, SCPU_state &state, CSystem* system) = 0;
      virtual void flush_icache() = 0;
      virtual void flush_icache_asm() = 0;
      virtual void enable_icache() = 0;
      virtual void restore_icache() = 0;
};

#endif // IICACHE_H
