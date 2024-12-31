#ifndef ITRANSLATIONBUFFER_H
#define ITRANSLATIONBUFFER_H

#include "StdAfx.h"
#include "cpu_defs.h"
class CSystemComponent;
class SCPU_state;

class ITranslationBuffer
{
public:
    virtual ~ITranslationBuffer() {}
    virtual int FindTBEntry(u64 virt, int flags, const SCPU_state &state) = 0;
    virtual void add_tb(u64 virt, u64 pte_phys, u64 pte_flags, int flags,
                        const SCPU_state &state) = 0;
    virtual void add_tb_d(u64 virt, u64 pte, const SCPU_state &state) = 0;
    virtual void add_tb_i(u64 virt, u64 pte, const SCPU_state &state) = 0;
    virtual void tbia(int flags, const SCPU_state &state) = 0;
    virtual void tbiap(int flags, const SCPU_state &state) = 0;
    virtual void tbis(u64 virt, int flags, const SCPU_state &state) = 0;
    virtual int virt2phys(u64 virt, u64* phys, int flags, bool* asm_bit, u32 ins,
                          const SCPU_state &state) = 0;
};

#endif
