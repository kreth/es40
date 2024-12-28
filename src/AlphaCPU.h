/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * WWW    : http://www.es40.org
 * E-mail : camiel@es40.org
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 * 
 * Although this is not required, the author would appreciate being notified 
 * of, and receiving any modifications you may make to the source code that 
 * might serve the general public.
 */

/**
 * \file
 * Contains the definitions for the emulated DecChip 21264CB EV68 Alpha processor.
 *
 * $Id: AlphaCPU.h,v 1.59 2008/06/12 07:29:44 iamcamiel Exp $
 *
 * X-1.59       Camiel Vanderhoeven                             12-JUN-2008
 *   a) Support to keep secondary CPUs waiting until activated from primary.
 *   b) Support for last written and last read memory locations.
 *
 * X-1.58       Camiel Vanderhoeven                             31-MAY-2008
 *      Changes to include parts of Poco.
 *
 * X-1.57       Camiel Vanderhoeven                             24-MAR-2008
 *      Comments.
 *
 * X-1.56       Camiel Vanderhoeven                             14-MAR-2008
 *      Formatting.
 *
 * X-1.55       Camiel Vanderhoeven                             14-MAR-2008
 *   1. More meaningful exceptions replace throwing (int) 1.
 *   2. U64 macro replaces X64 macro.
 *
 * X-1.54       Camiel Vanderhoeven                             13-MAR-2008
 *      Create init(), start_threads() and stop_threads() functions.
 *
 * X-1.53       Camiel Vanderhoeven                             11-MAR-2008
 *      Named, debuggable mutexes.
 *
 * X-1.52       Camiel Vanderhoeven                             05-MAR-2008
 *      Multi-threading version.
 *
 * X-1.51       Brian Wheeler                                   29-FEB-2008
 *      Add BREAKPOINT INSTRUCTION command to IDB.
 *
 * X-1.50       Camiel Vanderhoeven                             08-FEB-2008
 *      Show originating device name on memory errors.
 *
 * X-1.49       Camiel Vanderhoeven                             01-FEB-2008
 *      Avoid unnecessary shift-operations to calculate constant values.
 *
 * X-1.48       Camiel Vanderhoeven                             30-JAN-2008
 *      Always use set_pc or add_pc to change the program counter.
 *
 * X-1.47       Camiel Vanderhoeven                             30-JAN-2008
 *      Remember number of instructions left in current memory page, so
 *      that the translation-buffer doens't need to be consulted on every
 *      instruction fetch when the Icache is disabled.
 *
 * X-1.46       Camiel Vanderhoeven                             29-JAN-2008
 *      Cleanup.
 *
 * X-1.45       Camiel Vanderhoeven                             29-JAN-2008
 *      Remember separate last found translation-buffer entries for read
 *      and write operations. This should help with memory copy operations.
 *
 * X-1.44       Camiel Vanderhoeven                             28-JAN-2008
 *      Better floating-point exception handling.
 *
 * X-1.43       Camiel Vanderhoeven                             27-JAN-2008
 *      Comments.
 *
 * X-1.40       Camiel Vanderhoeven                             27-JAN-2008
 *      Minor floating-point improvements.
 *
 * X-1.39       Camiel Vanderhoeven                             25-JAN-2008
 *      Added option to disable the icache.
 *
 * X-1.37       Camiel Vanderhoeven                             21-JAN-2008
 *      Moved some macro's to cpu_defs.h; implement new floating-point code.
 *
 * X-1.36       Camiel Vanderhoeven                             19-JAN-2008
 *      Run CPU in a separate thread if CPU_THREADS is defined.
 *      NOTA BENE: This is very experimental, and has several problems.
 *
 * X-1.35       Camiel Vanderhoeven                             18-JAN-2008
 *      Comments.
 *
 * X-1.34       Camiel Vanderhoeven                             18-JAN-2008
 *      Process device interrupts after a 100-cpu-cycle delay.
 *
 * X-1.33       Camiel Vanderhoeven                             08-JAN-2008
 *      Removed last references to IDE disk read SRM replacement.
 *
 * X-1.32       Camiel Vanderhoeven                             02-JAN-2008
 *      Endianess fix.
 *
 * X-1.31       Camiel Vanderhoeven                             02-JAN-2008
 *      Comments. Undid part of last change because of performance impact.
 *
 * X-1.30       Camiel Vanderhoeven                             29-DEC-2007
 *      Avoid referencing uninitialized data.
 *
 * X-1.29       Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.28       Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.27       Camiel Vanderhoeven                             2-DEC-2007
 *      Changed the way translation buffers work, the way interrupts work,
 *      added vmspal routines. 
 *
 * X-1.26       Brian Wheeler                                   1-DEC-2007
 *      Added support for instruction counting, underlined lines in
 *      listings, corrected some unsigned/signed issues.
 *
 * X-1.25	    Brian Wheeler    				                22-NOV-2007
 *	Added set_r and set_f for LOADREG and LOADFPREG debugger commands.
 *
 * X-1.24       Camiel Vanderhoeven                             06-NOV-2007
 *      Performance improvements to ICACHE: last result is kept; cache
 *      lines are larger (512 DWORDS in stead of 16 DWORDS), cache size is
 *      configurable (both number of cache lines and size of each cache 
 *      line), memcpy is used to move memory into the ICACHE.
 *      CAVEAT: ICACHE can only be filled from memory (not from I/O).
 *
 * X-1.23       Eduardo Marcelo Ferrat                          31-OCT-2007
 *      Disable SRM replacement routines.
 *
 * X-1.22       Camiel Vanderhoeven                             17-APR-2007
 *      Give ASM bit a value (true) for PALmode Icache entries.
 *
 * X-1.21       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.20	Camiel Vanderhoeven				7-APR-2007
 *	Added get_hwpcb;
 *
 * X-1.19	Camiel Vanderhoeven				5-APR-2007
 *	Fixed X-1.14. The virtual address was returned instead of the 
 *	physical one!
 *
 * X-1.18       Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.17	Camiel Vanderhoeven				18-MAR-2007
 *   	Removed pointles comparison (v_prbr > 0).
 *
 * X-1.16	Camiel Vanderhoeven				14-MAR-2007
 *	bListing removed.
 *
 * X-1.15	Camiel Vanderhoeven				12-MAR-2007
 *   a)	Added possibility to retrieve physical address of current instruction.
 *   b) Added member function get_pal_base.
 *
 * X-1.14	Camiel Vanderhoeven				9-MAR-2007
 *	Try to translate a virtual PRBR value to a physical one in get_prbr.
 *
 * X-1.13	Camiel Vanderhoeven				8-MAR-2007
 *	va_form now takes a boolean argument bIBOX to determine which ASN
 *	and VPTB to use.
 *
 * X-1.12	Camiel Vanderhoeven				7-MAR-2007				
 *	Added get_tb, get_asn and get_spe functions.
 *
 * X-1.11	Camiel Vanderhoeven				22-FEB-2007
 *	Add ASM bit to the instruction cache & corresponding functions.
 *
 * X-1.10	Camiel Vanderhoeven				18-FEB-2007
 *	Add get_f function.
 *
 * X-1.9        Camiel Vanderhoeven                             16-FEB-2007
 *   a) Added CAlphaCPU::listing.
 *   b) CAlphaCPU::DoClock now returns a value.
 *
 * X-1.8        Camiel Vanderhoeven                             12-FEB-2007
 *	Added get_r and get_prbr functions as inlines.
 *
 * X-1.7        Camiel Vanderhoeven                             12-FEB-2007
 *	Added inline functions to get and update the program counter (pc).
 *
 * X-1.6	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.5        Camiel Vanderhoeven                             9-FEB-2007
 *	Added comments.
 *
 * X-1.4        Camiel Vanderhoeven                             9-FEB-2007
 *      Moved debugging flags (booleans) to TraceEngine.
 *
 * X-1.3	Camiel Vanderhoeven				7-FEB-2007
 *	Added comments.
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 **/
#if !defined(INCLUDED_ALPHACPU_H)
#define INCLUDED_ALPHACPU_H

