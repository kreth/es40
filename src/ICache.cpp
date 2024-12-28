#include "ICache.h"
#include "System.h"
#include "AlphaCPU.h"
#include "cpu_defs.h"
#include <string.h>

ICache::ICache(CConfigurator * cfg, CSystem* system) : myCfg(cfg), cSystem(system)
{
   next_icache = 0;
   last_found_icache = 0;
   icache_enabled = true;
   for(int i=0;i < ICACHE_ENTRIES; i++) {
      icache[i].valid = false;
   }
   icache_enabled = myCfg->get_bool_value("icache", false);
}

ICache::~ICache()
{
}

/**
 * Get an instruction from the instruction cache.
 * If necessary, fill a new cache block from memory.
 *
 * get_icache checks all cache entries, to see if there is a
 * cache entry that matches the current address space number,
 * and that contains the address we're looking for. If it
 * exists, the instruction is fetched from this cache,
 * otherwise, the physical address for the instruction is
 * calculated, and the cache block is filled.
 *
 * The last cache entry that was a hit is remembered, so that
 * cache entry is checked first on the next instruction. (very
 * likely to be the same cache block)
 *
 * It would be easiest to do without the instruction cache
 * altogether, but unfortunately SRM uses self-modifying
 * code, that relies on the correct instruction stream to
 * remain in the cache.
 **/
int ICache::get_icache(u64 address, u32* data, SCPU_state &state, CSystemComponent* device)
{
   int   i = last_found_icache;
   u64   v_a;
   u64   p_a;
   int   result;
   bool  asm_bit;

   if(icache_enabled)
   {
      if(icache[i].valid
         && (icache[i].asn == state.asn || icache[i].asm_bit)
         && icache[i].address == (address & ICACHE_MATCH_MASK))
      {
         *data = endian_32(icache[i].data[(address >> 2) & ICACHE_INDEX_MASK]);
         return 0;
      }

      for(i = 0; i < ICACHE_ENTRIES; i++)
      {
         if(icache[i].valid
            && (icache[i].asn == state.asn || icache[i].asm_bit)
            && icache[i].address == (address & ICACHE_MATCH_MASK))
         {
            last_found_icache = i;
            *data = endian_32(icache[i].data[(address >> 2) & ICACHE_INDEX_MASK]);
            return 0;
         }
      }

      v_a = address & ICACHE_MATCH_MASK;

      if(address & 1)
      {
         p_a = v_a &~U64(0x1);
         asm_bit = true;
      }
      else
      {
         result = ((CAlphaCPU*)device)->virt2phys(v_a, &p_a, ACCESS_EXEC, &asm_bit, 0);
         if(result)
            return result;
      }

      memcpy(icache[next_icache].data, cSystem->PtrToMem(p_a),
             ICACHE_LINE_SIZE * 4);

      icache[next_icache].valid = true;
      icache[next_icache].asn = state.asn;
      icache[next_icache].asm_bit = asm_bit;
      icache[next_icache].address = address & ICACHE_MATCH_MASK;
      icache[next_icache].p_address = p_a;

      *data = endian_32(icache[next_icache].data[(address >> 2) & ICACHE_INDEX_MASK]);

      last_found_icache = next_icache;
      next_icache++;
      if(next_icache == ICACHE_ENTRIES)
         next_icache = 0;
      return 0;
   }

   // icache disabled
   if(address & 1)
   {
      state.pc_phys = address &~U64(0x3);
      state.rem_ins_in_page = 1;
   }
   else
   {
      if(!state.rem_ins_in_page)
      {
         result = ((CAlphaCPU*)device)->virt2phys(address, &state.pc_phys, ACCESS_EXEC, &asm_bit, 0);
         if(result)
            return result;
         state.rem_ins_in_page = 2048 - ((((u32) address) >> 2) & 2047);
      }
   }

   *data = (u32) cSystem->ReadMem(state.pc_phys, 32, device);
   return 0;
}

/**
 * Empty the instruction cache.
 **/
void ICache::flush_icache(SCPU_state &state)
{
   if(icache_enabled)
   {

      //  memset(state.icache,0,sizeof(state.icache));
      int i;
      for(i = 0; i < ICACHE_ENTRIES; i++)
      {
         icache[i].valid = false;

         //    state.icache[i].asm_bit = true;
      }

      next_icache = 0;
      last_found_icache = 0;
   }
}

/**
 * Empty the instruction cache of lines with the ASM bit clear.
 **/
void ICache::flush_icache_asm(SCPU_state &state)
{
   if(icache_enabled)
   {
      int i;
      for(i = 0; i < ICACHE_ENTRIES; i++)
         if(!icache[i].asm_bit)
            icache[i].valid = false;
   }
}


/**
 * \brief Enable i-cache regardles of config file.
 *
 * Required for SRM-ROM decompression.
 **/
void ICache::enable_icache(SCPU_state &state)
{
   icache_enabled = true;
}

/**
 * \brief Enable or disable i-cache depending on config file.
 **/
void ICache::restore_icache(SCPU_state &state)
{
   bool  newval;

   newval = myCfg->get_bool_value("icache", false);

   if(!newval)
      flush_icache(state);

   icache_enabled = newval;
}
