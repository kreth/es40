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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Although this is not required, the author would appreciate being notified of, 
 * and receiving any modifications you may make to the source code that might serve
 * the general public.
 */

//#define CONSTANT_TIME_FACTOR 100

/**
 * \file 
 * Contains the code for the emulated DecChip 21264CB EV68 Alpha processor.
 *
 * $Id: AlphaCPU.cpp,v 1.82 2009/03/16 01:33:27 iamcamiel Exp $
 *
 * X-1.82       Camiel Vanderhoeven                             15-MAR-2008
 *   a) Added CONSTANT_TIME_FACTOR define to lock the CPU timing.
 *   b) Fixed a bug in SPE0 handling, spotted by David Hittner.
 *
 * X-1.81       Camiel Vanderhoeven                             12-JUN-2008
 *   a) Support to keep secondary CPUs waiting until activated from primary.
 *   b) New unaligned memory access handling.
 *   c) Fixed support for current_pc_physical handling with new icache.
 *
 * X-1.80       Camiel Vanderhoeven                             31-MAY-2008
 *      Changes to include parts of Poco.
 *
 * X-1.79       Camiel Vanderhoeven                             26-MAR-2008
 *      Fix compiler warnings.
 *
 * X-1.78       Camiel Vanderhoeven                             19-MAR-2008
 *      IDB versions compileable again.
 *
 * X-1.77       Camiel Vanderhoeven                             15-MAR-2008
 *      Remove confusing outer for-loop in CAlphaCPU::run().
 *
 * X-1.76       Camiel Vanderhoeven                             14-MAR-2008
 *      Formatting.
 *
 * X-1.75       Camiel Vanderhoeven                             14-MAR-2008
 *   1. More meaningful exceptions replace throwing (int) 1.
 *   2. U64 macro replaces X64 macro.
 *
 * X-1.74       Camiel Vanderhoeven                             13-MAR-2008
 *      Create init(), start_threads() and stop_threads() functions.
 *
 * X-1.73       Camiel Vanderhoeven                             11-MAR-2008
 *      Don't printf calibration loop.
 *
 * X-1.72       Camiel Vanderhoeven                             05-MAR-2008
 *      Multi-threading version.
 *
 * X-1.71       Camiel Vanderhoeven                             04-MAR-2008
 *      Support some basic MP features. (CPUID read from C-Chip MISC 
 *      register, inter-processor interrupts)
 *
 * X-1.70       Camiel Vanderhoeven                             29-FEB-2008
 *      Comments.
 *
 * X-1.69       Brian Wheeler                                   29-FEB-2008
 *      Add BREAKPOINT INSTRUCTION command to IDB.
 *
 * X-1.68       Brian Wheeler                                   27-FEB-2008
 *      Avoid compiler warnings.
 *
 * X-1.67       Camiel Vanderhoeven                             08-FEB-2008
 *      Show originating device name on memory errors.
 *
 * X-1.66       Camiel Vanderhoeven                             05-FEB-2008
 *      Only use new floating-point code when HAVE_NEW_FP has been defined.
 *
 * X-1.65       Camiel Vanderhoeven                             01-FEB-2008
 *      Avoid unnecessary shift-operations to calculate constant values.
 *
 * X-1.64       Camiel Vanderhoeven                             30-JAN-2008
 *      Always use set_pc or add_pc to change the program counter.
 *
 * X-1.63       Camiel Vanderhoeven                             30-JAN-2008
 *      Remember number of instructions left in current memory page, so
 *      that the translation-buffer doens't need to be consulted on every
 *      instruction fetch when the Icache is disabled.
 *
 * X-1.62       Camiel Vanderhoeven                             29-JAN-2008
 *      Comments.
 *
 * X-1.61       Camiel Vanderhoeven                             29-JAN-2008
 *      Undid last change, remember separate last found translation-buffer
 *      entries for read and wrote operations. This should help with memory
 *      copy operations.
 *
 * X-1.60       Camiel Vanderhoeven                             27-JAN-2008
 *      Have GO_PAL throw an exception, so we don't continue doing what we
 *      were doing before the exception was taken.
 *
 * X-1.59       Camiel Vanderhoeven                             26-JAN-2008
 *      Made IDB compile again.
 *
 * X-1.58       Camiel Vanderhoeven                             25-JAN-2008
 *      Added option to disable the icache.
 *
 * X-1.57       Camiel Vanderhoeven                             22-JAN-2008
 *      Nicer initialization of "state" structure.
 *
 * X-1.56       Camiel Vanderhoeven                             22-JAN-2008
 *      Implemented missing /V integer instructions.
 *
 * X-1.55       Camiel Vanderhoeven                             21-JAN-2008
 *      Moved some macro's to cpu_defs.h; implement new floating-point code.
 *
 * X-1.54       Camiel Vanderhoeven                             19-JAN-2008
 *      Run CPU in a separate thread if CPU_THREADS is defined.
 *      NOTA BENE: This is very experimental, and has several problems.
 *
 * X-1.53       Camiel Vanderhoeven                             18-JAN-2008
 *      Replaced sext_64 inlines with sext_u64_<bits> inlines for
 *      performance reasons (thanks to David Hittner for spotting this!);
 *      Process device interrupts after a 100-cpu-cycle delay.
 *
 * X-1.52       David Hittner                                   16-JAN-2008
 *      Added ADDL/V instruction, added MIPS estimate (define MIPS_ESTIMATE)
 *
 * X-1.51       Camiel Vanderhoeven                             08-JAN-2008
 *      Removed last references to IDE disk read SRM replacement.
 *
 * X-1.50       Camiel Vanderhoeven                             30-DEC-2007
 *      Print file id on initialization.
 *
 * X-1.49       Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.48       Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.47       Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.46       Camiel Vanderhoeven                             2-DEC-2007
 *      Changed the way translation buffers work, the way interrupts work. 
 *
 * X-1.45       Brian Wheeler                                   1-DEC-2007
 *      Added support for instruction counting, underlined lines in
 *      listings, corrected some unsigned/signed issues.
 *
 * X-1.44       Camiel Vanderhoeven                             16-NOV-2007
 *      Avoid more compiler warnings.
 *
 * X-1.43       Camiel Vanderhoeven                             16-NOV-2007
 *      Avoid compiler warning about default without any cases.
 *
 * X-1.42       Camiel Vanderhoeven                             08-NOV-2007
 *      Instruction set complete now.
 *
 * X-1.41       Camiel Vanderhoeven                             06-NOV-2007
 *      Performance improvements to ICACHE: last result is kept; cache
 *      lines are larger (512 DWORDS in stead of 16 DWORDS), cache size is
 *      configurable (both number of cache lines and size of each cache 
 *      line), memcpy is used to move memory into the ICACHE.
 *      CAVEAT: ICACHE can only be filled from memory (not from I/O).
 *
 * X-1.40       Camiel Vanderhoeven                             02-NOV-2007
 *      Added integer /V instructions.
 *
 * X-1.39       Camiel Vanderhoeven                             02-NOV-2007
 *      Added missing floating point instructions.
 *
 * X-1.38       Eduardo Marcelo Ferrat                          31-OCT-2007
 *      EXC_SUM contained the wrong register (3 in stead of 1) on a DTBM
 *      exception. Added instructions for CVTDG, CVTGD, MULG, CVTGF.
 *
 * X-1.37       Camiel Vanderhoeven                             18-APR-2007
 *      Faster lockstep mechanism (send info 50 cpu cycles at a time)
 *
 * X-1.36       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.35	Camiel Vanderhoeven				10-APR-2007
 *	New mechanism for SRM replacements. Where these need to be executed,
 *	CSystem::LoadROM() puts a special opcode (a CALL_PAL instruction
 *	with an otherwise illegal operand of 0x01234xx) in memory. 
 *	CAlphaCPU::DoClock() recognizes these opcodes and performs the SRM
 *	action.
 *
 * X-1.34	Camiel Vanderhoeven				10-APR-2007
 *	Unintentional version number increase.
 *
 * X-1.33       Camiel Vanderhoeven                             30-MAR-2007
 *      Formatting.
 *
 * X-1.32	Camiel Vanderhoeven				29-MAR-2007
 *	Added AST to the list of conditions that cause the processor to go to 
 *	the interrupt PAL vector (680).
 *
 * X-1.31	Brian Wheeler					28-MAR-2007
 *	Fixed missing ) after #if !defined(SRM_NO_SPEEDUPS
 *
 * X-1.30	Camiel Vanderhoeven				26-MAR-2007
 *   a)	Possibility to disable SRM-code replacements with the defines
 *	SRM_NO_IDE, SRM_NO_SRL, and SRM_NO_SPEEDUPS
 *   b) Possibility to send SRM-code replacement debugging messages to the
 *	console, with the defines DEBUG_SRM_IDE and DEBUG_SRM_SRL
 *   c)	Added software-interrupts to the list of conditions that can cause
 *	the processot to go to the interrupt PAL vector (680)
 *
 * X-1.29	Camiel Vanderhoeven				14-MAR-2007
 *	Formatting.
 *
 * X-1.28	Camiel Vanderhoeven				14-MAR-2007
 *	Fixed typo in "case 0x22: OP(CPYSE,F12_f3);"
 *
 * X-1.27	Camiel Vanderhoeven				13-MAR-2007
 *	Added some floating-point opcodes, added es_float.h inclusion
 *
 * X-1.26	Camiel Vanderhoeven				12-MAR-2007
 *   a)	Changed call to CTranslationBuffer::convert_address (arguments list
 *	changed)
 *   b) Set values for EXC_SUM and MM_STAT on various exceptions
 *
 * X-1.25	Camiel Vanderhoeven				9-MAR-2007
 *	In the listing-process, addresses were executed twice
 *
 * X-1.24	Camiel Vanderhoeven				8-MAR-2007
 *   a)	Changed call to CTranslationBuffer::write_pte (arguments list
 *	changed)
 *   b)	Backed-out X-1.23 as real problem was solved. (X-1.3 in cpu_bwx.h)
 *
 * X-1.23	Camiel Vanderhoeven				7-MAR-2007				
 *	HACK to stop APB.EXE from crashing when passing bootflags
 *
 * X-1.22	Camiel Vanderhoeven				3-MAR-2007
 *	Wrote code to be executed in stead of SRM console code for writing
 *	to the serial port, and reading from IDE disks. Mechanism is based 
 *	on recognition of the PC value. Should be replaced with a better 
 *	mechanism in the future.
 *
 * X-1.21	Camiel Vanderhoeven				2-MAR-2007
 *	Initialize debug_string to "".
 *
 * X-1.20	Camiel Vanderhoeven				2-MAR-2007
 *	Fixed problem in Save and RestoreState; argument f conflicted with
 *	class member f.
 *
 * X-1.19	Camiel Vanderhoeven				28-FEB-2007
 *	Added support for the lockstep-mechanism.
 *
 * X-1.18	Camiel Vanderhoeven				27-FEB-2007
 *	Removed an unreachable "return 0;" line from DoClock
 *
 * X-1.17	Camiel Vanderhoeven				22-FEB-2007
 *	E_FAULT returned from translation buffer now causes DFAULT exception
 *
 * X-1.16	Camiel Vanderhoven				22-FEB-2007
 *   a)	Changed call to CTranslationBuffer::convert_address (arguments list
 *	changed)
 *   b)	Fixed HW_MTPR and HW_MFPR opcodes
 *
 * X-1.15	Camiel Vanderhoeven				19-FEB-2007
 *	Fixed preprocessor macro concatenation bug (used ## both before and
 *	after the literal; changed this to only before).
 *
 * X-1.14	Camiel Vanderhoeven				18-FEB-2007
 *	Put all actual code behind the processor opcodes in cpu_xxx.h include
 *	files, and replaced them with OP(...,...) macro's in this file.
 *
 * X-1.13       Camiel Vanderhoeven                             16-FEB-2007
 *   a) Added CAlphaCPU::listing.
 *   b) Clocking changes (due to changes in CSystem): CAlphaCPU::DoClock now 
 *      returns a value, and the CPU is registered as a fast clocked device.
 *      
 * X-1.12       Brian Wheeler                                   13-FEB-2007
 *      Different algorithm used for UMULH (previous algorithm suffered from
 *      portability issues).
 *
 * X-1.11       Camiel Vanderhoeven                             13-FEB-2007
 *   a) Bugfix in the UMULH instruction.
 *   b) Bugfix in the HW_MTPR VA_CTL instruction. Now updates va_ctl_va_mode
 *      instead of i_ctl_va_mode.
 *
 * X-1.10       Camiel Vanderhoeven                             12-FEB-2007
 *   a) Moved debugging macro's to cpu_debug.h
 *   b) Cleaned up SEXT and REG macro's (a lot neater now)
 *   c) Moved CAlphaCPU::get_r and CAlphaCPU::get_prbr to AlphaCPU.h as
 *      inline functions
 *   d) Use SEXT macro in a some places where exotic constructions were used 
 *      previously
 *
 * X-1.9        Camiel Vanderhoeven                             12-FEB-2007
 *   a) Added X64_BYTE, X64_WORD, X64_LONG and X64_QUAD, and used these 
 *      instead of the corresponding values.
 *   b) Added ier to the variables that are saved to the state file.
 *
 * X-1.8        Camiel Vanderhoeven                             9-FEB-2007
 *   a) Moved debugging flags (booleans) to CSystem.cpp.
 *   b) Removed loggin of last_write_loc and last_write_val
 *
 * X-1.7        Camiel Vanderhoeven                             7-FEB-2007
 *      Made various dubugging-related statements dependent on the 
 *      definition of IDB (interactive debugger)
 *
 * X-1.6        Camiel Vanderhoeven                             3-FEB-2007
 *      Inline function printable moved to StdAfx.h
 *
 * X-1.5        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.4        Brian Wheeler                                   3-FEB-2007
 *      More scanf and printf statements made compatible with Linux/GCC/glibc.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Scanf and printf statements made compatible with Linux/GCC/glibc.
 *      
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Includes are now case-correct (necessary on Linux)
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 **/
#include "StdAfx.h"
#include "AlphaCPU.h"
#include "TraceEngine.h"
#include "lockstep.h"
#include "cpu_memory.h"
#include "cpu_control.h"
#include "cpu_arith.h"
#include "cpu_logical.h"
#include "cpu_bwx.h"
#include "cpu_fp_memory.h"
#include "cpu_fp_branch.h"
#include "cpu_fp_operate.h"
#include "cpu_misc.h"
#include "cpu_vax.h"
#include "cpu_mvi.h"
#include "cpu_pal.h"
#include "cpu_debug.h"
#include "ICache.h"
#include "TranslationBuffer.h"
#include "DebugOutput.h"

