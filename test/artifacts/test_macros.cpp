#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define U64(a)  UINT64_C(a)
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

typedef u8  u_int8_t;
typedef u16 u_int16_t;
typedef u32 u_int32_t;
typedef u64 u_int64_t;

/**
 * Sign-extend a 32-bit value to 64 bits.
 **/
inline u64 sext_u64_32(u64 a)
{
  return (
	  ((a) & U64(0x0000000080000000)) ? ((a) | U64(0xffffffff00000000)) :
	  ((a) & U64(0x00000000ffffffff))
	  );
}

/// The state structure contains all elements
///that need to be saved to the statefile
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

  u32   rem_ins_in_page; /**< Number of instructions remaining in current page */
};

// INTERRUPT VECTORS
#define DTBM_DOUBLE_3 U64(0x100)
#define DTBM_DOUBLE_4 U64(0x180)
#define FEN           U64(0x200)
#define UNALIGN       U64(0x280)
#define DTBM_SINGLE   U64(0x300)
#define DFAULT        U64(0x380)
#define OPCDEC        U64(0x400)
#define IACV          U64(0x480)
#define MCHK          U64(0x500)
#define ITB_MISS      U64(0x580)
#define ARITH         U64(0x600)
#define INTERRUPT     U64(0x680)
#define MT_FPCR       U64(0x700)
#define RESET         U64(0x780)

/* Instruction formats */
#define I_V_OP        26        /* opcode */
#define I_M_OP        0x3F
#define I_OP          (I_M_OP << I_V_OP)
#define I_V_RA        21        /* Ra */
#define I_M_RA        0x1F
#define I_V_RB        16        /* Rb */
#define I_M_RB        0x1F
#define I_V_FTRP      13        /* floating trap mode */
#define I_M_FTRP      0x7
#define I_FTRP        (I_M_FTRP << I_V_FTRP)
#define I_F_VAXRSV    0x4800    /* VAX reserved */
#define I_FTRP_V      0x2000    /* /V trap */
#define I_FTRP_U      0x2000    /* /U trap */
#define I_FTRP_S      0x8000    /* /S trap */
#define I_FTRP_SUI    0xE000    /* /SUI trap */
#define I_FTRP_SVI    0xE000    /* /SVI trap */
#define I_V_FRND      11        /* floating round mode */
#define I_M_FRND      0x3
#define I_FRND        (I_M_FRND << I_V_FRND)
#define I_FRND_C      0         /* chopped */
#define I_FRND_M      1         /* to minus inf */
#define I_FRND_N      2         /* normal */
#define I_FRND_D      3         /* dynamic */
#define I_FRND_P      3         /* in FPCR: plus inf */
#define I_V_FSRC      9         /* floating source */
#define I_M_FSRC      0x3
#define I_FSRC        (I_M_FSRC << I_V_FSRC)
#define I_FSRC_X      0x0200    /* data type X */
#define I_V_FFNC      5         /* floating function */
#define I_M_FFNC      0x3F
#define I_V_LIT8      13        /* integer 8b literal */
#define I_M_LIT8      0xFF
#define I_V_ILIT      12        /* literal flag */
#define I_ILIT        (1u << I_V_ILIT)
#define I_V_IFNC      5         /* integer function */
#define I_M_IFNC      0x3F
#define I_V_RC        0         /* Rc */
#define I_M_RC        0x1F
#define I_V_MDSP      0         /* memory displacement */
#define I_M_MDSP      0xFFFF
#define I_V_BDSP      0
#define I_M_BDSP      0x1FFFFF  /* branch displacement */
#define I_V_PALOP     0
#define I_M_PALOP     0x3FFFFFF /* PAL subopcode */
#define I_GETOP(x)    (((x) >> I_V_OP) & I_M_OP)
#define I_GETRA(x)    (((x) >> I_V_RA) & I_M_RA)
#define I_GETRB(x)    (((x) >> I_V_RB) & I_M_RB)
#define I_GETLIT8(x)  (((x) >> I_V_LIT8) & I_M_LIT8)
#define I_GETIFNC(x)  (((x) >> I_V_IFNC) & I_M_IFNC)
#define I_GETFRND(x)  (((x) >> I_V_FRND) & I_M_FRND)
#define I_GETFFNC(x)  (((x) >> I_V_FFNC) & I_M_FFNC)
#define I_GETRC(x)    (((x) >> I_V_RC) & I_M_RC)
#define I_GETMDSP(x)  (((x) >> I_V_MDSP) & I_M_MDSP)
#define I_GETBDSP(x)  (((x) >> I_V_BDSP) & I_M_BDSP)
#define I_GETPAL(x)   (((x) >> I_V_PALOP) & I_M_PALOP)

#define UF_V_NM 63
#define UF_NM   U64(0x8000000000000000)         /* normalized */

