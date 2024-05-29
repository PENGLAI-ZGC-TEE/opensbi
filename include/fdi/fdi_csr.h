/* FDI CSR DEFINE */

#ifndef __INCLUDE_FDI_FDI_CSR_H__
#define __INCLUDE_FDI_FDI_CSR_H__


/* DASICS csrs */
#define CSR_DUMCFG          0x9e0
#define CSR_DUMBOUNDLO      0x9e2
#define CSR_DUMBOUNDHI      0x9e3

/* DASICS Main cfg */
#define DASICS_MAINCFG_MASK 0xfUL
#define DASICS_UCFG_CLS     0x8UL
#define DASICS_SCFG_CLS     0x4UL
#define DASICS_UCFG_ENA     0x2UL
#define DASICS_SCFG_ENA     0x1UL

/* DASICS Lib csrs */
#define CSR_DLCFG0          0x880

#define CSR_DLBOUND0LO      0x890
#define CSR_DLBOUND0HI      0x891
#define CSR_DLBOUND1LO      0x892
#define CSR_DLBOUND1HI      0x893
#define CSR_DLBOUND2LO      0x894
#define CSR_DLBOUND2HI      0x895
#define CSR_DLBOUND3LO      0x896
#define CSR_DLBOUND3HI      0x897



#define CSR_DMAINCALL       0x8b0
#define CSR_DRETURNPC       0x8b1


#define CSR_DJBOUND0LO      0x8c0
#define CSR_DJBOUND0HI      0x8c1


#define CSR_DJCFG           0x8c8


/* DASICS Lib cfg */
#define DASICS_LIBCFG_WIDTH 4
#define DASICS_LIBCFG_MASK  0xfUL
#define DASICS_LIBCFG_V     0x8UL
#define DASICS_LIBCFG_R     0x2UL
#define DASICS_LIBCFG_W     0x1UL

#define DASICS_JUMPCFG_WIDTH 	1
#define DASICS_JUMPCFG_MASK 	0xffffUL
#define DASICS_JUMPCFG_V    	0x1UL


struct fdi_state_t
{
    unsigned long fdi_dumcfg;
    unsigned long fdi_dumboundlo;
    unsigned long fdi_dumboundhi;

    unsigned long fdi_dlcfg0;

    unsigned long fdi_dlbound0lo;
    unsigned long fdi_dlbound0hi;
    unsigned long fdi_dlbound1lo;
    unsigned long fdi_dlbound1hi;
    unsigned long fdi_dlbound2lo;
    unsigned long fdi_dlbound2hi;
    unsigned long fdi_dlbound3lo;    
    unsigned long fdi_dlbound3hi;    

    unsigned long fdi_djcfg;
    unsigned long fdi_djbound0lo;
    unsigned long fdi_djbound0hi;

};

struct enclave_t;

void set_fdi_from_enclave_to_host(struct enclave_t * enclave);
void set_fdi_from_host_to_enclave(struct enclave_t * enclave);




#endif