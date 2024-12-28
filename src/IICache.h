/**
 * This class defines the abstract interface for the Instruction Cache
 **/      
#ifndef IICACHE_H
#define IICACHE_H

#include "StdAfx.h"

class CSystemComponent;
class SCPU_state;

class IICache
{
   public:
      virtual ~IICache() {}
      virtual int get_icache(u64 address, u32* data, SCPU_state &state, CSystemComponent* device) = 0;
      virtual void flush_icache(SCPU_state &state) = 0;
      virtual void flush_icache_asm(SCPU_state &state) = 0;
      virtual void enable_icache(SCPU_state &state) = 0;
      virtual void restore_icache(SCPU_state &state) = 0;

};

#endif // IICACHE_H