#include "AlphaCPUState.h"
#include "StateCapture.h"
#include "SystemComponent.h"
#include "System.h"
#include "cpu_defs.h"

class IICache;

#ifdef IDB
#define NEXT                        \
  if(state.single_step_mode)        \
    goto *op_vec[0x40];             \
  else \
  while(!next_ins(ins, opcode)) {}; \
  goto *op_vec[opcode];
#else
#define NEXT                        \
  while(!next_ins(ins, opcode)) {}; \
  goto *op_vec[opcode];
#endif // IDB

/**
 * \brief Emulated CPU.
 *
 * The CPU emulated is the DECchip 21264CB Alpha Processor (EV68).
 * 
 * Documentation consulted:
 *  - Alpha 21264/EV68CB and 21264/EV68DC Microprocessor Hardware Reference Manual [HRM] (http://download.majix.org/dec/21264ev68cb_ev68dc_hrm.pdf)
 *  - DS-0026A-TE: Alpha 21264B Microprocessor Hardware Reference Manual [HRM] (http://ftp.digital.com/pub/Digital/info/semiconductor/literature/21264hrm.pdf)
 *  - Alpha Architecture Reference Manual, fourth edition [ARM] (http://download.majix.org/dec/alpha_arch_ref.pdf)
 *	.
 **/