#if !defined(HAVE_NEW_FP)
#include "es40_float.h"
#endif
void CAlphaCPU::release_threads()
{
  mySemaphore.set();
}

void CAlphaCPU::run()
{
  try
  {
    mySemaphore.wait();
    while (state.wait_for_start)
    {
      if(StopThread)
        return;
      CThread::sleep(1);
    }
    printf("*** CPU%d *** STARTING ***\n", get_cpuid());
    for(;;)
    {
      if(StopThread)
        return;
      //for(int i = 0; i < 1000000; i++)
      execute();
    }
  }
  catch(CException & e)
  {
    printf("Exception in CPU thread: %s.\n", e.displayText().c_str());

    // Let the thread die...
  }
}

/**
 * Constructor.
 **/
CAlphaCPU::CAlphaCPU(CConfigurator* cfg, CSystem* system) : CSystemComponent(cfg, system), mySemaphore(0, 1)
{
  myICache = new ICache(cfg, this);
  myTB = new TranslationBuffer(this);
  StopThread = false;
}

/**
 * Initialize the CPU.
 **/
void CAlphaCPU::init()
{
  memset(&state, 0, sizeof(state));

  cpu_hz = myCfg->get_num_value("speed", true, 500000000);

  state.iProcNum = cSystem->RegisterCPU(this);

  state.wait_for_start = (state.iProcNum == 0) ? false : true;
  state.single_step_mode = false;
  enable_icache();
  flush_icache();
  // read config and enable icache accordingly
  restore_icache();

  tbia(ACCESS_READ);
  tbia(ACCESS_EXEC);

  //  state.fpcr = U64(0x8ff0000000000000);
  state.fpen = true;
  state.i_ctl_other = U64(0x502086);
  state.smc = 1;

  // SROM imitation...
  add_tb(0, 0, U64(0xff61), ACCESS_READ);

#if defined(IDB)
  bListing = false;
#endif
  myThread = 0;

  cc_large = 0;
  prev_cc = 0;
  start_cc = 0;
  prev_time = 0;
  prev_icount = 0;
  start_icount = 0;

#if defined(CONSTANT_TIME_FACTOR)
  cc_per_instruction = CONSTANT_TIME_FACTOR;
#else
  cc_per_instruction = 70;
#endif
  ins_per_timer_int = cpu_hz / 1024;
  next_timer_int = state.iProcNum ? U64(0xFFFFFFFFFFFFFFFF) : ins_per_timer_int;  /* only on CPU 0 */

  state.r[22] = state.r[22 + 32] = state.iProcNum;

  printf("%s(%d): $Id: AlphaCPU.cpp,v 1.82 2009/03/16 01:33:27 iamcamiel Exp $\n",
         devid_string, state.iProcNum);
}

