#include <sm/sm.h>
#include <sm/enclave.h>
#include <sm/platform/pmp/enclave_mm.h>
#include <sm/platform/spmp_xs/enclave_mm.h>
#include <sm/platform/spmp_xs/spmp.h>
#include <sbi/riscv_atomic.h>
#include <sbi/riscv_locks.h>
#include <sm/math.h>
#include <sbi/sbi_string.h>

/* debug region */
extern struct mm_region_t mm_regions[N_PMP_REGIONS + N_SPMP_REGIONS];
static unsigned long spmp_bitmap = 0;
static spinlock_t spmp_bitmap_lock = SPIN_LOCK_INITIALIZER;

uintptr_t mm_init_with_spmp(uintptr_t paddr, unsigned long size)
{
	uintptr_t retval = 0;
	int region_idx = 0;
	int spmp_idx =0;
	struct spmp_config_t spmp_config;

	// check align of paddr and size
	if(check_mem_size(paddr, size) < 0)
		return -1UL;

	// acquire a free enclave region
	spin_lock(&spmp_bitmap_lock);

	// check memory overlap
	// memory overlap should be checked after acquire lock
	if(check_mem_overlap(paddr, size) < 0)
	{
		retval = -1UL;
		goto out;
	}

	// alloc a free spmp
	for(region_idx = N_PMP_REGIONS; region_idx < (N_PMP_REGIONS + N_SPMP_REGIONS); ++region_idx)
	{
		printm_err("regiong idx:%d", region_idx);
		spmp_idx = REGION_TO_SPMP(region_idx);
		if(!(spmp_bitmap & (1 << spmp_idx)))
		{
			//FIXME: we already have mm_regions[x].valid, why pmp_bitmap again
			spmp_bitmap |= (1 << spmp_idx);
			break;
		}
	}
	if(region_idx >= N_PMP_REGIONS + N_SPMP_REGIONS)
	{
		retval = -1UL;
		goto out;
	}

	// set sPMP to protect enclave memory region
	spmp_config.paddr = paddr;
	spmp_config.size = size;
	spmp_config.perm = SPMP_NO_PERM;
	spmp_config.mode = SPMP_NAPOT;
	set_spmp(spmp_idx, spmp_config);

	printm("The region idx is %d.", region_idx);
	printm("The region idx valid before is %d.", mm_regions[region_idx].valid);
	printm("The mm_list_head enclave_class before is %p.", mm_regions[region_idx].mm_list_head);
	// mark this region is valid and init mm_list
	mm_regions[region_idx].valid = 1;
	mm_regions[region_idx].paddr = paddr;
	mm_regions[region_idx].size = size;
	mm_regions[region_idx].enclave_class = SPMP_REGION;
	struct mm_list_t *mm_list = (struct mm_list_t *)PADDR_2_MM_LIST(paddr);
	mm_list->order = ilog2(size-1) + 1;
	mm_list->prev_mm = NULL;
	mm_list->next_mm = NULL;
	struct mm_list_head_t *mm_list_head = (struct mm_list_head_t*)paddr;
	mm_list_head->order = mm_list->order;
	mm_list_head->prev_list_head = NULL;
	mm_list_head->next_list_head = NULL;
	mm_list_head->mm_list = mm_list;
	mm_regions[region_idx].mm_list_head = mm_list_head;
	printm("The region idx valid after is %d.", mm_regions[region_idx].valid);
	printm("The mm_list_head enclave_class after is %p.", mm_regions[region_idx].mm_list_head);
	
out:
	spin_unlock(&spmp_bitmap_lock);
	return retval;
}

int grant_spmp_enclave_access(struct enclave_t* enclave)
{
	int region_idx = 0;
	int spmp_idx = 0;
	struct spmp_config_t spmp_config;

	if(check_mem_size(enclave->paddr, enclave->size) < 0)
		return -1;

	//set spmp permission, ensure that enclave's paddr and size is pmp legal
	//TODO: support multiple memory regions
	spin_lock(&spmp_bitmap_lock);
	for(region_idx = N_PMP_REGIONS; region_idx < N_PMP_REGIONS + N_SPMP_REGIONS; ++region_idx)
	{
		printm("enter region idx:%d", region_idx);
		if(mm_regions[region_idx].valid && region_contain(
					mm_regions[region_idx].paddr, mm_regions[region_idx].size,
					enclave->paddr, enclave->size))
		{
			break;
		}
	}
	spin_unlock(&spmp_bitmap_lock);

	if(region_idx >= N_PMP_REGIONS + N_SPMP_REGIONS || region_idx < N_PMP_REGIONS)
	{
		printm_err("SPMP M mode: grant_enclave_access: can not find exact mm_region\r\n");
		return -1;
	}

	spmp_idx = REGION_TO_SPMP(region_idx);
#if 0
	pmp_config.paddr = enclave->paddr;
	pmp_config.size = enclave->size;
#else
	spmp_config.paddr = mm_regions[region_idx].paddr;
	spmp_config.size = mm_regions[region_idx].size;
#endif
	spmp_config.perm = SPMP_R | SPMP_W | SPMP_X;
	spmp_config.mode = SPMP_NAPOT;

	/* Note: here we only set the sPMP regions in local Hart*/
	set_spmp(spmp_idx, spmp_config);

	/*FIXME: we should handle the case that the sPMP region contains larger region */
	if (spmp_config.paddr != enclave->paddr || spmp_config.size != enclave->size){
		printm("[Penglai Monitor@%s] warning, region != enclave mem.", __func__);
		printm("[Penglai Monitor@%s] region: paddr(0x%lx) size(0x%lx).",
				__func__, spmp_config.paddr, spmp_config.size);
		printm("[Penglai Monitor@%s] enclave mem: paddr(0x%lx) size(0x%lx).",
				__func__, enclave->paddr, enclave->size);
	}

	uint64_t sstatus = csr_read(CSR_SSTATUS);
	printm("sstatus after spmp status is:%#lx.", sstatus);

	return 0;
}