class CAlphaCPU : public CSystemComponent, public CRunnable
{
  public:
    virtual int   SaveState(FILE* f);
    virtual int   RestoreState(FILE* f);
    void          irq_h(int number, bool assert, int delay);
    int           get_cpuid();

    virtual void  run();    // Poco Thread entry point
    inline void   mips_estimate();
    inline void   skip_memtest();
    inline void   inc_cc();
    inline void   handle_timer();
    inline void   handle_interrupt();
    inline bool   next_ins(u32 &ins, int &opcode);
    void          execute();
    void          release_threads();

    void          set_PAL_BASE(u64 pb);
    virtual void  check_state();
    CAlphaCPU(CConfigurator* cfg, CSystem* system);
    virtual       ~CAlphaCPU();
    u64           get_r(int i, bool translate);
    u64           get_f(int i);
    void          set_r(int reg, u64 val);
    void          set_f(int reg, u64 val);
    u64           get_prbr(void);
    u64           get_hwpcb(void);
    u64           get_pc();
    u64           get_pal_base();

    bool          get_waiting() { return state.wait_for_start; }
    void          stop_waiting() { state.wait_for_start = false; }
    void          enable_single_step_mode() { state.single_step_mode = true; }
    void          disable_single_step_mode() { state.single_step_mode = false; }
    bool          get_single_step_mode() { return state.single_step_mode; }
    void          set_single_step_mode(bool mode) { state.single_step_mode = mode; }
#ifdef IDB
    u64           get_current_pc_physical();
    u64           get_instruction_count();
    u32           get_last_instruction();
    u64           get_last_read_loc() { return last_read_loc; }
    u64           get_last_write_loc() { return last_write_loc; }
#endif
    u64           get_clean_pc();
    void          next_pc();
    void          set_pc(u64 p_pc);
    void          add_pc(u64 a_pc);

    u64           get_speed() { return cpu_hz; };

    u64           va_form(u64 address, bool bIBOX);

#if defined(IDB)
    void          listing(u64 from, u64 to);
    void          listing(u64 from, u64 to, u64 mark);
#endif
    int           virt2phys(u64 virt, u64*  phys, int flags, bool*  asm_bit,
                            u32 instruction);

    virtual void  init();
    virtual void  start_threads();
    virtual void  stop_threads();

    void          run_for_cycles(int num_cycles, std::vector<StateCapture> & log );

    void enable_icache();
    void restore_icache();
      
  private:
    CThread * myThread;
    CSemaphore mySemaphore;
    IICache * myICache;
    bool StopThread;

    // Instruction Cache
    int get_icache(u64 address, u32* data);
    void flush_icache();
    void flush_icache_asm();

    int             FindTBEntry(u64 virt, int flags);
    void            add_tb(u64 virt, u64 pte_phys, u64 pte_flags, int flags);
    void            add_tb_i(u64 virt, u64 pte);
    void            add_tb_d(u64 virt, u64 pte);
    void            tbia(int flags);
    void            tbiap(int flags);
    void            tbis(u64 virt, int flags);

