#include "TranslationBuffer.h"
#include <string.h>
#include "cpu_defs.h"
#include "AlphaCPU.h"
#include "SystemComponent.h"

TranslationBuffer::TranslationBuffer(CAlphaCPU* cpu)
   : cCpu(cpu)
{
   next_tb[0] = 0;
   next_tb[1] = 0;
   last_found_tb[0][0] = 0;
   last_found_tb[0][1] = 0;
   last_found_tb[1][0] = 0;
   last_found_tb[1][1] = 0;
   for (int i = 0; i < TB_ENTRIES; i++)
   {
      tb[0][i].valid = false;
      tb[1][i].valid = false;
   }
}
    
TranslationBuffer::~TranslationBuffer()
{
}

void TranslationBuffer::set_pc(u64 p_pc) {
   cCpu->set_pc(p_pc);
}

int TranslationBuffer::vmspal_ent_dtbm_double_3(int flags) {
   cCpu->vmspal_ent_dtbm_double_3(flags);
}

int TranslationBuffer::vmspal_ent_dtbm_single(int flags) {
   cCpu->vmspal_ent_dtbm_single(flags);
}

int TranslationBuffer::vmspal_ent_itbm(int flags) {
   cCpu->vmspal_ent_itbm(flags);
}

int TranslationBuffer::vmspal_ent_iacv(int flags) {
   cCpu->vmspal_ent_iacv(flags);
}

int TranslationBuffer::vmspal_ent_dfault(int flags) {
   cCpu->vmspal_ent_dfault(flags);
}

void TranslationBuffer::set_exc_sum(u64 val) {
   cCpu->set_exc_sum(val);
}

void TranslationBuffer::set_exc_addr(u64 val) {
   cCpu->set_exc_addr(val);
}

void TranslationBuffer::set_mm_stat(u64 val) {
   cCpu->set_mm_stat(val);
}

void TranslationBuffer::set_fault_va(u64 val) {
   cCpu->set_fault_va(val);
}

/**
 * \brief Find translation-buffer entry
 *
 * Try to find a translation-buffer entry that maps the page inside which
 * the specified virtual address lies.
 *
 * \param virt    Virtual address to find in translation buffer.
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 * \return        Number of matching entry, or -1 if no match found.
 **/
int TranslationBuffer::FindTBEntry(u64 virt, int flags, const SCPU_state &state)
{
   
   // Use ITB (tb[1]) if ACCESS_EXEC is set, otherwise use DTB (tb[0])
   int t = (flags & ACCESS_EXEC) ? 1 : 0;
   int asn = (flags & ACCESS_EXEC) ? state.asn : state.asn0;
   int rw = (flags & ACCESS_WRITE) ? 1 : 0;

   // Try last match first; this is a good quess, especially in the ITB
   int i = last_found_tb[t][rw];
   if(tb[t][i].valid
      && !((tb[t][i].virt ^ virt) & tb[t][i].match_mask)
      && (tb[t][i].asm_bit || (tb[t][i].asn == asn))) return i;

   // Otherwise, loop through the TB entries to find a match.
   for(i = 0; i < TB_ENTRIES; i++)
   {
      if(tb[t][i].valid
         && !((tb[t][i].virt ^ virt) & tb[t][i].match_mask)
         && (tb[t][i].asm_bit || (tb[t][i].asn == asn)))
      {
         last_found_tb[t][rw] = i;
         return i;
      }
   }

   return -1;
}

#define GH_0_MATCH  U64(0x000007ffffffe000) /* <42:13> */
#define GH_0_PHYS   U64(0x00000fffffffe000) /* <43:13> */
#define GH_0_KEEP   U64(0x0000000000001fff) /* <12:0>  */