int retrieve_spmp_enclave_access(struct enclave_t *enclave)
{
	int region_idx = 0;
	int spmp_idx = 0;
	struct spmp_config_t spmp_config;

	//set spmp permission, ensure that enclave's paddr and size is spmp legal
	//TODO: support multiple memory regions
	spin_lock(&spmp_bitmap_lock);
	for(region_idx = N_PMP_REGIONS; region_idx < N_PMP_REGIONS + N_SPMP_REGIONS; ++region_idx)
	{
		printm_err("back region idx:%d.", region_idx);
		if(mm_regions[region_idx].valid && region_contain(
					mm_regions[region_idx].paddr, mm_regions[region_idx].size,
					enclave->paddr, enclave->size))
		{
			break;
		}
	}
	spin_unlock(&spmp_bitmap_lock);

	if(region_idx >= N_PMP_REGIONS + N_SPMP_REGIONS)
	{
		printm_err("M mode: Error: %s\r\n", __func__);
		/* For Debug */
		for (region_idx = 0; region_idx < N_SPMP_REGIONS; ++region_idx) {
			printm("[Monitor Debug@%s] mm_region[%d], valid(%d), paddr(0x%lx) size(0x%lx)\n",
					__func__, region_idx, mm_regions[region_idx].valid, mm_regions[region_idx].paddr,
					mm_regions[region_idx].size);
		}
		printm("[Monitor Debug@%s] enclave paddr(0x%lx) size(0x%lx)\n",
				__func__, enclave->paddr, enclave->size);

		return -1;
	}

	spmp_idx = REGION_TO_SPMP(region_idx);

	// set PMP to protect the entire PMP region
	spmp_config.paddr = mm_regions[region_idx].paddr;
	spmp_config.size = mm_regions[region_idx].size;
	spmp_config.perm = SPMP_NO_PERM;
	spmp_config.mode = SPMP_NAPOT;

	/* Note: here we only set the PMP regions in local Hart*/
	set_spmp(spmp_idx, spmp_config);

	return 0;
}

// void* mm_alloc_with_spmp(unsigned long req_size, unsigned long *resp_size)
// {
// 	void* ret_addr = NULL;
// 	if(req_size == 0)
// 		return ret_addr;

// 	//TODO: reduce lock granularity
// 	spin_lock(&spmp_bitmap_lock);

// 	//print_buddy_system();

// 	unsigned long order = ilog2(req_size-1) + 1;
// 	/* N_SPMP_REGION should be judged first */
// 	for(int region_idx = N_PMP_REGIONS; region_idx < N_SPMP_REGIONS + N_PMP_REGIONS; ++region_idx)
// 	{
// 		struct mm_list_t* mm_region = alloc_one_region(region_idx, order);

// 		//there is no enough space in current pmp region
// 		if(!mm_region)
// 			continue;

// 		while(mm_region->order > order)
// 		{
// 			printm("Need to alloc new region\n");
// 			//allocated mm region need to be split
// 			mm_region->order -= 1;
// 			mm_region->prev_mm = NULL;
// 			mm_region->next_mm = NULL;

// 			void* new_mm_region_paddr = MM_LIST_2_PADDR(mm_region) + (1 << mm_region->order);
// 			struct mm_list_t* new_mm_region = PADDR_2_MM_LIST(new_mm_region_paddr);
// 			new_mm_region->order = mm_region->order;
// 			new_mm_region->prev_mm = NULL;
// 			new_mm_region->next_mm = NULL;
// 			insert_mm_region(region_idx, new_mm_region, 0);
// 		}

// 		ret_addr = MM_LIST_2_PADDR(mm_region);
// 		break;
// 	}

// 	//print_buddy_system();

// 	spin_unlock(&spmp_bitmap_lock);

// 	if(ret_addr && resp_size)
// 	{
// 		*resp_size = 1 << order;
// 		sbi_memset(ret_addr, 0, *resp_size);
// 	}

// 	return ret_addr;
// }