    /* Floating Point routines */
    u64             ieee_lds(u32 op);
    u32             ieee_sts(u64 op);
    u64             ieee_cvtst(u64 op, u32 ins);
    u64             ieee_cvtts(u64 op, u32 ins);
    s32             ieee_fcmp(u64 s1, u64 s2, u32 ins, u32 trap_nan);
    u64             ieee_cvtif(u64 val, u32 ins, u32 dp);
    u64             ieee_cvtfi(u64 op, u32 ins);
    u64             ieee_fadd(u64 s1, u64 s2, u32 ins, u32 dp, bool sub);
    u64             ieee_fmul(u64 s1, u64 s2, u32 ins, u32 dp);
    u64             ieee_fdiv(u64 s1, u64 s2, u32 ins, u32 dp);
    u64             ieee_sqrt(u64 op, u32 ins, u32 dp);
    int             ieee_unpack(u64 op, UFP* r, u32 ins);
    void            ieee_norm(UFP* r);
    u64             ieee_rpack(UFP* r, u32 ins, u32 dp);
    void            ieee_trap(u64 trap, u32 instenb, u64 fpcrdsb, u32 ins);
    u64             vax_ldf(u32 op);
    u64             vax_ldg(u64 op);
    u32             vax_stf(u64 op);
    u64             vax_stg(u64 op);
    void            vax_trap(u64 mask, u32 ins);
    void            vax_unpack(u64 op, UFP* r, u32 ins);
    void            vax_unpack_d(u64 op, UFP* r, u32 ins);
    void            vax_norm(UFP* r);
    u64             vax_rpack(UFP* r, u32 ins, u32 dp);
    u64             vax_rpack_d(UFP* r, u32 ins);
    int             vax_fcmp(u64 s1, u64 s2, u32 ins);
    u64             vax_cvtif(u64 val, u32 ins, u32 dp);
    u64             vax_cvtfi(u64 op, u32 ins);
    u64             vax_fadd(u64 s1, u64 s2, u32 ins, u32 dp, bool sub);
    u64             vax_fmul(u64 s1, u64 s2, u32 ins, u32 dp);
    u64             vax_fdiv(u64 s1, u64 s2, u32 ins, u32 dp);
    u64             vax_sqrt(u64 op, u32 ins, u32 dp);

    /* VMS PALcode call: */
    void            vmspal_call_cflush();
    void            vmspal_call_draina();
    void            vmspal_call_ldqp();
    void            vmspal_call_stqp();
    void            vmspal_call_swpctx();
    void            vmspal_call_mfpr_asn();
    void            vmspal_call_mtpr_asten();
    void            vmspal_call_mtpr_astsr();
    void            vmspal_call_cserve();
    void            vmspal_call_mfpr_fen();
    void            vmspal_call_mtpr_fen();
    void            vmspal_call_mfpr_ipl();
    void            vmspal_call_mtpr_ipl();
    void            vmspal_call_mfpr_mces();
    void            vmspal_call_mtpr_mces();
    void            vmspal_call_mfpr_pcbb();
    void            vmspal_call_mfpr_prbr();
    void            vmspal_call_mtpr_prbr();
    void            vmspal_call_mfpr_ptbr();
    void            vmspal_call_mfpr_scbb();
    void            vmspal_call_mtpr_scbb();
    void            vmspal_call_mtpr_sirr();
    void            vmspal_call_mfpr_sisr();
    void            vmspal_call_mfpr_tbchk();
    void            vmspal_call_mtpr_tbia();
    void            vmspal_call_mtpr_tbiap();
    void            vmspal_call_mtpr_tbis();
    void            vmspal_call_mfpr_esp();
    void            vmspal_call_mtpr_esp();
    void            vmspal_call_mfpr_ssp();
    void            vmspal_call_mtpr_ssp();
    void            vmspal_call_mfpr_usp();
    void            vmspal_call_mtpr_usp();
    void            vmspal_call_mtpr_tbisd();
    void            vmspal_call_mtpr_tbisi();
    void            vmspal_call_mfpr_asten();
    void            vmspal_call_mfpr_astsr();
    void            vmspal_call_mfpr_vptb();
    void            vmspal_call_mtpr_datfx();
    void            vmspal_call_mfpr_whami();
    void            vmspal_call_imb();
    void            vmspal_call_prober();
    void            vmspal_call_probew();
    void            vmspal_call_rd_ps();
    int             vmspal_call_rei();
    void            vmspal_call_swasten();
    void            vmspal_call_wr_ps_sw();
    void            vmspal_call_rscc();
    void            vmspal_call_read_unq();
    void            vmspal_call_write_unq();

