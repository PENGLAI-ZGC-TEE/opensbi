/* FDI TRAP HANDLE */

#ifndef __INCLUDE_FDI_FDI_TRAP_H__
#define __INCLUDE_FDI_FDI_TRAP_H__

#include <sm/print.h>
#include <sm/platform/pmp/platform.h>
#include <stdint.h>
#include <sm/enclave_args.h>
#include <sbi/sbi_trap.h>



uintptr_t handle_fdi_trap(struct sbi_trap_regs *regs, uintptr_t mcause, uintptr_t mtval);




#endif