#define GH_1_MATCH  U64(0x000007ffffff0000)
#define GH_1_PHYS   U64(0x00000fffffff0000)
#define GH_1_KEEP   U64(0x000000000000ffff)
#define GH_2_MATCH  U64(0x000007fffff80000)
#define GH_2_PHYS   U64(0x00000ffffff80000)
#define GH_2_KEEP   U64(0x000000000007ffff)
#define GH_3_MATCH  U64(0x000007ffffc00000)
#define GH_3_PHYS   U64(0x00000fffffc00000)
#define GH_3_KEEP   U64(0x00000000003fffff)

/**
 * \brief Add translation-buffer entry
 *
 * Add a translation-buffer entry to one of the translation buffers.
 *
 * \param virt    Virtual address.
 * \param pte     Translation in DTB_PTE format (see add_tb_d).
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/

void TranslationBuffer::add_tb(u64 virt, u64 pte_phys, u64 pte_flags, int flags,
                               const SCPU_state &state)
{
   int t = (flags & ACCESS_EXEC) ? 1 : 0;
   int rw = (flags & ACCESS_WRITE) ? 1 : 0;
   u64 match_mask = 0;
   u64 keep_mask = 0;
   u64 phys_mask = 0;
   int i;
   int asn = (flags & ACCESS_EXEC) ? state.asn : state.asn0;

   switch(pte_flags & 0x60)  // granularity hint
   {
   case 0:
      match_mask = GH_0_MATCH;
      phys_mask = GH_0_PHYS;
      keep_mask = GH_0_KEEP;
      break;

   case 0x20:
      match_mask = GH_1_MATCH;
      phys_mask = GH_1_PHYS;
      keep_mask = GH_1_KEEP;
      break;

   case 0x40:
      match_mask = GH_2_MATCH;
      phys_mask = GH_2_PHYS;
      keep_mask = GH_2_KEEP;
      break;

   case 0x60:
      match_mask = GH_3_MATCH;
      phys_mask = GH_3_PHYS;
      keep_mask = GH_3_KEEP;
      break;
   }

   i = FindTBEntry(virt, flags, state);

   if(i < 0)
   {
      i = next_tb[t];
      next_tb[t]++;
      if(next_tb[t] == TB_ENTRIES)
         next_tb[t] = 0;
   }

   tb[t][i].match_mask = match_mask;
   tb[t][i].keep_mask = keep_mask;
   tb[t][i].virt = virt & match_mask;
   tb[t][i].phys = pte_phys & phys_mask;
   tb[t][i].fault[0] = (int) pte_flags & 2;
   tb[t][i].fault[1] = (int) pte_flags & 4;
   tb[t][i].fault[2] = (int) pte_flags & 8;
   tb[t][i].access[0][0] = (int) pte_flags & 0x100;
   tb[t][i].access[1][0] = (int) pte_flags & 0x1000;
   tb[t][i].access[0][1] = (int) pte_flags & 0x200;
   tb[t][i].access[1][1] = (int) pte_flags & 0x2000;
   tb[t][i].access[0][2] = (int) pte_flags & 0x400;
   tb[t][i].access[1][2] = (int) pte_flags & 0x4000;
   tb[t][i].access[0][3] = (int) pte_flags & 0x800;
   tb[t][i].access[1][3] = (int) pte_flags & 0x8000;
   tb[t][i].asm_bit = (int) pte_flags & 0x10;
   tb[t][i].asn = asn;
   tb[t][i].valid = true;
   last_found_tb[t][rw] = i;

#if defined(DEBUG_TB_)
#if defined(IDB)
   if(bTB_Debug)
#endif
   {
      printf("Add TB---------------------------------------\n");
      printf("Map VIRT    %016" PRIx64 "\n", tb[i].virt);
      printf("Matching    %016" PRIx64 "\n", tb[i].match_mask);
      printf("And keeping %016" PRIx64 "\n", tb[i].keep_mask);
      printf("To PHYS     %016" PRIx64 "\n", tb[i].phys);
      printf("Read : %c%c%c%c %c\n", tb[i].access[0][0] ? 'K' : '-',
             tb[i].access[0][1] ? 'E' : '-',
             tb[i].access[0][2] ? 'S' : '-',
             tb[i].access[0][3] ? 'U' : '-', tb[i].fault[0] ? 'F' : '-');
      printf("Write: %c%c%c%c %c\n", tb[i].access[1][0] ? 'K' : '-',
             tb[i].access[1][1] ? 'E' : '-',
             tb[i].access[1][2] ? 'S' : '-',
             tb[i].access[1][3] ? 'U' : '-', tb[i].fault[1] ? 'F' : '-');
      printf("Exec : %c%c%c%c %c\n", tb[i].access[1][0] ? 'K' : '-',
             tb[i].access[1][1] ? 'E' : '-',
             tb[i].access[1][2] ? 'S' : '-',
             tb[i].access[1][3] ? 'U' : '-', tb[i].fault[1] ? 'F' : '-');
   }
#endif
}

/**
 * \brief Add translation-buffer entry to the DTB
 *
 * The format of the PTE field is:
 * \code
 *   63 62           32 31     16  15  14  13  12  11  10  9   8  7 6  5  4  3  2   1  0
 *  +--+---------------+---------+---+---+---+---+---+---+---+---+-+----+---+-+---+---+-+
 *  |  |  PA <43:13>   |         |UWE|SWE|EWE|KWE|URE|SRE|ERE|KRE| | GH |ASM| |FOW|FOR| |
 *  +--+---------------+---------+---+---+---+---+---+---+---+---+-+----+---+-+---+---+-+
 *                               +-------------------------------+    |   |   +-------+
 *                                                           |        |   |       |
 *  (user,supervisor,executive,kernel)(read,write)enable ----+        |   |       |
 *                                              granularity hint -----+   |       |
 *                                               address space match -----+       |
 *                                                      fault-on-(read,write) ----+
 * \endcode
 *
 * \param virt    Virtual address.
 * \param pte     Translation in DTB_PTE format.
 **/