void CAlphaCPU::start_threads()
{
  char  buffer[5];
  mySemaphore.tryWait(1);
  if(!myThread)
  {
    sprintf(buffer, "cpu%d", state.iProcNum);
    myThread = new CThread(buffer);
    printf(" %s", myThread->getName().c_str());
    StopThread = false;
    myThread->start(*this);
  }
}

void CAlphaCPU::stop_threads()
{
  StopThread = true;
  if(myThread)
  {
    mySemaphore.set();
    printf(" %s", myThread->getName().c_str());
    myThread->join();
    delete myThread;
    myThread = 0;
  }

  mySemaphore.tryWait(1);
}

/**
 * Destructor.
 **/
CAlphaCPU::~CAlphaCPU()
{
  delete myICache;
  delete myTB;
  stop_threads();
}

// ICache
int CAlphaCPU::get_icache(u64 address, u32* data)
{
    return myICache->get_icache(address, data, state, cSystem);
}
void CAlphaCPU::flush_icache()
{
    myICache->flush_icache();
}

void CAlphaCPU::flush_icache_asm()
{
    myICache->flush_icache_asm();
}

/**
 * \brief Enable i-cache regardles of config file.
 *
 * Required for SRM-ROM decompression.
 **/
void CAlphaCPU::enable_icache()
{
   myICache->enable_icache();
}

/**
 * \brief Enable or disable i-cache depending on config file.
 **/
void CAlphaCPU::restore_icache()
{
  myICache->restore_icache();
}


#if defined(IDB)
char    dbg_string[1000];
#if !defined(LS_MASTER) && !defined(LS_SLAVE)
char*   dbg_strptr;
#endif

/**
 * \brief Do whatever needs to be done to a debug-string.
 *
 * Used in IDB-mode to handle the disassembly- string. In es40_idb, it is
 * written to the standard output.
 *
 * \param s       Pointer to the debug string.
 **/
void handle_debug_string(char* s)
{
#if defined(LS_SLAVE) || defined(LS_MASTER)

  //    lockstep_compare(s);
  * dbg_strptr++ = '\n';
  *dbg_strptr = '\0';
#else
  if(*s)
    printf("%s\n", s);
#endif
}
#endif
#if defined(MIPS_ESTIMATE)

// MIPS_INTERVAL must take longer than 1 second to execute
// or estimate will generate a divide-by-zero error
#define MIPS_INTERVAL 0xfffffff
static time_t saved = 0;
static u64    count;
static double min_mips = 999999999999999.0;
static double max_mips = 0.0;
#include <time.h>
#endif

/**
 * Check if threads are still running.
 *
 * Calibrate the CPU timing loop.
 **/
void CAlphaCPU::check_state()
{
  if(myThread && !myThread->isRunning())
    FAILURE(Thread, "CPU thread has died");

#if !defined(CONSTANT_TIME_FACTOR)
  if (state.instruction_count>0)
  {
  // correct CPU timing loop...
  u64 icount = state.instruction_count;
  u64 cc = cc_large;
  u64 time = start_time.elapsed();
  s64 ce = cc_per_instruction;

  u64 cc_aim = time * cpu_hz / 1000000; // microsecond resolution
  u64 ce_aim = cc_aim / icount;

  s64 icount_lapse = icount - prev_icount;
  s64 cc_diff = cc_aim - cc;
  s64 ce_diff = (u64) ((float) cc_diff / (float) icount_lapse);

  s64 ce_new = ce_aim + ce_diff;
  if(ce_new < 0)
    ce_new = 0;
  if(ce_new > 200)
    ce_new = 200;

  if(ce_new != ce)
  {
#if defined(DEBUG_CPU_TIMING_LOOP)
    printf("                                    "
	   " time %12" PRId64 " | prev %12" PRId64 "\n",
	   time, prev_time);
    printf("          count lapse %12" PRId64 " |"
	   " curr %12" PRId64 " | prev %12" PRId64 "\n",
	   icount_ lapse, icount, prev_icount);
    printf("cc %12" PRId64 " | aim %12" PRId64 " |"
	   " diff %12" PRId64 " | prev %12" PRId64 "\n",
	   cc, cc_aim, cc_diff, prev_cc);
    printf("ce %12" PRId64 " | aim %12" PRId64 " |"
	   " diff %12" PRId64 " | new  %12" PRId64 "\n",
	   ce, ce_aim, ce_diff, ce_new);
    printf("======================================"
	   "====================================\n");
    printf("cpu %d speed factor: %d\n", get_cpuid(), ce_new);
#endif
    cc_per_instruction = ce_new;
  }

  prev_cc = cc;
  prev_icount = icount;
  prev_time = time;
  }
#endif
  return;
}

inline void CAlphaCPU::mips_estimate()
{
#if defined(MIPS_ESTIMATE)

  // Calculate simulated performance statistics
  if(++count >= MIPS_INTERVAL)
  {
    clock_t  current = clock();
    if(saved > 0)
    {
      double  secs = (current - saved) / (double)CLOCKS_PER_SEC;
      double  ips = MIPS_INTERVAL / secs;
      double  mips = ips / 1000000.0;
      if(max_mips < mips)
        max_mips = mips;
      if(min_mips > mips)
        min_mips = mips;
      printf("ES40 MIPS (%3.1f sec):: current: %5.3f, min: %5.3f, max: %5.3f\n",
             secs, mips, min_mips, max_mips);
    }

    saved = current;
    count = 0;
  }
#endif
}

