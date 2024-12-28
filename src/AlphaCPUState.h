// state of the alpha cpu
#ifndef ALPHA_CPU_STATE_H
#define ALPHA_CPU_STATE_H

#include "cpu_defs.h" // Include any necessary cpu_defs for types

/// Number of entries in each Translation Buffer
#define TB_ENTRIES        16

/// The state structure contains all elements that need to be saved to the statefile
struct SCPU_state
{
      bool  wait_for_start;
      u64   pal_base;       /**< IPR PAL_BASE [HRM: p 5-15] */
      u64   pc;             /**< Program counter */
      u64   cc;             /**< IPR CC: Cycle counter [HRM p 5-3] */
      u64   r[64];          /**< Integer registers (0-31 normal, 32-63 shadow) */
      u64   dc_stat;        /**< IPR DC_STAT: Dcache status [HRM p 5-31..32] */
      bool  ppcen;          /**< IPR PCTX: ppce (proc perf counting enable) [HRM p 5-21..23] */
      u64   i_stat;         /**< IPR I_STAT: Ibox status [HRM p 5-18..20] */
      u64   pctr_ctl;       /**< IPR PCTR_CTL [HRM p 5-23..25] */
      bool  cc_ena;         /**< IPR CC_CTL: Cycle counter enabled [HRM p 5-3] */
      u32   cc_offset;      /**< IPR CC: Cycle counter offset [HRM p 5-3] */
      u64   dc_ctl;         /**< IPR DC_CTL: Dcache control [HRM p 5-30..31] */
      int   alt_cm;         /**< IPR DTB_ALTMODE: alternative cm for HW_LD/HW_ST [HRM p 5-26..27] */
      int   smc;            /**< IPR M_CTL: smc (speculative miss control) [HRM p 5-29..30] */
      bool  fpen;           /**< IPR PCTX: fpe (floating point enable) [HRM p 5-21..23] */
      bool  sde;            /**< IPR I_CTL: sde[1] (PALshadow enable) [HRM p 5-15..18] */
      u64   fault_va;       /**< IPR VA: virtual address of last Dstream miss or fault [HRM p 5-4] */
      u64   exc_sum;        /**< IPR EXC_SUM: exception summary [HRM p 5-13..15] */
      int   i_ctl_va_mode;  /**< IPR I_CTL: (va_form_32 + va_48) [HRM p 5-15..17] */
      int   va_ctl_va_mode; /**< IPR VA_CTL: (va_form_32 + va_48) [HRM p 5-4] */
      u64   i_ctl_vptb;     /**< IPR I_CTL: vptb (virtual page table base) [HRM p 5-15..16] */
      u64   va_ctl_vptb;    /**< IPR VA_CTL: vptb (virtual page table base) [HRM p 5-4] */
      int   cm;           /**< IPR IER_CM: cm (current mode) [HRM p 5-9..10] */
      int   asn;          /**< IPR PCTX: asn (address space number) [HRM p 5-21..22] */
      int   asn0;         /**< IPR DTB_ASN0: asn (address space number) [HRM p 5-28] */
      int   asn1;         /**< IPR DTB_ASN1: asn (address space number) [HRM p 5-28] */
      int   eien;         /**< IPR IER_CM: eien (external interrupt enable) [HRM p 5-9..10] */
      int   slen;         /**< IPR IER_CM: slen (serial line interrupt enable) [HRM p 5-9..10] */
      int   cren;         /**< IPR IER_CM: cren (corrected read error int enable) [HRM p 5-9..10] */
      int   pcen;         /**< IPR IER_CM: pcen (perf counter interrupt enable) [HRM p 5-9..10] */
      int   sien;         /**< IPR IER_CM: sien (software interrupt enable) [HRM p 5-9..10] */
      int   asten;        /**< IPR IER_CM: asten (AST interrupt enable) [HRM p 5-9..10] */
      int   sir;          /**< IPR SIRR: sir (software interrupt request) [HRM p 5-10..11] */
      int   eir;          /**< external interrupt request */
      int   slr;          /**< serial line interrupt request */
      int   crr;          /**< corrected read error interrupt */
      int   pcr;          /**< perf counter interrupt */
      int   astrr;        /**< IPR PCTX: astrr (AST request) [HRM p 5-21..22] */
      int   aster;        /**< IPR PCTX: aster (AST enable) [HRM p 5-21..22] */
      u64   i_ctl_other;  /**< various bits in IPR I_CTL that have no meaning to the emulator */
      u64   mm_stat;      /**< IPR MM_STAT: memory management status [HRM p 5-28..29] */
      bool  hwe;          /**< IPR I_CLT: hwe (allow palmode ins in kernel mode) [HRM p 5-15..17] */
      int   m_ctl_spe;    /**< IPR M_CTL: spe (Super Page mode enabled) [HRM p 5-29..30] */
      int   i_ctl_spe;    /**< IPR I_CTL: spe (Super Page mode enabled) [HRM p 5-15..18] */
      u64   exc_addr;     /**< IPR EXC_ADDR: address of last exception [HRM p 5-8] */
      u64   pmpc;
      u64   fpcr;         /**< Floating-Point Control Register [HRM p 2-36] */
      bool  bIntrFlag;
      u64   current_pc;   /**< Virtual address of current instruction */

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
      u32   rem_ins_in_page;      /**< Number of instructions remaining in current page */
      u64   pc_phys;
      u64   f[64];    /**< Floating point registers (0-31 normal, 32-63 shadow) */
      int   iProcNum; /**< number of the current processor (0 in a 1-processor system) */
      u64   instruction_count;  /**< Number of times doclock has been called */
      u64   last_tb_virt;
      bool  pal_vms;            /**< True if the PALcode base is 0x8000 (=VMS PALcode base) */
      bool  check_int;          /**< True if an interrupt may be pending */
      int   irq_h_timer[6];     /**< Timers for delayed IRQ_H[0:5] assertion */
      bool  check_timers;
      bool  single_step_mode;   /** true, if CPU should only do one step in execute function */
};

#endif // ALPHA_CPU_STATE_H
  