void TranslationBuffer::add_tb_d(u64 virt, u64 pte, const SCPU_state &state)
{
   add_tb(virt, pte >> (32 - 13), pte, ACCESS_READ, state);
}

/**
 * \brief Add translation-buffer entry to the ITB
 *
 * The format of the PTE field is:
 * \code
 *   63              44 43           13 12  11  10  9   8  7 6  5  4  3   0
 *  +------------------+---------------+--+---+---+---+---+-+----+---+-----+
 *  |                  |  PA <43:13>   |  |URE|SRE|ERE|KRE| | GH |ASM|     |
 *  +------------------+---------------+--+---+---+---+---+-+----+---+-----+
 *                                        +---------------+    |   |   
 *                                                    |        |   |       
 *  (user,supervisor,executive,kernel)read enable ----+        |   |       
 *                                       granularity hint -----+   |       
 *                                        address space match -----+       
 *
 * \endcode
 *
 * \param virt    Virtual address.
 * \param pte     Translation in ITB_PTE format.
 **/
void TranslationBuffer::add_tb_i(u64 virt, u64 pte, const SCPU_state &state)
{
   add_tb(virt, pte, pte & 0xf70, ACCESS_EXEC, state);
}

/**
 * \brief Invalidate all translation-buffer entries
 *
 * Invalidate all translation-buffer entries in one of the translation buffers.
 *
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/
void TranslationBuffer::tbia(int flags, const SCPU_state &state)
{
   int t = (flags & ACCESS_EXEC) ? 1 : 0;
   int i;
   for(i = 0; i < TB_ENTRIES; i++)
      tb[t][i].valid = false;
   last_found_tb[t][0] = 0;
   last_found_tb[t][1] = 0;
   next_tb[t] = 0;
}

/**
 * \brief Invalidate all process-specific translation-buffer entries
 *
 * Invalidate all translation-buffer entries that do not have the ASM bit
 * set in one of the translation buffers.
 *
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/
void TranslationBuffer::tbiap(int flags, const SCPU_state &state)
{
   int t = (flags & ACCESS_EXEC) ? 1 : 0;
   int i;
   for(i = 0; i < TB_ENTRIES; i++)
      if(!tb[t][i].asm_bit)
         tb[t][i].valid = false;
}
    
/**
 * \brief Invalidate single translation-buffer entry
 *
 * \param virt    Virtual address for which the entry should be invalidated.
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/
void TranslationBuffer::tbis(u64 virt, int flags, const SCPU_state &state)
{
   int t = (flags & ACCESS_EXEC) ? 1 : 0;
   int i = FindTBEntry(virt, flags, state);
   if(i >= 0)
      tb[t][i].valid = false;
}

/**
 * \brief Translate a virtual address to a physical address.
 *
 * Translate a 64-bit virtual address into a 64-bit physical address, using
 * the page table buffers.
 *
 * The following steps are taken to resolve the address:
 *  - See if the address can be found in the translation buffer.
 *  - If not, try to load the right page table entry into the translation
 *    buffer, if this is not possible, trap to the OS.
 *  - Check access privileges.
 *  - Check fault bits.
 *  .
 *
 * \param virt    Virtual address to be translated.
 * \param phys    Pointer to where the physical address is to be returned.
 * \param flags   Set of flags that determine the exact functioning of the
 *                function. A combination of the following flags:
 *                  - ACCESS_READ   Data-read-access. 
 *                  - ACCESS_WRITE  Data-write-access.
 *                  - ACCESS_EXEC   Code-read-access.
 *                  - NO_CHECK      Do not perform access checks.
 *                  - VPTE          VPTE access; if this misses, it's a double miss.
 *                  - FAKE          Access is not initiated by executing code, but by
 *                                  the debugger. If a translation can't be found
 *                                  through the translation buffer, don't bother.
 *                  - ALT           Use alt_cm for access checks instead of cm.
 *                  - RECUR         Recursive try. We tried to find this address
 *                                  before, added a TB entry, and now it should sail
 *                                  through.
 *                  - PROBE         Access is for a PROBER or PROBEW access; Don't
 *                                  swap in the page if it is outswapped.
 *                  - PROBEW        Access is for a PROBEW access.
 *                  .
 * \param asm_bit Status of the ASM (address space match) bit in the page-table-entry.
 * \param ins     Instruction currently being executed. Important for the correct
 *                handling of traps.
 *               
 * \return        0 on success, -1 if address could not be converted without
 *                help (in this case state.pc contains the address of the
 *                next instruction to execute (PALcode or OS entry point).
 **/