inline void CAlphaCPU::skip_memtest()
{
  if (!(state.current_pc & U64(0x8b000)))
  {
      return;
  }

  if (state.current_pc == U64(0x8bb90))
  {
    if (state.r[5] != U64(0xaaaaaaaaaaaaaaaa))
    {
      printf("wrong memory check skip!\n");
    }
    else
    {
      state.r[0] = state.r[4];
    }
  }

  if (state.current_pc == U64(0x8bbe0))
  {
    if (state.r[5] != U64(0xaaaaaaaaaaaaaaaa))
    {
      printf("wrong memory check skip!\n");
    }
    else
    {
      state.r[16] = 0;
    }
  }

  if (state.current_pc == U64(0x8bc28))
  {
    if (state.r[5] != U64(0xaaaaaaaaaaaaaaaa))
    {
      printf("wrong memory check skip!\n");
    }
    else
    {
      state.r[8] = state.r[4];
    }
  }

  if (state.current_pc == U64(0x8bc70))
  {
    if (state.r[7] != U64(0x5555555555555555))
    {
      printf("wrong memory check skip1!\n");
    }
    else
    {
      state.r[0] = 0;
    }
  }

  if (state.current_pc == U64(0x8bcb0))
  {
    if (state.r[7] != U64(0x5555555555555555))
    {
      printf("wrong memory check skip2!\n");
    }
    else
    {
      state.r[3] = state.r[4];
    }
  }
}

inline void CAlphaCPU::inc_cc()
{
  // We're actually executing code. Cycle counter should be updated, interrupt and interrupt
  // timer status needs to be checked, and the next instruction should be fetched from the
  // instruction cache.
  // Increase the cycle counter if it is currently enabled.
  state.instruction_count++;
  cc_large += cc_per_instruction;

  if(cc_large > next_timer_int)
  {
    next_timer_int += ins_per_timer_int;
    cSystem->interrupt(-1, true);
  }

  if(state.cc_ena)
  {
    state.cc += cc_per_instruction;
  }
}

inline void CAlphaCPU::handle_timer()
{
  if(state.check_timers)
  {

    // There are one or more active delayed irq_h interrupts. Go through the 6
    // irq_h timers, decrease them as needed, and set the interrupt if the timer
    // reaches 0.
    state.check_timers = false;
    for(int i = 0; i < 6; i++)
    {
      if(state.irq_h_timer[i])
      {

        // This timer is active. Decrease it, and check if it reached 0.
        state.irq_h_timer[i]--;
        if(state.irq_h_timer[i])
        {

          // The timer hasn't reached 0 yet; check on the timers again next clock tick.
          state.check_timers = true;
        }
        else
        {

          // The timer has reached 0. Set the interrupt status, and set the flag that we
          // need to check the interrupt status
          state.eir |= (U64(0x1) << i);
          state.check_int = true;
        }
      }
    }
  }
}

inline void CAlphaCPU::handle_interrupt()
{
  if(state.check_int && !(state.pc & 1))
  {

    // One or more of the variables that affect interrupt status have changed, and we are not
    // currently inside PALmode. It is not certain that this means we hava an interrupt to
    // service, but we might have. This needs to be checked.

    /*      
    if (state.pal_vms) {
      // PALcode base is set to 0x8000; meaning OpenVMS PALcode is currently active. In this
      // case, our VMS PALcode replacement routines are valid, and should be used as it is
      // faster than using the original PALcode.
        
      if (state.eir & state.eien & 6)
        if (vmspal_ent_ext_int(state.eir&state.eien & 6))
          return;

      if (state.sir & state.sien & 0xfffc)
        if (vmspal_ent_sw_int(state.sir&state.sien))
          return;

      if (state.asten && (state.aster & state.astrr & ((1<<(state.cm+1))-1) ))
        if (vmspal_ent_ast_int(state.aster & state.astrr & ((1<<(state.cm+1))-1) ))
          return;

      if (state.sir & state.sien)
        if (vmspal_ent_sw_int(state.sir&state.sien))
          return;
    } else 
*/
    {

      // PALcode base is set to an unsupported value. We have no choice but to transfer control
      // to PALmode at the PALcode interrupt entry point.
      //        if (state.eir & 8)
      //        {
      //          printf("%s: IP interrupt received%s...\n",devid_string, (state.eien&8)?"(enabled)":"(masked)");
      //        }
      if((state.eien & state.eir) || (state.sien & state.sir) || (state.asten
       && (state.aster & state.astrr & ((1 << (state.cm + 1)) - 1))))
      {
        GO_PAL(INTERRUPT);
        return;
      }
    }

    // This point is reached only if there are no more active interrupts. We can safely set
    // check_int to false now to save time on the next CPU clock ticks.
    state.check_int = false;
  }
}

inline bool CAlphaCPU::next_ins(u32 &ins, int &opcode)
{
  if(state.instruction_count % (int)1e6) {
     if(StopThread) {
        // return from inner loop
        opcode =0x40;
        return true;
     }
  }
  mips_estimate();

  state.current_pc = state.pc;

  skip_memtest();

  // Service interrupts
  if(DO_ACTION)
  {
    inc_cc();
    handle_timer();
    handle_interrupt();

    // If profiling is enabled, increase the profiling counter for the current block of addresses.
#if defined(PROFILE)
    PROFILE_DO(state.pc);
#endif

    // Get the next instruction from the instruction cache.
    if(get_icache(state.pc, &ins))
    {
        return false;
    }
#if defined(IDB)
    current_pc_physical = state.pc_phys;
#endif
  }           // if (DO_ACTION)
  else
  {

    // We're not really executing any code (DO_ACTION is false); that means that we're
    // in a debugging session, and just listing instructions at a particular address.
    // In this case, we treat the program counter as a physical address.
    ins = (u32) (cSystem->ReadMem(state.pc, 32, this));
  }

  // Increase the program counter. The current value is retained in state.current_pc.
  next_pc();

  // Clear "always zero" registers. The last instruction might have written something to
  // one of these registers.
  state.r[31] = 0;
  state.f[31] = 0;

#if defined(IDB)
  last_instruction = ins;
#endif

  opcode = ins >> 26;

  return true;
}

/**
 * \brief Called each clock-cycle.
 *
 * This is where the actual CPU emulation takes place. Each clocktick, one instruction
 * is processed by the processor. The instruction pipeline is not emulated, things are
 * complicated enough as it is. The one exception is the instruction cache, which is
 * implemented, to accomodate self-modifying code. The instruction cache can be disabled
 * if self-modifying code is not expected.
 **/
