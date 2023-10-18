#include "enclave_mm.c"
#include "platform_thread.c"

#include <sm/print.h>
#include <sm/platform/spmp/spmp.h>

static unsigned long form_pmp_size(unsigned long pmp_size) {
  int i;
  // make lower bits all ones
  // 0x00ee --> 0x00ff
  for (i = 1; i <= 32; i = i << 1)
	  pmp_size |= (pmp_size >> i);
  // return (0x00ff + 1) = 0x0100
  return (pmp_size + 1);
}

int platform_init()
{
  struct pmp_config_t pmp_config;
  u32 hartid = current_hartid();
  printm_err("***hart id = %u ***\n", hartid);

  //Clear pmp1, this pmp is reserved for allowing kernel
  //to config page table for enclave in enclave's memory.
  //There is no need to broadcast to other hart as every
  //hart will execute this function.
  //clear_pmp(1);
  clear_pmp_and_sync(1);

  //config the PMP 0 to protect security monitor
  pmp_config.paddr = (uintptr_t)SM_BASE;
  pmp_config.size = form_pmp_size((unsigned long)SM_SIZE);
  pmp_config.mode = PMP_A_NAPOT;
  pmp_config.perm = PMP_NO_PERM;
  set_pmp_and_sync(0, pmp_config);

  //config the last PMP to allow kernel to access memory
  pmp_config.paddr = 0;
  pmp_config.size = -1UL;
  pmp_config.mode = PMP_A_NAPOT;
  pmp_config.perm = PMP_R | PMP_W | PMP_X;
  //set_pmp(NPMP-1, pmp_config);
  set_pmp_and_sync(NPMP-1, pmp_config);
  printm_err("******* set_pmp  paddr: 0x%lx, size: 0x%lx, mode: 0x%lx, perm: 0x%lx*******\n", pmp_config.paddr, pmp_config.size, pmp_config.mode, pmp_config.perm);
  
#if 0
  pmp_config = get_pmp(NPMP-1);
  printm_err("******* get_pmp  paddr: 0x%lx, size: 0x%lx, mode: 0x%lx, perm: 0x%lx*******\n", pmp_config.paddr, pmp_config.size, pmp_config.mode, pmp_config.perm);
#endif

#if 1
//config the last sPMP (SRWX=1000) to allow user to access memory 
  struct spmp_config_t spmp_config;
  spmp_config.paddr = 0;
  spmp_config.size = -1UL;
  spmp_config.mode = SPMP_NAPOT;
  spmp_config.perm = SPMP_NO_PERM;
  spmp_config.sbit = SPMP_S;
  set_spmp(NSPMP-1, spmp_config);
  //set_spmp_and_sync(NSPMP-1, spmp_config);

  dump_spmps();
  
  printm_err("\n******* set_spmp paddr: 0x%lx, size: 0x%lx, mode: 0x%lx, perm: 0x%lx, sbit: 0x%lx*******\n", spmp_config.paddr, spmp_config.size, spmp_config.mode, spmp_config.perm, spmp_config.sbit);

  spmp_config = get_spmp(NSPMP-1);
  printm_err("******* get_spmp paddr: 0x%lx, size: 0x%lx, mode: 0x%lx, perm: 0x%lx, sbit: 0x%lx*******\n", spmp_config.paddr, spmp_config.size, spmp_config.mode, spmp_config.perm, spmp_config.sbit);
  printm_err("\n");
#endif
  printm("[Penglai Monitor@%s] setting initial PMP ready\n", __func__);
  return 0;
}