int TranslationBuffer::virt2phys(u64 virt, u64* phys, int flags, bool* asm_bit, u32 ins,
                                 const SCPU_state &state)
{
   int   t = (flags & ACCESS_EXEC) ? 1 : 0;
   int   i;
   int   res;

   int   spe = (flags & ACCESS_EXEC) ? state.i_ctl_spe : state.m_ctl_spe;
   int   asn = (flags & ACCESS_EXEC) ? state.asn : state.asn0;
   int   cm = (flags & ALT) ? state.alt_cm : state.cm;
   bool  forreal = !(flags & FAKE);

#if defined IDB
   if(bListing)
   {
      *phys = virt;
      return 0;
   }
#endif
#if defined(DEBUG_TB)
   if(forreal)
#if defined(IDB)
      if(bTB_Debug)
#endif
         printf("TB %" PRIx64 ",%x: ", virt, flags);
#endif

   // try superpage first.
   if(spe && !cm)
   {
#if defined(DEBUG_TB)
      if(forreal)
#if defined(IDB)
         if(bTB_Debug)
#endif
            printf("try spe...");
#endif

      // HRM 5.3.9: SPE[2], when set, enables superpage mapping when VA[47:46] = 2.
      // In this mode, VA[43:13] are mapped directly to PA[43:13] and VA[45:44] are
      // ignored.
      if(((virt & SPE_2_MASK) == SPE_2_MATCH) && (spe & 4))
      {
         *phys = virt & SPE_2_MAP;
         if(asm_bit)
            *asm_bit = false;
#if defined(DEBUG_TB)
         if(forreal)
#if defined(IDB)
            if(bTB_Debug)
#endif
               printf("SPE\n");
#endif
         return 0;
      }

      // SPE[1], when set, enables superpage mapping when VA[47:41] = 7E. In
      // this mode, VA[40:13] are mapped directly to PA[40:13] and PA[43:41] are
      // copies of PA[40] (sign extension).
      else if(((virt & SPE_1_MASK) == SPE_1_MATCH) && (spe & 2))
      {
         *phys = (virt & SPE_1_MAP) | ((virt & SPE_1_TEST) ? SPE_1_ADD : 0);
         if(asm_bit)
            *asm_bit = false;
#if defined(DEBUG_TB)
         if(forreal)
#if defined(IDB)
            if(bTB_Debug)
#endif
               printf("SPE\n");
#endif
         return 0;
      }

      // SPE[0], when set, enables superpage mapping when VA[47:30] = 3FFFE.
      // In this mode, VA[29:13] are mapped directly to PA[29:13] and PA[43:30] are
      // cleared.
      else if(((virt & SPE_0_MASK) == SPE_0_MATCH) && (spe & 1))
      {
         *phys = virt & SPE_0_MAP;
         if(asm_bit)
            *asm_bit = false;
#if defined(DEBUG_TB)
         if(forreal)
#if defined(IDB)
            if(bTB_Debug)
#endif
               printf("SPE\n");
#endif
         return 0;
      }
   }

   // try to find it in the translation buffer
   i = FindTBEntry(virt, flags, state);

   if(i < 0)       // not found, either trap to PALcode, or try to load the TB entry and try again.
   {
      if(!forreal)  // debugger-lookup of the address
         return -1;  // report failure, and don't look any further
      if(!state.pal_vms)  // unknown PALcode
      {

         // transfer execution to PALcode
         set_exc_addr(state.current_pc);
         if(flags & VPTE)
         {
            set_fault_va(virt);
            set_exc_sum((u64) REG_1 << 8);
            set_pc(state.pal_base + DTBM_DOUBLE_3 + 1);
         }
         else if(flags & ACCESS_EXEC)
         {
            set_pc(state.pal_base + ITB_MISS + 1);
         }
         else
         {
            set_fault_va(virt);
            set_exc_sum((u64) REG_1 << 8);

            u32 opcode = I_GETOP(ins);
            set_mm_stat(
               (
                (opcode == 0x1b || opcode == 0x1f) ? opcode -
                0x18 : opcode
                ) <<
               4 |
               (flags & ACCESS_WRITE));
            set_pc(state.pal_base + DTBM_SINGLE + 1);
         }

         return -1;
      }
      else  // VMS PALcode
      {
         if(flags & RECUR) // we already tried this
         {
            printf("Translationbuffer RECUR lookup failed!\n");
            return -1;
         }

         set_exc_addr(state.current_pc);
         if(flags & VPTE)
         {

            // try to handle the double miss. If this needs to transfer control
            // to the OS, it will return non-zero value.
            if((res = vmspal_ent_dtbm_double_3(flags)) != 0)
               return res;

            // Double miss succesfully handled. Try to get the physical address again.
            return virt2phys(virt, phys, flags | RECUR, asm_bit, ins, state);
         }
         else if(flags & ACCESS_EXEC)
         {

            // try to handle the ITB miss. If this needs to transfer control
            // to the OS, it will return non-zero value.
            if((res = vmspal_ent_itbm(flags)) != 0)
               return res;

            // ITB miss succesfully handled. Try to get the physical address again.
            return virt2phys(virt, phys, flags | RECUR, asm_bit, ins, state);
         }
         else
         {
            set_fault_va(virt);
            set_exc_sum((u64) REG_1 << 8);

            u32 opcode = I_GETOP(ins);
            set_mm_stat(
               (
                (opcode == 0x1b || opcode == 0x1f) ? opcode -
                0x18 : opcode
                ) <<
               4 |
               (flags & ACCESS_WRITE));

            // try to handle the single miss. If this needs to transfer control
            // to the OS, it will return non-zero value.
            if((res = vmspal_ent_dtbm_single(flags)) != 0)
               return res;

            // Single miss succesfully handled. Try to get the physical address again.
            return virt2phys(virt, phys, flags | RECUR, asm_bit, ins, state);
         }
      }
   }

   // If we get here, the number of the matching TB entry is in i.
#if defined(DEBUG_TB)
   else
   {
      if(forreal)
#if defined(IDB)
         if(bTB_Debug)
#endif
            printf("entry %d - ", i);
   }
#endif
   if(!(flags & NO_CHECK))
   {

      // check if requested access is allowed
      if(!(tb[t][i].access[flags & ACCESS_WRITE][cm]))
      {
#if defined(DEBUG_TB)
         if(forreal)
#if defined(IDB)
            if(bTB_Debug)
#endif
               printf("acv\n");
#endif
         if(flags & ACCESS_EXEC)
         {

            // handle I-stream access violation
            set_exc_addr(state.current_pc);
            set_exc_sum(0);
            if(state.pal_vms)
            {
               if((res = vmspal_ent_iacv(flags)) != 0)
                  return res;
            }
            else
            {
               set_pc(state.pal_base + IACV + 1);
               return -1;
            }
         }
         else
         {

            // Handle D-stream access violation
            set_exc_addr(state.current_pc);
            set_fault_va(virt);
            set_exc_sum((u64) REG_1 << 8);

            u32 opcode = I_GETOP(ins);
            set_mm_stat(
               (
                (opcode == 0x1b || opcode == 0x1f) ? opcode -
                0x18 : opcode
                ) <<
               4 |
               (flags & ACCESS_WRITE) |
               2);
            if(state.pal_vms)
            {
               if((res = vmspal_ent_dfault(flags)) != 0)
                  return res;
            }
            else
            {
               set_pc(state.pal_base + DFAULT + 1);
               return -1;
            }
         }
      }

      // check if requested access doesn't fault
      if(tb[t][i].fault[flags & ACCESS_MODE])
      {
#if defined(DEBUG_TB)
         if(forreal)
#if defined(IDB)
            if(bTB_Debug)
#endif
               printf("fault\n");
#endif
         if(flags & ACCESS_EXEC)
         {

            // handle I-stream access fault
            set_exc_addr(state.current_pc);
            set_exc_sum(0);
            if(state.pal_vms)
            {
               if((res = vmspal_ent_iacv(flags)) != 0)
                  return res;
            }
            else
            {
               set_pc(state.pal_base + IACV + 1);
               return -1;
            }
         }
         else
         {

            // handle D-stream access fault
            set_exc_addr(state.current_pc);
            set_fault_va(virt);
            set_exc_sum((u64) REG_1 << 8);

            u32 opcode = I_GETOP(ins);
            set_mm_stat(
               (
                (opcode == 0x1b || opcode == 0x1f) ? opcode -
                0x18 : opcode
                ) <<
               4 |
               (flags & ACCESS_WRITE) |
               ((flags & ACCESS_WRITE) ? 8 : 4));
            if(state.pal_vms)
            {
               if((res = vmspal_ent_dfault(flags)) != 0)
                  return res;
            }
            else
            {
               set_pc(state.pal_base + DFAULT + 1);
               return -1;
            }
         }
      }
   }

   // No access violations or faults
   // Return the converted address
   *phys = tb[t][i].phys | (virt & tb[t][i].keep_mask);
   if(asm_bit)
      *asm_bit = tb[t][i].asm_bit ? true : false;

#if defined(DEBUG_TB)
   if(forreal)
#if defined(IDB)
      if(bTB_Debug)
#endif
         printf("phys: %" PRIx64 " - OK\n", *phys);
#endif
   return 0;
}
