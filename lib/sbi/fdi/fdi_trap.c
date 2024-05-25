/* This file handle fdi trap */

#include <fdi/fdi_csr.h>
#include <fdi/fdi_trap.h>


uintptr_t handle_fdi_trap(struct sbi_trap_regs *regs, uintptr_t mcause, uintptr_t mtval)
{
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


    regs->mepc += 4;

    return 0;
}