void CAlphaCPU::execute()
{
  u32 ins;
  int i;
  u64 phys_address;
  u64 temp_64;
  u64 temp_64_1;
  u64 temp_64_2;
  UFP ufp1;
  UFP ufp2;

  bool pbc;

  int opcode;
  int function;

  static void* op_vec[0x41] = {
      &&op_00,
      &&unknown,
      &&unknown,
      &&unknown,
      &&unknown,
      &&unknown,
      &&unknown,
      &&unknown,
      &&op_08,
      &&op_09,
      &&op_0a,
      &&op_0b,
      &&op_0c,
      &&op_0d,
      &&op_0e,
      &&op_0f,
      &&op_10,
      &&op_11,
      &&op_12,
      &&op_13,
      &&op_14,
      &&op_15,
      &&op_16,
      &&op_17,
      &&op_18,
      &&op_19,
      &&op_1a,
      &&op_1b,
      &&op_1c,
      &&op_1d,
      &&op_1e,
      &&op_1f,
      &&op_20,
      &&op_21,
      &&op_22,
      &&op_23,
      &&op_24,
      &&op_25,
      &&op_26,
      &&op_27,
      &&op_28,
      &&op_29,
      &&op_2a,
      &&op_2b,
      &&op_2c,
      &&op_2d,
      &&op_2e,
      &&op_2f,
      &&op_30,
      &&op_31,
      &&op_32,
      &&op_33,
      &&op_34,
      &&op_35,
      &&op_36,
      &&op_37,
      &&op_38,
      &&op_39,
      &&op_3a,
      &&op_3b,
      &&op_3c,
      &&op_3d,
      &&op_3e,
      &&op_3f,
      &&exit,
  };

#if defined(IDB)
  char*   funcname = 0;
  dbg_string[0] = '\0';
#if !defined(LS_MASTER) && !defined(LS_SLAVE)
  dbg_strptr = dbg_string;
#endif
#endif

#ifdef IDB
  // handle single step mode
  if(state.single_step_mode) {
    while(!next_ins(ins, opcode)) {};
    goto *op_vec[opcode];
  }
#endif
  
  NEXT;

  // Decode and dispatch opcode. This is kept very compact using the OP-macro defined in
  // cpu_debug.h. For the normal emulator, this simply calls the DO_<mnemonic> macro defined
  // in one of the other cpu_*.h files; but for the interactive debugger, it will also do
  // disassembly, where the second parameter to the macro (e.g. R12_R3) determines the
  // formatting applied to the operands. The macro ends with "return 0;".

op_00:  // CALL_PAL
  function = ins & 0x1fffffff;
  OP(CALL_PAL, PAL);

  //    switch (function)
  //    {
  //      case 0x123401: OP_FNC(vmspal_int_read_ide, NOP);
  //      default: OP(CALL_PAL,PAL);
  //    }
op_08:
  OP(LDA, MEM);

op_09:
  OP(LDAH, MEM);

op_0a:
  OP(LDBU, MEM);

op_0b:
  OP(LDQ_U, MEM);

op_0c:
  OP(LDWU, MEM);

op_0d:
  OP(STW, MEM);

op_0e:
  OP(STB, MEM);

op_0f:
  OP(STQ_U, MEM);

op_10:  // INTA* instructions
  function = (ins >> 5) & 0x7f;
  switch(function)
  {
  case 0x40:  OP(ADDL_V, R12_R3);
  case 0x00:  OP(ADDL, R12_R3);
  case 0x02:  OP(S4ADDL, R12_R3);
  case 0x49:  OP(SUBL_V, R12_R3);
  case 0x09:  OP(SUBL, R12_R3);
  case 0x0b:  OP(S4SUBL, R12_R3);
  case 0x0f:  OP(CMPBGE, R12_R3);
  case 0x12:  OP(S8ADDL, R12_R3);
  case 0x1b:  OP(S8SUBL, R12_R3);
  case 0x1d:  OP(CMPULT, R12_R3);
  case 0x60:  OP(ADDQ_V, R12_R3);
  case 0x20:  OP(ADDQ, R12_R3);
  case 0x22:  OP(S4ADDQ, R12_R3);
  case 0x69:  OP(SUBQ_V, R12_R3);
  case 0x29:  OP(SUBQ, R12_R3);
  case 0x2b:  OP(S4SUBQ, R12_R3);
  case 0x2d:  OP(CMPEQ, R12_R3);
  case 0x32:  OP(S8ADDQ, R12_R3);
  case 0x3b:  OP(S8SUBQ, R12_R3);
  case 0x3d:  OP(CMPULE, R12_R3);
  case 0x4d:  OP(CMPLT, R12_R3);
  case 0x6d:  OP(CMPLE, R12_R3);
  default:    UNKNOWN2;
  }

op_11:  // INTL* instructions
  function = (ins >> 5) & 0x7f;
  switch(function)
  {
  case 0x00:  OP(AND, R12_R3);
  case 0x08:  OP(BIC, R12_R3);
  case 0x14:  OP(CMOVLBS, R12_R3);
  case 0x16:  OP(CMOVLBC, R12_R3);
  case 0x20:  OP(BIS, R12_R3);
  case 0x24:  OP(CMOVEQ, R12_R3);
  case 0x26:  OP(CMOVNE, R12_R3);
  case 0x28:  OP(ORNOT, R12_R3);
  case 0x40:  OP(XOR, R12_R3);
  case 0x44:  OP(CMOVLT, R12_R3);
  case 0x46:  OP(CMOVGE, R12_R3);
  case 0x48:  OP(EQV, R12_R3);
  case 0x61:  OP(AMASK, R2_R3);
  case 0x64:  OP(CMOVLE, R12_R3);
  case 0x66:  OP(CMOVGT, R12_R3);
  case 0x6c:  OP(IMPLVER, X_R3);
  default:    UNKNOWN2;
  }

op_12:  // INTS* instructions
  function = (ins >> 5) & 0x7f;
  switch(function)
  {
  case 0x02:  OP(MSKBL, R12_R3);
  case 0x06:  OP(EXTBL, R12_R3);
  case 0x0b:  OP(INSBL, R12_R3);
  case 0x12:  OP(MSKWL, R12_R3);
  case 0x16:  OP(EXTWL, R12_R3);
  case 0x1b:  OP(INSWL, R12_R3);
  case 0x22:  OP(MSKLL, R12_R3);
  case 0x26:  OP(EXTLL, R12_R3);
  case 0x2b:  OP(INSLL, R12_R3);
  case 0x30:  OP(ZAP, R12_R3);
  case 0x31:  OP(ZAPNOT, R12_R3);
  case 0x32:  OP(MSKQL, R12_R3);
  case 0x34:  OP(SRL, R12_R3);
  case 0x36:  OP(EXTQL, R12_R3);
  case 0x39:  OP(SLL, R12_R3);
  case 0x3b:  OP(INSQL, R12_R3);
  case 0x3c:  OP(SRA, R12_R3);
  case 0x52:  OP(MSKWH, R12_R3);
  case 0x57:  OP(INSWH, R12_R3);
  case 0x5a:  OP(EXTWH, R12_R3);
  case 0x62:  OP(MSKLH, R12_R3);
  case 0x67:  OP(INSLH, R12_R3);
  case 0x6a:  OP(EXTLH, R12_R3);
  case 0x72:  OP(MSKQH, R12_R3);
  case 0x77:  OP(INSQH, R12_R3);
  case 0x7a:  OP(EXTQH, R12_R3);
  default:    UNKNOWN2;
  }

op_13:  // INTM* instructions
  function = (ins >> 5) & 0x7f;
  switch(function)  // ignore /V for now
  {
  case 0x40:  OP(MULL_V, R12_R3);
  case 0x00:  OP(MULL, R12_R3);
  case 0x60:  OP(MULQ_V, R12_R3);
  case 0x20:  OP(MULQ, R12_R3);
  case 0x30:  OP(UMULH, R12_R3);
  default:    UNKNOWN2;
  }

op_14:          // ITFP* instructions
  function = (ins >> 5) & 0x7ff;
  switch(function)
  {
  case 0x004:
    OP(ITOFS, R1_F3);

  case 0x00a:
  case 0x08a:
  case 0x10a:
  case 0x18a:
  case 0x40a:
  case 0x48a:
  case 0x50a:
  case 0x58a:
    OP(SQRTF, F2_F3);

  case 0x00b:
  case 0x04b:
  case 0x08b:
  case 0x0cb:
  case 0x10b:
  case 0x14b:
  case 0x18b:
  case 0x1cb:
  case 0x50b:
  case 0x54b:
  case 0x58b:
  case 0x5cb:
  case 0x70b:
  case 0x74b:
  case 0x78b:
  case 0x7cb:
    OP(SQRTS, F2_F3);

  case 0x014:
    OP(ITOFF, R1_F3);

  case 0x024:
    OP(ITOFT, R1_F3);

  case 0x02a:
  case 0x0aa:
  case 0x12a:
  case 0x1aa:
  case 0x42a:
  case 0x4aa:
  case 0x52a:
  case 0x5aa:
    OP(SQRTG, F2_F3);

  case 0x02b:
  case 0x06b:
  case 0x0ab:
  case 0x0eb:
  case 0x12b:
  case 0x16b:
  case 0x1ab:
  case 0x1eb:
  case 0x52b:
  case 0x56b:
  case 0x5ab:
  case 0x5eb:
  case 0x72b:
  case 0x76b:
  case 0x7ab:
  case 0x7eb:
    OP(SQRTT, F2_F3);

  default:
    UNKNOWN2;
  }

op_15:          // FLTV* instructions
  function = (ins >> 5) & 0x7ff;
  switch(function)
  {
  case 0x0a5:
  case 0x4a5:
    OP(CMPGEQ, F12_F3);

  case 0x0a6:
  case 0x4a6:
    OP(CMPGLT, F12_F3);

  case 0x0a7:
  case 0x4a7:
    OP(CMPGLE, F12_F3);

  case 0x03c:
  case 0x0bc:
    OP(CVTQF, F2_F3);

  case 0x03e:
  case 0x0be:
    OP(CVTQG, F2_F3);

  default:
    if(function & 0x200)
    {
    UNKNOWN2;
    }

    switch(function & 0x7f)
    {
    case 0x000: OP(ADDF, F12_F3);
    case 0x001: OP(SUBF, F12_F3);
    case 0x002: OP(MULF, F12_F3);
    case 0x003: OP(DIVF, F12_F3);
    case 0x01e: OP(CVTDG, F2_F3);
    case 0x020: OP(ADDG, F12_F3);
    case 0x021: OP(SUBG, F12_F3);
    case 0x022: OP(MULG, F12_F3);
    case 0x023: OP(DIVG, F12_F3);
    case 0x02c: OP(CVTGF, F12_F3);
    case 0x02d: OP(CVTGD, F2_F3);
    case 0x02f: OP(CVTGQ, F2_F3);
    default:  UNKNOWN2;
    }
    break;
  }

op_16:          // FLTI* instructions
  function = (ins >> 5) & 0x7ff;
  switch(function)
  {
  case 0x0a4:
  case 0x5a4:
    OP(CMPTUN, F12_F3);

  case 0x0a5:
  case 0x5a5:
    OP(CMPTEQ, F12_F3);

  case 0x0a6:
  case 0x5a6:
    OP(CMPTLT, F12_F3);

  case 0x0a7:
  case 0x5a7:
    OP(CMPTLE, F12_F3);

  case 0x2ac:
  case 0x6ac:
    OP(CVTST, F2_F3);

  default:
    if(((function & 0x600) == 0x200) || ((function & 0x500) == 0x400))
    {
      UNKNOWN2;
    }

    switch(function & 0x3f)
    {
    case 0x00:  OP(ADDS, F12_F3);
    case 0x01:  OP(SUBS, F12_F3);
    case 0x02:  OP(MULS, F12_F3);
    case 0x03:  OP(DIVS, F12_F3);
    case 0x20:  OP(ADDT, F12_F3);
    case 0x21:  OP(SUBT, F12_F3);
    case 0x22:  OP(MULT, F12_F3);
    case 0x23:  OP(DIVT, F12_F3);
    case 0x2c:  OP(CVTTS, F2_F3);
    case 0x2f:  OP(CVTTQ, F2_F3);
    case 0x3c:  if((function & 0x300) == 0x100){ UNKNOWN2; }OP(CVTQS, F2_F3);
    case 0x3e:  if((function & 0x300) == 0x100){ UNKNOWN2; }OP(CVTQT, F2_F3);
    default:    UNKNOWN2;
    }
    break;
  }

op_17:          // FLTL* instructions
  function = (ins >> 5) & 0x7ff;
  switch(function)
  {
  case 0x010:
    OP(CVTLQ, F2_F3);

  case 0x020:
    OP(CPYS, F12_F3);

  case 0x021:
    OP(CPYSN, F12_F3);

  case 0x022:
    OP(CPYSE, F12_F3);

  case 0x024:
    OP(MT_FPCR, X_F1);

  case 0x025:
    OP(MF_FPCR, X_F1);

  case 0x02a:
    OP(FCMOVEQ, F12_F3);

  case 0x02b:
    OP(FCMOVNE, F12_F3);

  case 0x02c:
    OP(FCMOVLT, F12_F3);

  case 0x02d:
    OP(FCMOVGE, F12_F3);

  case 0x02e:
    OP(FCMOVLE, F12_F3);

  case 0x02f:
    OP(FCMOVGT, F12_F3);

  case 0x030:
  case 0x130:
  case 0x530:
    OP(CVTQL, F12_F3);

  default:
    UNKNOWN2;
  }

op_18:          // MISC* instructions
  function = (ins & 0xffff);
  switch(function)
  {
  case 0x0000:  OP(TRAPB, NOP);
  case 0x0400:  OP(EXCB, NOP);
  case 0x4000:  OP(MB, NOP);
  case 0x4400:  OP(WMB, NOP);
  case 0x8000:  OP(FETCH, NOP);
  case 0xA000:  OP(FETCH_M, NOP);
  case 0xC000:  OP(RPCC, X_R1);
  case 0xE000:  OP(RC, X_R1);
  case 0xE800:  OP(ECB, NOP);
  case 0xF000:  OP(RS, X_R1);
  case 0xF800:  OP(WH64, NOP);
  case 0xFC00:  OP(WH64EN, NOP);
  default:      UNKNOWN2;
  }

op_19:          // HW_MFPR
  function = (ins >> 8) & 0xff;
  OP(HW_MFPR, MFPR);

op_1a:         // JSR* instructions
  OP(JMP, JMP);

op_1b:          // PAL reserved - HW_LD
  function = (ins >> 12) & 0xf;
  if(function & 1)
  {
    OP(HW_LDQ, HW_LD);
  }
  else
  {
    OP(HW_LDL, HW_LD);
  }

op_1c:          // FPTI* instructions
  function = (ins >> 5) & 0x7f;
  switch(function)
  {
  case 0x00:  OP(SEXTB, R2_R3);
  case 0x01:  OP(SEXTW, R2_R3);
  case 0x30:  OP(CTPOP, R2_R3);
  case 0x31:  OP(PERR, R2_R3);
  case 0x32:  OP(CTLZ, R2_R3);
  case 0x33:  OP(CTTZ, R2_R3);
  case 0x34:  OP(UNPKBW, R2_R3);
  case 0x35:  OP(UNPKBL, R2_R3);
  case 0x36:  OP(PKWB, R2_R3);
  case 0x37:  OP(PKLB, R2_R3);
  case 0x38:  OP(MINSB8, R12_R3);
  case 0x39:  OP(MINSW4, R12_R3);
  case 0x3a:  OP(MINUB8, R12_R3);
  case 0x3b:  OP(MINUW4, R12_R3);
  case 0x3c:  OP(MAXUB8, R12_R3);
  case 0x3d:  OP(MAXUW4, R12_R3);
  case 0x3e:  OP(MAXSB8, R12_R3);
  case 0x3f:  OP(MAXSW4, R12_R3);
  case 0x70:  OP(FTOIT, F1_R3);
  case 0x78:  OP(FTOIS, F1_R3);
  default:    UNKNOWN2;
  }

op_1d:          // HW_MTPR
  function = (ins >> 8) & 0xff;
  OP(HW_MTPR, MTPR);

op_1e:
  OP(HW_RET, RET);

op_1f:          // HW_ST
  function = (ins >> 12) & 0xf;
  if(function & 1)
  {
    OP(HW_STQ, HW_ST);
  }
  else
  {
    OP(HW_STL, HW_ST);
  }

op_20:
  OP(LDF, FMEM);

op_21:
  OP(LDG, FMEM);

op_22:
  OP(LDS, FMEM);

op_23:
  OP(LDT, FMEM);

op_24:
  OP(STF, FMEM);

op_25:
  OP(STG, FMEM);

op_26:
  OP(STS, FMEM);

op_27:
  OP(STT, FMEM);

op_28:
  OP(LDL, MEM);

op_29:
  OP(LDQ, MEM);

op_2a:
  OP(LDL_L, MEM);

op_2b:
  OP(LDQ_L, MEM);

op_2c:
  OP(STL, MEM);

op_2d:
  OP(STQ, MEM);

op_2e:
  OP(STL_C, MEM);

op_2f:
  OP(STQ_C, MEM);

op_30:
  OP(BR, BR);

op_31:
  OP(FBEQ, FCOND);

op_32:
  OP(FBLT, FCOND);

op_33:
  OP(FBLE, FCOND);

op_34:
  OP(BSR, BSR);

op_35:
  OP(FBNE, FCOND);

op_36:
  OP(FBGE, FCOND);

op_37:
  OP(FBGT, FCOND);

op_38:
  OP(BLBC, COND);

op_39:
  OP(BEQ, COND);

op_3a:
  OP(BLT, COND);

op_3b:
  OP(BLE, COND);

op_3c:
  OP(BLBS, COND);

op_3d:
  OP(BNE, COND);

op_3e:
  OP(BGE, COND);

op_3f:
  OP(BGT, COND);

unknown:
  UNKNOWN1;

exit:
  // fprintf(stderr, "state.pc=" "%016" PRIx64 ", state.current_pc=" "%016" PRIx64 "\n",
  //         state.pc, state.current_pc);
  return;
}

