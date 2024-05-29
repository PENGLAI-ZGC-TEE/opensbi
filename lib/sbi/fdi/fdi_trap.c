/* This file handle fdi trap */

#include <sbi/sbi_unpriv.h>
#include <fdi/fdi_csr.h>
#include <fdi/fdi_trap.h>
#include <sm/sm.h>
#include <sm/platform/pmp/enclave_mm.h>
#include <sm/enclave.h>


void set_fdi_from_enclave_to_host(struct enclave_t * enclave)
{
    if (enclave->fdi_enable) // Save fdi state and clear 
    {
        enclave->fdi_state.fdi_dumcfg = csr_read(CSR_DUMCFG);
        enclave->fdi_state.fdi_dumboundlo = csr_read(CSR_DUMBOUNDLO);
        enclave->fdi_state.fdi_dumboundhi = csr_read(CSR_DUMBOUNDHI);

        enclave->fdi_state.fdi_dlcfg0 = csr_read(CSR_DLCFG0);

        enclave->fdi_state.fdi_dlbound0lo = csr_read(CSR_DLBOUND0LO);
        enclave->fdi_state.fdi_dlbound0hi = csr_read(CSR_DLBOUND0HI);
        enclave->fdi_state.fdi_dlbound1lo = csr_read(CSR_DLBOUND1LO);
        enclave->fdi_state.fdi_dlbound1hi = csr_read(CSR_DLBOUND1HI);
        enclave->fdi_state.fdi_dlbound2lo = csr_read(CSR_DLBOUND2LO);
        enclave->fdi_state.fdi_dlbound2hi = csr_read(CSR_DLBOUND2HI);
        enclave->fdi_state.fdi_dlbound3lo = csr_read(CSR_DLBOUND3LO);
        enclave->fdi_state.fdi_dlbound3hi = csr_read(CSR_DLBOUND3HI);

        enclave->fdi_state.fdi_djcfg = csr_read(CSR_DJCFG);

        enclave->fdi_state.fdi_djbound0lo = csr_read(CSR_DJBOUND0LO);
        enclave->fdi_state.fdi_djbound0hi = csr_read(CSR_DJBOUND0HI);
    
        csr_write(CSR_DUMCFG, 0);
        csr_write(CSR_DUMBOUNDLO, 0);
        csr_write(CSR_DUMBOUNDHI, 0);

        csr_write(CSR_DLCFG0, 0);
        csr_write(CSR_DLBOUND0LO, 0);
        csr_write(CSR_DLBOUND0HI, 0);
        csr_write(CSR_DLBOUND1LO, 0);
        csr_write(CSR_DLBOUND1HI, 0);
        csr_write(CSR_DLBOUND2LO, 0);
        csr_write(CSR_DLBOUND2HI, 0);
        csr_write(CSR_DLBOUND3LO, 0);
        csr_write(CSR_DLBOUND3HI, 0);

        csr_write(CSR_DJCFG, 0);
        csr_write(CSR_DJBOUND0LO, 0);
        csr_write(CSR_DJBOUND0HI, 0);
    }
}

void set_fdi_from_host_to_enclave(struct enclave_t * enclave)
{
    if (enclave->fdi_enable) // Restore fdi
    {
        csr_write(CSR_DUMCFG, enclave->fdi_state.fdi_dumcfg);
        csr_write(CSR_DUMBOUNDLO, enclave->fdi_state.fdi_dumboundlo);
        csr_write(CSR_DUMBOUNDHI, enclave->fdi_state.fdi_dumboundhi);

        csr_write(CSR_DLCFG0, enclave->fdi_state.fdi_dlcfg0);
        csr_write(CSR_DLBOUND0LO, enclave->fdi_state.fdi_dlbound0lo);
        csr_write(CSR_DLBOUND0HI, enclave->fdi_state.fdi_dlbound0hi);
        csr_write(CSR_DLBOUND1LO, enclave->fdi_state.fdi_dlbound1lo);
        csr_write(CSR_DLBOUND1HI, enclave->fdi_state.fdi_dlbound1hi);
        csr_write(CSR_DLBOUND2LO, enclave->fdi_state.fdi_dlbound2lo);
        csr_write(CSR_DLBOUND2HI, enclave->fdi_state.fdi_dlbound2hi);
        csr_write(CSR_DLBOUND3LO, enclave->fdi_state.fdi_dlbound3lo);
        csr_write(CSR_DLBOUND3HI, enclave->fdi_state.fdi_dlbound3hi);

        csr_write(CSR_DJCFG, enclave->fdi_state.fdi_djcfg);
        csr_write(CSR_DJBOUND0LO, enclave->fdi_state.fdi_djbound0lo);
        csr_write(CSR_DJBOUND0HI, enclave->fdi_state.fdi_djbound0hi);
    }
}


uintptr_t handle_fdi_trap(struct sbi_trap_regs *regs, uintptr_t mcause, uintptr_t mtval)
{

    // If compressed, mepc += 4
    __attribute__ ((__unused__)) ulong inst;
    __attribute__ ((__unused__)) uintptr_t ret = 0;
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
    // We can exit direct.
    ret = sm_exit_enclave((uintptr_t *)regs, 0x1);

    regs->a0 = 0;
    regs->a1 = 0;

    regs->mepc += strid;   
    return 0;
}