    /* VMS PALcode entry: */
    int             vmspal_ent_dtbm_double_3(int flags);
    int             vmspal_ent_dtbm_single(int flags);
    int             vmspal_ent_itbm(int flags);
    int             vmspal_ent_iacv(int flags);
    int             vmspal_ent_dfault(int flags);
    int             vmspal_ent_ext_int(int ei);
    int             vmspal_ent_sw_int(int si);
    int             vmspal_ent_ast_int(int ast);

    /* VMS PALcode internal: */
    int             vmspal_int_initiate_exception();
    int             vmspal_int_initiate_interrupt();

    // ... ... ...
    u64             cc_large;
    u64             start_icount;
    u64             start_cc;
    CTimestamp start_time;
    u64             prev_icount;
    u64             prev_cc;
    u64             prev_time;
    u64             cc_per_instruction;
    u64             ins_per_timer_int;
    u64             next_timer_int;
    u64             cpu_hz;

    struct SCPU_state state;   /**< Determines CPU state that needs to be saved to the state file */

#ifdef IDB
    u64 current_pc_physical;  /**< Physical address of current instruction */
    u32 last_instruction;
    u64 last_read_loc;
    u64 last_write_loc;
#endif
};

/** Translate raw register (0..31) number to a number that takes PALshadow
    registers into consideration (0..63). Considers the program counter
    (to determine if we're in PALmode), and the SDE (Shadow Enable) bit. */
#define RREG(a)                                                          \
    (                                                                    \
      ((a) & 0x1f) +                                                     \
        (((state.pc & 1) && (((a) & 0xc) == 0x4) && state.sde) ? 32 : 0) \
    )

/**
 * Set the PALcode BASE register, and determine whether we're running VMS PALcode.
 **/
inline void CAlphaCPU::set_PAL_BASE(u64 pb)
{
  state.pal_base = pb;
  state.pal_vms = (pb == U64(0x8000));
}

/**
 * Convert a virtual address to va_form format.
 * Used for IPR VA_FORM [HRM 5-5..6] and IPR IVA_FORM [HRM 5-9].
 **/
inline u64 CAlphaCPU::va_form(u64 address, bool bIBOX)
{
  switch(bIBOX ? state.i_ctl_va_mode : state.va_ctl_va_mode)
  {
  case 0:
    return((bIBOX ? state.i_ctl_vptb : state.va_ctl_vptb) & U64(0xfffffffe00000000)) | ((address >> 10) & U64(0x00000001fffffff8));

  case 1:
    return
      ((bIBOX ? state.i_ctl_vptb : state.va_ctl_vptb) & U64(0xfffff80000000000)) | ((address >> 10) & U64(0x0000003ffffffff8)) |
      (((address >> 10) & U64(0x0000002000000000)) * U64(0x3e));

  case 2:
    return((bIBOX ? state.i_ctl_vptb : state.va_ctl_vptb) & U64(0xffffffffc0000000)) | ((address >> 10) & U64(0x00000000003ffff8));
  }

  return 0;
}

/**
 * Return processor number.
 **/
inline int CAlphaCPU::get_cpuid()
{
  return state.iProcNum;
}

/**
 * Assert or release an external interrupt line to the cpu.
 **/
inline void CAlphaCPU::irq_h(int number, bool assert, int delay)
{
  bool  active = (state.eir & (U64(0x1) << number))
  || state.irq_h_timer[number];
  if(assert && !active)
  {
    if(delay)
    {
      state.irq_h_timer[number] = delay;
      state.check_timers = true;
    }
    else
    {
      state.eir |= (U64(0x1) << number);
      state.check_int = true;
    }

    return;
  }

  if(!assert && active)
  {
    state.eir &= ~(U64(0x1) << number);
    state.irq_h_timer[number] = 0;
    state.check_timers = false;
    for(int i = 0; i < 6; i++)
    {
      if(state.irq_h_timer[i])
        state.check_timers = true;
    }
  }
}

