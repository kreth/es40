#ifndef TRANSLATIONBUFFER_H
#define TRANSLATIONBUFFER_H
#include "ITranslationBuffer.h"
#include "AlphaCPUState.h"

/// Number of entries in each Translation Buffer
#define TB_ENTRIES        16

class CAlphaCPU;
class TranslationBuffer : public ITranslationBuffer
{
   public:
      TranslationBuffer(CAlphaCPU* cpu);
      virtual ~TranslationBuffer();
      int FindTBEntry(u64 virt, int flags, const SCPU_state &state) override;
      void add_tb(u64 virt, u64 pte_phys, u64 pte_flags, int flags, const SCPU_state &state) override;
      void add_tb_d(u64 virt, u64 pte, const SCPU_state &state) override;
      void add_tb_i(u64 virt, u64 pte, const SCPU_state &state) override;
      void tbia(int flags, const SCPU_state &state) override;
      void tbiap(int flags, const SCPU_state &state) override;
      void tbis(u64 virt, int flags, const SCPU_state &state) override;
      int virt2phys(u64 virt, u64* phys, int flags, bool* asm_bit, u32 ins,
                    const SCPU_state &state) override;
  private:
      void set_pc(u64 p_pc);
      void set_exc_sum(u64 exception_summary);
      void set_exc_addr(u64 address_of_last_exception);
      void set_mm_stat(u64 mem_mgmt_status);
      void set_fault_va(u64 va_of_last_Dmiss_or_fault);
      
      int vmspal_ent_dtbm_double_3(int flags);
      int vmspal_ent_dtbm_single(int flags);
      int vmspal_ent_itbm(int flags);
      int vmspal_ent_iacv(int flags);
      int vmspal_ent_dfault(int flags);


      
      CAlphaCPU* cCpu;
      /**
       * \brief Translation Buffer Entry.
       *
       * A translation buffer entry provides the mapping from a page of virtual memory to a page of physical memory.
       **/
      struct STBEntry
      {
            u64   virt;         /**< Virtual address of page*/
            u64   phys;         /**< Physical address of page*/
            u64   match_mask;   /**< The virtual address has to match for these bits to be a hit*/
            u64   keep_mask;    /**< This part of the virtual address is OR-ed with the phys address*/
            int   asn;          /**< Address Space Number*/
            int   asm_bit;      /**< Address Space Match bit*/
            int   access[2][4]; /**< Access permitted [read/write][current mode]*/
            int   fault[3];     /**< Fault on access [read/write/execute]*/
            bool  valid;        /**< Valid entry*/
      } tb[2][TB_ENTRIES];  /**< Translation buffer entries */
      int   next_tb[2];     /**< Number of next translation buffer entry to use */
      int   last_found_tb[2][2];  /**< Number of last translation buffer entry found */
};
extern bool bTB_Debug; // set by TraceEngine on user request
#endif
