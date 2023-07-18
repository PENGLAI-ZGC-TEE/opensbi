#ifndef __SPMP_XS_ENCLAVE_MM__
#define __SPMP_XS_ENCLAVE_MM__

#include <sm/platform/pmp/enclave_mm.h>

#define N_SPMP_REGIONS 16
#define REGION_TO_SPMP(region_idx)   (region_idx - N_PMP_REGIONS)
#define SPMP_TO_REGION(spmp_idx)   (spmp_idx + N_PMP_REGIONS)

extern struct mm_region_t mm_regions[N_PMP_REGIONS + N_SPMP_REGIONS];

uintptr_t mm_init_with_spmp(uintptr_t paddr, unsigned long size);

int grant_spmp_enclave_access(struct enclave_t* enclave);

int retrieve_spmp_enclave_access(struct enclave_t *enclave);

#endif