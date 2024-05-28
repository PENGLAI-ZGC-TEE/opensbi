/* This file handle fdi trap */

#include <sbi/sbi_unpriv.h>
#include <fdi/fdi_csr.h>
#include <fdi/fdi_trap.h>
#include <sm/platform/pmp/enclave_mm.h>



uintptr_t handle_fdi_trap(struct sbi_trap_regs *regs, uintptr_t mcause, uintptr_t mtval)
{

    // If compressed, mepc += 4
    __attribute__ ((__unused__)) ulong inst;
    int strid = 4;
    
    if ((regs->mepc & 0x3ul) != 0) // This is a compressed instruction
    {
        strid = 2;
        goto deal_fdi;  
    }

	struct sbi_trap_info trap;
    // judge inst
    inst = sbi_get_insn(regs->mepc, &trap);
    if (inst)
    {
        //If the last 2 bit of 32bit are both 1, This instruction will be a 32bit
        if ((inst & 0x3ul) != 0x3ul)
        {   
            strid = 2;
        }
    } else 
    {
        sbi_printf("\x1b[31m [FDI_Opensbi]@%s: Try to get pc from 0x%lx failed, default mepc += 4 \x1b[0m\n", __func__, regs->mepc);        
    }

deal_fdi:
    switch (mcause)
    {
    case CAUSE_FDI_FETCH_FAULT:
        /* code */
        sbi_printf("\x1b[31m [FDI_Opensbi]@%s: Handle FDI_FETCH_FAULT in pc: 0x%lx, mtval: 0x%lx \x1b[0m\n", __func__, regs->mepc, mtval);
        break;
    case CAUSE_FDI_LOAD_ACCESS_FAULT:
        /* code */
        sbi_printf("\x1b[31m [FDI_Opensbi]@%s: FDI_LOAD_ACCESS_FAULT in pc: 0x%lx, mtval: 0x%lx \x1b[0m\n", __func__,  regs->mepc, mtval);
        break;
    case CAUSE_FDI_STORE_ACCESS_FAULT:
        /* code */
        sbi_printf("\x1b[31m [FDI_Opensbi]@%s: FDI_STORE_ACCESS_FAULT in pc: 0x%lx, mtval: 0x%lx \x1b[0m\n", __func__,  regs->mepc, mtval);
        break;        
    default:
        break;
    }


    regs->mepc += strid;

    return 0;
}