/**
 * Return program counter value.
 **/
inline u64 CAlphaCPU::get_pc()
{
  return state.pc;
}

#ifdef IDB

/**
 * Return the physical address the program counter refers to.
 **/
inline u64 CAlphaCPU::get_current_pc_physical()
{
  return state.pc_phys;
}
#endif

/**
 * Return program counter value without PALmode bit.
 **/
inline u64 CAlphaCPU::get_clean_pc()
{
  return state.pc &~U64(0x3);
}

/**
 * Jump to next instruction
 **/
inline void CAlphaCPU::next_pc()
{
  state.pc += 4;
  state.pc_phys += 4;
  if(state.rem_ins_in_page)
    state.rem_ins_in_page--;
}

/**
 * Set program counter to a certain value.
 **/
inline void CAlphaCPU::set_pc(u64 p_pc)
{
  state.pc = p_pc;
  state.rem_ins_in_page = 0;
}

/**
 * Add  value to the program counter.
 **/
inline void CAlphaCPU::add_pc(u64 a_pc)
{
  state.pc += a_pc;
  state.rem_ins_in_page = 0;
}

/**
 * Get a register value.
 * If \a translate is true, use shadow registers if currently enabled.
 **/
inline u64 CAlphaCPU::get_r(int i, bool translate)
{
  if(translate)
    return state.r[RREG(i)];
  else
    return state.r[i];
}

/**
 * Get a fp register value.
 **/
inline u64 CAlphaCPU::get_f(int i)
{
  return state.f[i];
}

/** 
 * Set a register value
 **/
inline void CAlphaCPU::set_r(int reg, u64 value)
{
  state.r[reg] = value;
}

/** 
 * Set a fp register value
 **/
inline void CAlphaCPU::set_f(int reg, u64 value)
{
  state.f[reg] = value;
}

/**
 * Get the PALcode base register.
 **/
inline u64 CAlphaCPU::get_pal_base()
{
  return state.pal_base;
}

/**
 * Get the processor base register.
 * A bit fuzzy...
 **/
inline u64 CAlphaCPU::get_prbr(void)
{
  u64   v_prbr; // virtual
  u64   p_prbr; // physical
  bool  b;
  if(state.r[21 + 32] && ((u64) (state.r[21 + 32] + 0xaf) < (u64)
       ((U64(0x1) << cSystem->get_memory_bits()))))
    v_prbr = cSystem->ReadMem(state.r[21 + 32] + 0xa8, 64, this);
  else
    v_prbr = cSystem->ReadMem(0x70a8 + (0x200 * get_cpuid()), 64, this);
  if(virt2phys(v_prbr, &p_prbr, ACCESS_READ | FAKE | NO_CHECK, &b, 0))
    p_prbr = v_prbr;
  if((u64) p_prbr > (u64) (U64(0x1) << cSystem->get_memory_bits()))
    p_prbr = 0;
  return p_prbr;
}

/**
 * Get the hardware process control block address.
 **/
inline u64 CAlphaCPU::get_hwpcb(void)
{
  u64   v_pcb;  // virtual
  u64   p_pcb;  // physical
  bool  b;
  if(state.r[21 + 32] && ((u64) (state.r[21 + 32] + 0x17) < (u64)
       ((U64(0x1) << cSystem->get_memory_bits()))))
    v_pcb = cSystem->ReadMem(state.r[21 + 32] + 0x10, 64, this);
  else
    v_pcb = cSystem->ReadMem(0x7010 + (0x200 * get_cpuid()), 64, this);
  if(virt2phys(v_pcb, &p_pcb, ACCESS_READ | NO_CHECK | FAKE, &b, 0))
    p_pcb = v_pcb;
  if(p_pcb > (u64) (U64(0x1) << cSystem->get_memory_bits()))
    p_pcb = 0;
  return p_pcb;
}

#if defined(IDB)
/**
 * Return the last instruction executed.
 **/
inline u32 CAlphaCPU::get_last_instruction(void)
{
  return last_instruction;
}
#endif
extern bool bTB_Debug;
#endif // !defined(INCLUDED_ALPHACPU_H)
