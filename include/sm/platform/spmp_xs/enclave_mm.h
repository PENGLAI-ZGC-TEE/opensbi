#ifndef __SPMP_XS_ENCLAVE_MM__
#define __SPMP_XS_ENCLAVE_MM__

#include <sm/platform/pmp/enclave_mm.h>

#define N_SPMP_REGIONS 16

uintptr_t mm_init_with_spmp(uintptr_t paddr, unsigned long size);

int grant_spmp_enclave_access(struct enclave_t* enclave);

int retrieve_spmp_enclave_access(struct enclave_t *enclave);

#endif