#if defined(IDB)

/**
 * \brief Produce disassembly-listing without marker
 *
 * \param from    Address of first instruction to be disassembled.
 * \param to      Address of instruction following the last instruction to
 *                be disassembled.
 **/
void CAlphaCPU::listing(u64 from, u64 to)
{
  listing(from, to, 0);
}

/**
 * \brief Produce disassembly-listing with marker
 *
 * \param from    Address of first instruction to be disassembled.
 * \param to      Address of instruction following the last instruction to
 *                be disassembled.
 * \param mark    Address of instruction to be underlined with a marker line.
 **/
void CAlphaCPU::listing(u64 from, u64 to, u64 mark)
{
  printf("%%CPU-I-LISTNG: Listing from %016" PRIx64 " to %016" PRIx64 "\n", from, to);

  u64   iSavedPC;
  bool  bSavedDebug;
  iSavedPC = state.pc;
  bSavedDebug = bDisassemble;
  bDisassemble = true;
  bListing = true;
  bool bSavedMode = get_single_step_mode();
  enable_single_step_mode();
  for(state.pc = from; state.pc <= to;)
  {
    execute();
    if(state.pc == mark)
      printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
  }
  set_single_step_mode(bSavedMode);
  bListing = false;
  state.pc = iSavedPC;
  bDisassemble = bSavedDebug;
}

