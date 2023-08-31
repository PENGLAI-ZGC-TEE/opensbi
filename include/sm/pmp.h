#ifndef _PMP_H
#define _PMP_H

#include <stdint.h>
#include <sbi/sbi_types.h>
#include <sbi/riscv_encoding.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_hartmask.h>

//number of PMP registers
#define NPMP 16

#define PMP_OFF   0x00
#define PMP_NO_PERM  0

//pmpfcg register's structure
//|63     56|55     48|47     40|39     32|31     24|23     16|15      8|7       0|
//| pmp7cfg | pmp6cfg | pmp5cfg | pmp4cfg | pmp3cfg | pmp2cfg | pmp1cfg | pmp1cfg |
#define PMP_PER_CFG_REG           8
#define PMPCFG_BIT_NUM            8
#define PMPCFG_BITS               0xFF

#if __riscv_xlen == 64
# define LIST_OF_PMP_REGS  X(0,0)  X(1,0)  X(2,0)  X(3,0) \
                           X(4,0)  X(5,0)  X(6,0)  X(7,0) \
                           X(8,2)  X(9,2)  X(10,2) X(11,2) \
                          X(12,2) X(13,2) X(14,2) X(15,2)
# define PMP_PER_GROUP  8
#else
# define LIST_OF_PMP_REGS  X(0,0)  X(1,0)  X(2,0)  X(3,0) \
                           X(4,1)  X(5,1)  X(6,1)  X(7,1) \
                           X(8,2)  X(9,2)  X(10,2) X(11,2) \
                           X(12,3) X(13,3) X(14,3) X(15,3)
# define PMP_PER_GROUP  4
#endif

#define PMP_UNSET(num, cfg_index) do {\
  uintptr_t pmpcfg = csr_read(CSR_PMPCFG##cfg_index); \
  pmpcfg &= ~((uintptr_t)0xff << (uintptr_t)PMPCFG_BIT_NUM*(num%PMP_PER_CFG_REG)); \
  asm volatile ("la t0, 1f \n\t" \
                "csrrw t0, mtvec, t0 \n\t" \
                "csrw pmpaddr"#num", %0\n\t" \
                "csrw pmpcfg"#cfg_index", %1\n\t" \
                "sfence.vma\n\t"\
                ".align 2\n\t" \
                "1: csrw mtvec, t0" \
                : : "r" (0), "r" (pmpcfg) : "t0"); \
} while(0)

#define PMP_SET(num, cfg_index, pmpaddr, pmpcfg) do { \
  uintptr_t oldcfg = csr_read(CSR_PMPCFG##cfg_index); \
  pmpcfg |= (oldcfg & ~((uintptr_t)PMPCFG_BITS << (uintptr_t)PMPCFG_BIT_NUM*(num%PMP_PER_CFG_REG))); \
  asm volatile ("la t0, 1f\n\t" \
                "csrrw t0, mtvec, t0\n\t" \
                "csrw pmpaddr"#num", %0\n\t" \
                "csrw pmpcfg"#cfg_index", %1\n\t" \
                "sfence.vma\n\t"\
                ".align 2\n\t" \
                "1: csrw mtvec, t0 \n\t" \
                : : "r" (pmpaddr), "r" (pmpcfg) : "t0"); \
} while(0)

#define PMP_READ(num, cfg_index, pmpaddr, pmpcfg) do { \
  asm volatile("csrr %0, pmpaddr"#num : "=r"(pmpaddr) :); \
  asm volatile("csrr %0, pmpcfg"#cfg_index : "=r"(pmpcfg) :); \
} while(0)

#define PMP_ERROR(error, msg) {\
  sbi_printf("%s:" msg "\n", __func__);\
  return error; \
}

struct pmp_config_t
{
  uintptr_t paddr;
  unsigned long size;
  uintptr_t perm;
  uintptr_t mode;
};

struct pmp_data_t
{
  struct pmp_config_t pmp_config_arg;
  int pmp_idx_arg;
  struct sbi_hartmask smask;
};

#define SBI_PMP_DATA_INIT(__ptr, __pmp_config_arg, __pmp_idx_arg, __src) \
do { \
	(__ptr)->pmp_config_arg = (__pmp_config_arg); \
	(__ptr)->pmp_idx_arg = (__pmp_idx_arg); \
	SBI_HARTMASK_INIT_EXCEPT(&(__ptr)->smask, (__src)); \
} while (0)

void set_pmp_and_sync(int pmp_idx, struct pmp_config_t);
void clear_pmp_and_sync(int pmp_idx);
void set_pmp(int pmp_idx, struct pmp_config_t);
void clear_pmp(int pmp_idx);
struct pmp_config_t get_pmp(int pmp_idx);
void dump_pmps(void);

#endif /* _PMP_H */