/* Bit patterns */
#define X64_BYTE      U64(0xff)
#define X64_WORD      U64(0xffff)
#define X64_LONG      U64(0xffffffff)
#define X64_QUAD      U64(0xffffffffffffffff)
#define B_SIGN        U64(0x80)
#define W_SIGN        U64(0x8000)
#define L_SIGN        U64(0x80000000)
#define Q_SIGN        U64(0x8000000000000000)
#define Q_GETSIGN(x)  (((x) >> 63) & 1)

/* Traps - corresponds to arithmetic trap summary register */
#define TRAP_SWC  U64(0x01) /* software completion */
#define TRAP_INV  U64(0x02) /* invalid operand */
#define TRAP_DZE  U64(0x04) /* divide by zero */
#define TRAP_OVF  U64(0x08) /* overflow */
#define TRAP_UNF  U64(0x10) /* underflow */
#define TRAP_INE  U64(0x20) /* inexact */
#define TRAP_IOV  U64(0x40) /* integer overflow */

#define TRAP_INT  U64(0x80) /* exception register is integer reg */

#define ARITH_TRAP(flags, reg)                                     \
  {                                                                \
    state.exc_sum |= flags; /* cause of trap */                    \
    state.exc_sum |= (reg & 0x1f) << 8; /* destination register */ \
    GO_PAL(ARITH);  /* trap */                                     \
  }

#define ARITH_TRAP_I(flags, reg)      \
  {                                   \
    state.exc_sum = 0;                \
    ARITH_TRAP(TRAP_INT | flags, reg) \
  }

#define GO_PAL(offset)                                          \
  {                                                             \
    if(bDisassemble)                                            \
    {                                                           \
      sprintf(dbg_strptr, " ==> PAL %" PRIx64 "!\n", offset);   \
      dbg_strptr += strlen(dbg_strptr);                         \
    }                                                           \
    handle_debug_string(dbg_string);                            \
    state.exc_addr = state.current_pc;                          \
    set_pc(state.pal_base | offset | 1);                        \
  }


/** Translate raw register (0..31) number to a number that takes PALshadow
    registers into consideration (0..63). Considers the program counter
    (to determine if we're in PALmode), and the SDE (Shadow Enable) bit. */
#define RREG(a)                                                          \
    (                                                                    \
      ((a) & 0x1f) +                                                     \
        (((state.pc & 1) && (((a) & 0xc) == 0x4) && state.sde) ? 32 : 0) \
    )


#define REG_1         RREG(I_GETRA(ins))
#define REG_2         RREG(I_GETRB(ins))
#define REG_3         RREG(I_GETRC(ins))
#define FREG_1        (I_GETRA(ins))
#define FREG_2        (I_GETRB(ins))
#define FREG_3        (I_GETRC(ins))
#define RA            REG_1
#define RAV           state.r[RA]
#define RB            REG_2
#define RBV           ((ins & 0x1000) ? ((ins >> 13) & 0xff) : state.r[RB])
#define V_2           RBV
#define RC            REG_3
#define RCV           state.r[RC]

#define DEBUG_XX sprintf(dbg_strptr, bListing ? "%08" PRIx64 ": " : "%016" PRIx64 "", state.current_pc); \
  dbg_strptr += strlen(dbg_strptr); \
  sprintf(dbg_strptr, "(%08x): ", ins); \
  dbg_strptr += strlen(dbg_strptr);

#define POST_R12_R3 POST_X64(state.r[REG_3]);
#define POST_X_R1 POST_X64(state.r[REG_1]);
#define POST_X64(a)                           \
  if(bDisassemble)                            \
  {                                           \
    if(!bListing)                             \
    {                                         \
      sprintf(dbg_strptr, " ==> %" PRIx64 "", a); \
      dbg_strptr += strlen(dbg_strptr);       \
    }                                         \
  }