u64 CAlphaCPU::get_instruction_count()
{
  return state.instruction_count;
}
#endif
static u32  cpu_magic1 = 0x2126468C;
static u32  cpu_magic2 = 0xC8646212;

/**
 * Save state to a Virtual Machine State file.
 **/
int CAlphaCPU::SaveState(FILE* f)
{
  long  ss = sizeof(state);

  fwrite(&cpu_magic1, sizeof(u32), 1, f);
  fwrite(&ss, sizeof(long), 1, f);
  fwrite(&state, sizeof(state), 1, f);
  fwrite(&cpu_magic2, sizeof(u32), 1, f);
  printf("%s: %ld bytes saved.\n", devid_string, ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/
int CAlphaCPU::RestoreState(FILE* f)
{
  long    ss;
  u32     m1;
  u32     m2;
  size_t  r;

  r = fread(&m1, sizeof(u32), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  if(m1 != cpu_magic1)
  {
    printf("%s: MAGIC 1 does not match!\n", devid_string);
    return -1;
  }

  r = fread(&ss, sizeof(long), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  if(ss != sizeof(state))
  {
    printf("%s: STRUCT SIZE does not match!\n", devid_string);
    return -1;
  }

  r = fread(&state, sizeof(state), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  r = fread(&m2, sizeof(u32), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  if(m2 != cpu_magic2)
  {
    printf("%s: MAGIC 1 does not match!\n", devid_string);
    return -1;
  }

  printf("%s: %ld bytes restored.\n", devid_string, ss);
  return 0;
}

/***************************************************************************/

/**
 * \name TB
 * Translation Buffer related functions
 ******************************************************************************/

//\{

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
int CAlphaCPU::FindTBEntry(u64 virt, int flags)
{
   return myTB->FindTBEntry(virt, flags, state);
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
int CAlphaCPU::virt2phys(u64 virt, u64* phys, int flags, bool* asm_bit, u32 ins)
{
   return myTB->virt2phys(virt, phys, flags, asm_bit, ins, state);
}

/**
 * \brief Add translation-buffer entry
 *
 * Add a translation-buffer entry to one of the translation buffers.
 *
 * \param virt    Virtual address.
 * \param pte     Translation in DTB_PTE format (see add_tb_d).
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/
void CAlphaCPU::add_tb(u64 virt, u64 pte_phys, u64 pte_flags, int flags)
{
   myTB->add_tb(virt, pte_phys, pte_flags, flags, state);
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
void CAlphaCPU::add_tb_d(u64 virt, u64 pte)
{
   add_tb(virt, pte >> (32 - 13), pte, ACCESS_READ);
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
void CAlphaCPU::add_tb_i(u64 virt, u64 pte)
{
  add_tb(virt, pte, pte & 0xf70, ACCESS_EXEC);
}

/**
 * \brief Invalidate all translation-buffer entries
 *
 * Invalidate all translation-buffer entries in one of the translation buffers.
 *
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/
void CAlphaCPU::tbia(int flags)
{
  myTB->tbia(flags, state);
}

/**
 * \brief Invalidate all process-specific translation-buffer entries
 *
 * Invalidate all translation-buffer entries that do not have the ASM bit
 * set in one of the translation buffers.
 *
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/
void CAlphaCPU::tbiap(int flags)
{
  myTB->tbiap(flags, state);
}

/**
 * \brief Invalidate single translation-buffer entry
 *
 * \param virt    Virtual address for which the entry should be invalidated.
 * \param flags   ACCESS_EXEC determines which translation buffer to use.
 **/
void CAlphaCPU::tbis(u64 virt, int flags)
{
  myTB->tbis(virt, flags, state);
}

//\}

#if defined(IDB)
const char*   PAL_NAME[] = {
  "HALT", "CFLUSH", "DRAINA", "LDQP", "STQP", "SWPCTX", "MFPR_ASN",
  "MTPR_ASTEN",
  "MTPR_ASTSR", "CSERVE", "SWPPAL", "MFPR_FEN", "MTPR_FEN", "MTPR_IPIR",
  "MFPR_IPL", "MTPR_IPL",
  "MFPR_MCES", "MTPR_MCES", "MFPR_PCBB", "MFPR_PRBR", "MTPR_PRBR",
  "MFPR_PTBR", "MFPR_SCBB", "MTPR_SCBB",
  "MTPR_SIRR", "MFPR_SISR", "MFPR_TBCHK", "MTPR_TBIA", "MTPR_TBIAP",
  "MTPR_TBIS", "MFPR_ESP", "MTPR_ESP",
  "MFPR_SSP", "MTPR_SSP", "MFPR_USP", "MTPR_USP", "MTPR_TBISD", "MTPR_TBISI",
  "MFPR_ASTEN", "MFPR_ASTSR",
  "28", "MFPR_VPTB", "MTPR_VPTB", "MTPR_PERFMON", "2C", "2D", "MTPR_DATFX",
  "2F",
  "30", "31", "32", "33", "34", "35", "36", "37",
  "38", "39", "3A", "3B", "3C", "3D", "WTINT", "MFPR_WHAMI",
  "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
  "-",
  "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
  "-",
  "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
  "-",
  "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
  "-",
  "BPT", "BUGCHK", "CHME", "CHMK", "CHMS", "CHMU", "IMB", "INSQHIL",
  "INSQTIL", "INSQHIQ", "INSQTIQ", "INSQUEL", "INSQUEQ", "INSQUEL/D",
  "INSQUEQ/D", "PROBER",
  "PROBEW", "RD_PS", "REI", "REMQHIL", "REMQTIL", "REMQHIQ", "REMQTIQ",
  "REMQUEL",
  "REMQUEQ", "REMQUEL/D", "REMQUEQ/D", "SWASTEN", "WR_PS_SW", "RSCC",
  "READ_UNQ", "WRITE_UNQ",
  "AMOVRR", "AMOVRM", "INSQHILR", "INSQTILR", "INSQHIQR", "INSQTIQR",
  "REMQHILR", "REMQTILR",
  "REMQHIQR", "REMQTIQR", "GENTRAP", "AB", "AC", "AD", "CLRFEN", "AF",
  "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB",
  "BC", "BD", "BE", "BF"
};

const char*   IPR_NAME[] = {
  "ITB_TAG", "ITB_PTE", "ITB_IAP", "ITB_IA", "ITB_IS", "PMPC", "EXC_ADDR",
  "IVA_FORM",
  "IER_CM", "CM", "IER", "IER_CM", "SIRR", "ISUM", "HW_INT_CLR", "EXC_SUM",
  "PAL_BASE", "I_CTL", "IC_FLUSH_ASM", "IC_FLUSH", "PCTR_CTL", "CLR_MAP",
  "I_STAT", "SLEEP",
  "?0001.1000?", "?0001.1001?", "?0001.1010?", "?0001.1011?", "?0001.1100?",
  "?0001.1101?", "?0001.1110?", "?0001.1111?",
  "DTB_TAG0", "DTB_PTE0", "?0010.0010?", "?0010.0011?", "DTB_IS0", "DTB_ASN0",
  "DTB_ALTMODE", "MM_STAT",
  "M_CTL", "DC_CTL", "DC_STAT", "C_DATA", "C_SHFT", "M_FIX", "?0010.1110?",
  "?0010.1111?",
  "?0011.0000?", "?0011.0001?", "?0011.0010?", "?0011.0011?", "?0011.0100?",
  "?0010.0101?", "?0010.0110?", "?0010.0111?",
  "?0011.1000?", "?0011.1001?", "?0011.1010?", "?0011.1011?", "?0011.1100?",
  "?0010.1101?", "?0010.1110?", "?0010.1111?",
  "PCTX.00000", "PCTX.00001", "PCTX.00010", "PCTX.00011", "PCTX.00100",
  "PCTX.00101", "PCTX.00110", "PCTX.00111",
  "PCTX.01000", "PCTX.01001", "PCTX.01010", "PCTX.01011", "PCTX.01100",
  "PCTX.01101", "PCTX.01110", "PCTX.01111",
  "PCTX.10000", "PCTX.10001", "PCTX.10010", "PCTX.10011", "PCTX.10100",
  "PCTX.10101", "PCTX.10110", "PCTX.10111",
  "PCTX.11000", "PCTX.11001", "PCTX.11010", "PCTX.11011", "PCTX.11100",
  "PCTX.11101", "PCTX.11110", "PCTX.11111",
  "PCTX.00000", "PCTX.00001", "PCTX.00010", "PCTX.00011", "PCTX.00100",
  "PCTX.00101", "PCTX.00110", "PCTX.00111",
  "PCTX.01000", "PCTX.01001", "PCTX.01010", "PCTX.01011", "PCTX.01100",
  "PCTX.01101", "PCTX.01110", "PCTX.01111",
  "PCTX.10000", "PCTX.10001", "PCTX.10010", "PCTX.10011", "PCTX.10100",
  "PCTX.10101", "PCTX.10110", "PCTX.10111",
  "PCTX.11000", "PCTX.11001", "PCTX.11010", "PCTX.11011", "PCTX.11100",
  "PCTX.11101", "PCTX.11110", "PCTX.11111",
  "?1000.0000?", "?1000.0001?", "?1000.0010?", "?1000.0011?", "?1000.0100?",
  "?1000.0101?", "?1000.0110?", "?1000.0111?",
  "?1000.1000?", "?1000.1001?", "?1000.1010?", "?1000.1011?", "?1000.1100?",
  "?1000.1101?", "?1000.1110?", "?1000.1111?",
  "?1001.0000?", "?1001.0001?", "?1001.0010?", "?1001.0011?", "?1001.0100?",
  "?1001.0101?", "?1001.0110?", "?1001.0111?",
  "?1001.1000?", "?1001.1001?", "?1001.1010?", "?1001.1011?", "?1001.1100?",
  "?1001.1101?", "?1001.1110?", "?1001.1111?",
  "DTB_TAG1", "DTB_PTE1", "DTB_IAP", "DTB_IA", "DTB_IS1", "DTB_ASN1",
  "?1010.0110?", "?1010.0111?",
  "?1010.1000?", "?1010.1001?", "?1010.1010?", "?1010.1011?", "?1010.1100?",
  "?1010.1101?", "?1010.1110?", "?1010.1111?",
  "?1011.0000?", "?1011.0001?", "?1011.0010?", "?1011.0011?", "?1011.0100?",
  "?1011.0101?", "?1011.0110?", "?1011.0111?",
  "?1011.1000?", "?1011.1001?", "?1011.1010?", "?1011.1011?", "?1011.1100?",
  "?1011.1101?", "?1011.1110?", "?1011.1111?",
  "CC", "CC_CTL", "VA", "VA_FORM", "VA_CTL", "?1100.0101?", "?1100.0110?",
  "?1100.0111?",
  "?1100.1000?", "?1100.1001?", "?1100.1010?", "?1100.1011?", "?1100.1100?",
  "?1100.1101?", "?1100.1110?", "?1100.1111?",
  "?1101.0000?", "?1101.0001?", "?1101.0010?", "?1101.0011?", "?1101.0100?",
  "?1101.0101?", "?1101.0110?", "?1101.0111?",
  "?1101.1000?", "?1101.1001?", "?1101.1010?", "?1101.1011?", "?1101.1100?",
  "?1101.1101?", "?1101.1110?", "?1101.1111?",
  "?1110.0000?", "?1110.0001?", "?1110.0010?", "?1110.0011?", "?1110.0100?",
  "?1110.0101?", "?1110.0110?", "?1110.0111?",
  "?1110.1000?", "?1110.1001?", "?1110.1010?", "?1110.1011?", "?1110.1100?",
  "?1110.1101?", "?1110.1110?", "?1110.1111?",
  "?1111.0000?", "?1111.0001?", "?1111.0010?", "?1111.0011?", "?1111.0100?",
  "?1111.0101?", "?1111.0110?", "?1111.0111?",
  "?1111.1000?", "?1111.1001?", "?1111.1010?", "?1111.1011?", "?1111.1100?",
  "?1111.1101?", "?1111.1110?", "?1111.1111?",
};
#endif
