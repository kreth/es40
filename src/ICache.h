#ifndef ICACHE_H
#define ICACHE_H

#include "IICache.h"
#include "StdAfx.h"
#include "cpu_defs.h"

// Number of entries in the Instruction Cache
#define ICACHE_ENTRIES    1024
// Size of Instruction Cache entries in DWORDS (instructions)
#define ICACHE_LINE_SIZE  512
/** These bits should match to have an Instruction Cache hit.
    This includes bit 0, because it indicates PALmode . */
#define ICACHE_MATCH_MASK (u64) (U64(0x1) - (ICACHE_LINE_SIZE * 4))
/// DWORD (instruction) number of an address in an ICache entry.
#define ICACHE_INDEX_MASK (u64) (ICACHE_LINE_SIZE - U64(0x1))
/// Byte numer of an address in an ICache entry.
#define ICACHE_BYTE_MASK  (u64) (ICACHE_INDEX_MASK << 2)

#include "IICache.h"
class CAlphaCPU;
class CConfigurator;

class ICache : public IICache
{
   public:
      ICache(CConfigurator * cfg, CAlphaCPU* cpu);
      virtual ~ICache();
      int get_icache(u64 address, u32* data, SCPU_state &state, CSystem* system) override;
      void flush_icache() override;
      void flush_icache_asm() override;
      void enable_icache() override;
      void restore_icache() override;

   private:
      /**
       * \brief Instruction cache entry.
       *
       * An instruction cache entry contains the address and address space number
       * (ASN) + 16 32-bit instructions. [HRM 2-11]
       **/
      struct SICache
      {
            int   asn;          /**< Address Space Number */
            u32   data[ICACHE_LINE_SIZE]; /**< Actual cached instructions */
            u64   address;      /**< Address of first instruction */
            u64   p_address;    /**< Physical address of first instruction */
            bool  asm_bit;      /**< Address Space Match bit */
            bool  valid;        /**< Valid cache entry */
      } icache[ICACHE_ENTRIES]; /**< Instruction cache entries [HRM p 2-11] */
      int next_icache;          /**< Number of next cache entry to use */
      int last_found_icache;    /**< Number of last cache entry found */
      bool  icache_enabled;
      CConfigurator* myCfg;
      CAlphaCPU* cCpu;
};
#endif