#define PRE_X_R1(mnemonic)                             \
  if(bDisassemble)                                     \
  {                                                    \
    DEBUG_XX;                                          \
    sprintf(dbg_strptr, #mnemonic " r%d", REG_1 & 31); \
    dbg_strptr += strlen(dbg_strptr);                  \
  }

#define PRE_R12_R3(mnemonic)                                           \
  if(bDisassemble)                                                     \
  {                                                                    \
    DEBUG_XX;                                                          \
    sprintf(dbg_strptr, #mnemonic " r%d, ", REG_1 & 31);               \
    dbg_strptr += strlen(dbg_strptr);                                  \
    if(ins & 0x1000)                                                   \
      sprintf(dbg_strptr, "%02" PRIx64 "H", V_2);                      \
    else                                                               \
      sprintf(dbg_strptr, "r%d", REG_2 & 31);                          \
    dbg_strptr += strlen(dbg_strptr);                                  \
    sprintf(dbg_strptr, ", r%d", REG_3 & 31);                          \
    dbg_strptr += strlen(dbg_strptr);                                  \
    if(!bListing)                                                      \
    {                                                                  \
      sprintf(dbg_strptr, ": (%" PRIx64 ",%" PRIx64 ")", state.r[REG_1], V_2); \
      dbg_strptr += strlen(dbg_strptr);                                \
    }                                                                  \
  }

// Debugging version of the OP macro:
// Execute the DO_<mnemonic> macro for an instruction, along with disassembling

#define NEXT {}

// the instruction if needed.
#define OP(mnemonic, format)       \
  PRE_##format(mnemonic);          \
  if(!bListing)                    \
  {                                \
    DO_##mnemonic;                 \
  } POST_##format;                 \
  handle_debug_string(dbg_string); \
  NEXT;

#define DO_ADDL   RCV = sext_u64_32(RAV + RBV);
#define DO_RdC state.r[REG_1] = state.bIntrFlag ? 1 : 0; \
  state.bIntrFlag = false;
#define DO_XOR    state.r[REG_3] = state.r[REG_1] ^ V_2;
#define DO_ADDL_V                                                               \
  {                                                                             \
    u64 rav = RAV;                                                              \
    u64 rbv = RBV;                                                              \
    RCV = sext_u64_32(rav + rbv);                                               \
                                                                             \
    /* test for integer overflow */                                             \
    if(((~rav ^ rbv) & (rav ^ RCV)) & L_SIGN)                                   \
    {                                                                           \
      ARITH_TRAP_I(TRAP_IOV, RC);                                               \
      printf("ADDL_V %016" PRIx64 " + %016" PRIx64 " = %016" PRIx64 " + TRAP.\n", rav, rbv, \
             RCV);                                                              \
    }                                                                           \
  }

struct SCPU_state state;
bool bDisassemble = true;
bool bListing = false;
char   dbg_string[1000];
char*  dbg_strptr;

/**
 * Set program counter to a certain value.
 **/
inline void set_pc(u64 p_pc)
{
  state.pc = p_pc;
  state.rem_ins_in_page = 0;
}

void handle_debug_string(char* s)
{
  if(*s)
    printf("%s\n", s);
}

#define NOT_IMPLEMETED printf("opcode %x is not implemeted.\n", opcode)
#define NOT_IMPLEMETED2 printf("op/fct %x/%x is not implemeted.\n", opcode, function)
#define NOT_IMPLEMETED3 printf("op/fct %x/%04x is not implemeted.\n", opcode, function)

bool do_instr(u32 ins) {
  unsigned int function;
  int opcode = ins >> 26;
  printf("%08x - %02x\n", ins, opcode); 
  dbg_string[0] = '\0';
  dbg_strptr = dbg_string;
  switch (opcode) {
  case 0x10:
    function = (ins >> 5) & 0x7f;
    switch(function)
      {
      case 0x40:  OP(ADDL_V, R12_R3);
	return true;
      case 0x00:  OP(ADDL, R12_R3);
	return true;
      default: NOT_IMPLEMETED2;
      }
    break;
  case 0x11:
    function = (ins >> 5) & 0x7f;
    switch(function)
      {
      case 0x40:  OP(XOR, R12_R3);
	return true;
      default: NOT_IMPLEMETED2;
      }
    break;
   case 0x17:
    function =  (ins & 0x7fff);
    switch(function)
      {
      default: NOT_IMPLEMETED3;
      }
    break;
  case 0x18:
    function =  (ins & 0xffff);
    switch(function)
      {
      case 0xE000:  OP(RdC, X_R1);
	return true;
      default: NOT_IMPLEMETED3;
      }
    break;
  default: NOT_IMPLEMETED;
  } // end switch (opcode)
  return false;
}

int main() {
  dbg_string[0] = '\0';
  dbg_strptr = dbg_string; 


  //struct SCPU_state state;
  state.sde = 1;
  state.pc = 0x12340001;
  state.r[31] = state.r[31+32] = 0;
  state.r[0] = state.r[32] = 0;
  
  u32 ins = 0x06a8a008;
  state.r[8] = state.r[8+32] = 0xffff;
  state.r[21] = state.r[21+32] = 0xaaaa;
  do_instr(0x441f0800);  // XOR r0, r31, r0: (0,0) ==> 0
  //OP(XOR, R12_R3);
  printf("r[12   ]:%08lx\n", state.r[12]);
  printf("r[12+32]:%08lx\n", state.r[12+32]);

  state.r[0] = state.r[32] = 0x44;
  do_instr(0x63ffe000);
  printf("r[0   ]:%08lx\n", state.r[0]);
  printf("r[0+32]:%08lx\n", state.r[32]);
  do_instr(0x5c000480);
  printf("r[0   ]:%08lx\n", state.r[0]);
  printf("r[0+32]:%08lx\n", state.r[32]);

    
  printf("#>%s:\n", dbg_string);
}
