//#include <sm/atomic.h>
#include <sbi/riscv_atomic.h>
#include <sbi/riscv_locks.h>
#include <sm/sm.h>
#include <sm/pmp.h>
#include <sm/enclave.h>
#include <sm/attest.h>
#include <sm/math.h>
#include <sbi/sbi_console.h>
#include <sm/page_map.h>
#include <sm/platform/pmp/enclave_mm.h>

//static int sm_initialized = 0;
//static spinlock_t sm_init_lock = SPINLOCK_INIT;

static spinlock_t shm_idx_lock = SPIN_LOCK_INITIALIZER;
static spinlock_t shm_eid_idx_lock = SPIN_LOCK_INITIALIZER;
static spinlock_t shm_ownership_lock = SPIN_LOCK_INITIALIZER;
// static spinlock_t clock_lock = SPIN_LOCK_INITIALIZER;

static unsigned long shm_idx = 0;
static unsigned long shm_eid_idx = 0;
struct enclave_shm_t enclave_shm[NUM_SHM];


void sm_init()
{
  platform_init();
  attest_init();
}

uintptr_t sm_mm_init(uintptr_t paddr, unsigned long size)
{
  uintptr_t retval = 0;

  printm("[Penglai Monitor] %s invoked\r\n",__func__);

  printm("[Penglai Monitor] %s paddr:0x%lx, size:0x%lx\r\n",__func__, paddr, size);
  /*DEBUG: Dump PMP registers here */
  dump_pmps();
  retval = mm_init(paddr, size);
  /*DEBUG: Dump PMP registers here */
  dump_pmps();

  printm("[Penglai Monitor] %s ret:%ld \r\n",__func__, retval);
  return retval;
}

uintptr_t sm_mm_extend(uintptr_t paddr, unsigned long size)
{
  uintptr_t retval = 0;
  printm("[Penglai Monitor] %s invoked\r\n",__func__);

  retval = mm_init(paddr, size);

  printm("[Penglai Monitor] %s return:%ld\r\n",__func__, retval);
  return retval;
}

uintptr_t sm_debug_print(uintptr_t* regs, uintptr_t arg0)
{
  print_buddy_system();
  return 0;
}

uintptr_t sm_alloc_enclave_mem(uintptr_t mm_alloc_arg)
{
  struct mm_alloc_arg_t mm_alloc_arg_local;
  uintptr_t retval = 0;

  printm("[Penglai Monitor] %s invoked\r\n",__func__);

  retval = copy_from_host(&mm_alloc_arg_local,
      (struct mm_alloc_arg_t*)mm_alloc_arg,
      sizeof(struct mm_alloc_arg_t));
  if(retval != 0)
  {
    printm_err("M mode: sm_alloc_enclave_mem: unknown error happended when copy from host\r\n");
    return ENCLAVE_ERROR;
  }

  dump_pmps();
  unsigned long resp_size = 0;
  void* paddr = mm_alloc(mm_alloc_arg_local.req_size, &resp_size);
  if(paddr == NULL)
  {
    printm("M mode: sm_alloc_enclave_mem: no enough memory\r\n");
    return ENCLAVE_NO_MEMORY;
  }
  dump_pmps();

  //grant kernel access to this memory
  if(grant_kernel_access(paddr, resp_size) != 0)
  {
    printm_err("M mode: ERROR: faile to grant kernel access to pa 0x%lx, size 0x%lx\r\n", (unsigned long) paddr, resp_size);
    mm_free(paddr, resp_size);
    return ENCLAVE_ERROR;
  }

  mm_alloc_arg_local.resp_addr = (uintptr_t)paddr;
  mm_alloc_arg_local.resp_size = resp_size;

  retval = copy_to_host((struct mm_alloc_arg_t*)mm_alloc_arg,
      &mm_alloc_arg_local,
      sizeof(struct mm_alloc_arg_t));
  if(retval != 0)
  {
    printm_err("M mode: sm_alloc_enclave_mem: unknown error happended when copy to host\r\n");
    return ENCLAVE_ERROR;
  }

  printm("[Penglai Monitor] %s return:%ld\r\n",__func__, retval);

  return ENCLAVE_SUCCESS;
}

uintptr_t sm_create_enclave(uintptr_t enclave_sbi_param)
{
  struct enclave_sbi_param_t enclave_sbi_param_local;
  uintptr_t retval = 0;

  printm("[Penglai Monitor] %s invoked\r\n",__func__);

  retval = copy_from_host(&enclave_sbi_param_local,
      (struct enclave_sbi_param_t*)enclave_sbi_param,
      sizeof(struct enclave_sbi_param_t));
  if(retval != 0)
  {
    printm_err("M mode: sm_create_enclave: unknown error happended when copy from host\r\n");
    return ENCLAVE_ERROR;
  }

  void* paddr = (void*)enclave_sbi_param_local.paddr;
  unsigned long size = (unsigned long)enclave_sbi_param_local.size;
  if(retrieve_kernel_access(paddr, size) != 0)
  {
    mm_free(paddr, size);
    return -1UL;
  }

  retval = create_enclave(enclave_sbi_param_local);

  printm("[Penglai Monitor] %s created return value:%ld \r\n",__func__, retval);
  return retval;
}

uintptr_t sm_attest_enclave(uintptr_t eid, uintptr_t report, uintptr_t nonce)
{
  uintptr_t retval;
  printm("[Penglai Monitor] %s invoked, eid:%ld\r\n",__func__, eid);

  retval = attest_enclave(eid, report, nonce);

  printm("[Penglai Monitor] %s return: %ld\r\n",__func__, retval);

  return retval;
}

uintptr_t sm_run_enclave(uintptr_t* regs, unsigned long eid)
{
  uintptr_t retval;
  printm("[Penglai Monitor] %s invoked, eid:%ld\r\n",__func__, eid);

  retval = run_enclave(regs, (unsigned int)eid);

  printm("[Penglai Monitor] %s return: %ld\r\n",__func__, retval);

  return retval;
}

uintptr_t sm_stop_enclave(uintptr_t* regs, unsigned long eid)
{
  uintptr_t retval;
  printm("[Penglai Monitor] %s invoked, eid:%ld\r\n",__func__, eid);

  retval = stop_enclave(regs, (unsigned int)eid);

  printm("[Penglai Monitor] %s return: %ld\r\n",__func__, retval);
  return retval;
}

uintptr_t sm_resume_enclave(uintptr_t* regs, unsigned long eid)
{
  uintptr_t retval = 0;
  uintptr_t resume_func_id = regs[11];

  switch(resume_func_id)
  {
    case RESUME_FROM_TIMER_IRQ:
      retval = resume_enclave(regs, eid);
      break;
    case RESUME_FROM_STOP:
      retval = resume_from_stop(regs, eid);
      break;
    case RESUME_FROM_OCALL:
      retval = resume_from_ocall(regs, eid);
      break;
    default:
      break;
  }

  return retval;
}

uintptr_t sm_exit_enclave(uintptr_t* regs, unsigned long retval)
{
  uintptr_t ret;
  printm("[Penglai Monitor] %s invoked\r\n",__func__);

  ret = exit_enclave(regs, retval);

  printm("[Penglai Monitor] %s return: %ld\r\n",__func__, ret);

  return ret;
}

uintptr_t sm_enclave_ocall(uintptr_t* regs, uintptr_t ocall_id, uintptr_t arg0, uintptr_t arg1)
{
  uintptr_t ret = 0;
  switch(ocall_id)
  {
    case OCALL_SYS_WRITE:
      ret = enclave_sys_write(regs);
      break;
    case OCALL_USER_DEFINED:
      ret = enclave_user_defined_ocall(regs, arg0);
      break;
    default:
      printm_err("[Penglai Monitor@%s] wrong ocall_id(%ld)\r\n", __func__, ocall_id);
      ret = -1UL;
      break;
  }
  return ret;
}

/**
 * \brief Retrun key to enclave.
 * 
 * \param regs          The enclave regs.
 * \param salt_va       Salt pointer in enclave address space.
 * \param salt_len      Salt length in bytes.
 * \param key_buf_va    Key buffer pointer in enclave address space.
 * \param key_buf_len   Key buffer length in bytes.
 */
uintptr_t sm_enclave_get_key(uintptr_t* regs, uintptr_t salt_va, uintptr_t salt_len,
    uintptr_t key_buf_va, uintptr_t key_buf_len)
{
  uintptr_t ret = 0;

  ret = enclave_derive_seal_key(regs, salt_va, salt_len, key_buf_va, key_buf_len);

  return ret;
}

/**
 * \brief This transitional function is used to destroy the enclave.
 *
 * \param regs The host reg.
 * \param enclave_eid The enclave id.
 */
uintptr_t sm_destroy_enclave(uintptr_t *regs, uintptr_t enclave_id)
{
  uintptr_t ret = 0;
  printm("[Penglai Monitor] %s invoked\r\n",__func__);

  ret = destroy_enclave(regs, enclave_id);

  printm("[Penglai Monitor] %s return: %ld\r\n",__func__, ret);

  return ret;
}

uintptr_t sm_do_timer_irq(uintptr_t *regs, uintptr_t mcause, uintptr_t mepc)
{
  uintptr_t ret;

  ret = do_timer_irq(regs, mcause, mepc);

  regs[10] = 0; //no errors in all cases for timer handler
  regs[11] = ret; //value
  return ret;
}


int32_t sm_create_shm(uint64_t key, uint64_t req_size){
  printm("[sm.c@%s] ----------sm create shm start---------\n", __func__);
  unsigned long resp_size = 0;
  printm("[sm.c@%s] req mem size is %ld.\n", __func__, (long int)req_size);
  // void* paddr = mm_alloc(req_size, &resp_size);
  void* paddr = NULL;
	struct pmp_config_t pmp_config = get_pmp(2);
	paddr = (void*)pmp_config.paddr;
	resp_size = pmp_config.size;
	pmp_config.perm = PMP_W | PMP_R;
	// pmp_config.mode = PMP_A_NAPOT;
	set_pmp_and_sync(2, pmp_config);
  if(paddr == NULL)
  {
    printm("[sm.c@%s] no enough memory to create share memory.\r\n", __func__);
    return -1;  // 返回值为-1，表示未成功分配share memory
  }
  printm("[sm.c@%s] shm paddr = 0x%lx, alloc mem size is %ld. \n", __func__, (unsigned long)paddr, (long int)resp_size);


  int eid = -1;
  eid = get_enclave_id();
  if (eid == -1){
    printm("[sm.c@%s] get_enclave_id failed! \n", __func__);
    return -1;
  }
  // else {
  //   printm("[sm.c@%s] get_enclave_id succeed! eid is %d .\n", __func__, eid);
  // }

  u8 pt_perm = PTE_R | PTE_W | PTE_U;

  struct enclave_t* enclave;
  enclave =  get_enclave(eid);
  enclave->pt_perm = pt_perm;
  
  unsigned long shmid = -1;

  uint32_t enclave_type = key & ENCLAVE_TYPE_MASK;
  uint64_t shm_key = (key & SHM_KEY_MASK) >> SHM_KEY_SHIFT;

  spin_lock(&shm_idx_lock);
  for (shm_idx = 0; shm_idx < NUM_SHM; shm_idx++){
    if (!enclave_shm[shm_idx].used){
      shmid = shm_idx;
      enclave_shm[shm_idx].used = 1;
      enclave_shm[shm_idx].key = shm_key;
      enclave_shm[shm_idx].paddr = (unsigned long)paddr;
      enclave_shm[shm_idx].size = (unsigned long)resp_size;
      enclave_shm[shm_idx].perm = pt_perm;

      //shm的创建者attach到共享内存
      spin_lock(&shm_eid_idx_lock);
      for (shm_eid_idx = 0; shm_eid_idx < NUM_EACH_SHM; shm_eid_idx++){
        if (!enclave_shm[shm_idx].eids[shm_eid_idx].used){
          enclave_shm[shm_idx].eids[shm_eid_idx].used = 1;
          enclave_shm[shm_idx].eids[shm_eid_idx].eid = eid;
          enclave_shm[shm_idx].eids[shm_eid_idx].enclave_type = enclave_type;
          break;
        }
      }
      spin_unlock(&shm_eid_idx_lock);
      break;
    } 
  }
  spin_unlock(&shm_idx_lock);


  return shmid;
}


// 
int32_t sm_map_shm(virtual_addr_t vaddr, uint32_t shmid){
  unsigned long paddr, shm_size;

  spin_lock(&shm_idx_lock);
  shm_idx = shmid;
  if (enclave_shm[shm_idx].used){
    paddr = enclave_shm[shm_idx].paddr;
    shm_size = enclave_shm[shm_idx].size;
  }else {
    printm("[SM@%s] share memory not exist!\n", __func__);
    return -1; // -1 share memory不存在
  }
  spin_unlock(&shm_idx_lock);

  int eid = -1;
  eid = get_enclave_id();
  if (eid == -1){
    printm("[sm.c@%s] get_enclave_id failed! \n", __func__);
    return -2; //-2 is get_enclave_id failed
  } 
  // else {
  //   printm("[sm.c@%s] get_enclave_id succeed! eid is %d.\n", __func__, eid);
  // }

  struct enclave_t* enclave;
  enclave =  get_enclave(eid);

  virtual_addr_t shm_va = enclave->shm_ptr;
  //将物理地址映射至创建者的虚拟地址空间中
  int ret = 0; 
  ret = map_pa2va(enclave, shm_va, (physical_addr_t) paddr, shm_size, enclave->pt_perm);

  uintptr_t shm_pa = get_enclave_paddr_from_va(enclave->root_page_table, shm_va);
 
  // printm("[sm.c@%s] get_enclave_paddr_from_va return shm_pa 0x%lx \n", __func__, (long int)shm_pa);
  if (shm_pa == paddr && ret == 0){
	  // printm("[sm.c@%s] ret shm_va 0x%lx \n", __func__, (long int)shm_va);
    enclave->shm_ptr = (unsigned long)shm_va + shm_size;
    // pa是vaddr指针指向的位置
    unsigned long* pa = (unsigned long*)get_enclave_paddr_from_va(enclave->root_page_table, vaddr);
    *pa = shm_va;
    return 0; // 0 映射成功
  }
  return -3;  // -3 映射失败
}

// 根据key
int32_t sm_get_shmid(uint64_t key){
  // uint32_t enclave_type = key & ENCLAVE_TYPE_MASK;
  uint64_t shm_key = (key & SHM_KEY_MASK) >> SHM_KEY_SHIFT;

  int32_t local_shmid = -1;
  spin_lock(&shm_idx_lock);
  for (shm_idx = 0; shm_idx < NUM_SHM; shm_idx++){
    if (enclave_shm[shm_idx].used && enclave_shm[shm_idx].key == shm_key){
      local_shmid = (int32_t) shm_idx;
      spin_unlock(&shm_idx_lock);
      return local_shmid;
    } 
  }
  spin_unlock(&shm_idx_lock);
  return local_shmid;
}


int32_t sm_attach_shm(uint32_t shmid, uint32_t enclave_type){
  int eid = -1;
  eid = get_enclave_id();
  if (eid == -1){
    printm("[sm.c@%s] get_enclave_id failed! \n", __func__);
    return -1;
  }
  // else {
  //   printm("[sm.c@%s] get_enclave_id succeed! eid is %d .\n", __func__, eid);
  // }

  struct enclave_t* enclave;
  enclave =  get_enclave(eid);


  spin_lock(&shm_idx_lock);
  shm_idx = shmid;
  if (enclave_shm[shm_idx].used){
    spin_lock(&shm_eid_idx_lock);
    for (shm_eid_idx = 0; shm_eid_idx < NUM_EACH_SHM; shm_eid_idx++){
      if (!enclave_shm[shm_idx].eids[shm_eid_idx].used){
        enclave_shm[shm_idx].eids[shm_eid_idx].used = 1;
        enclave_shm[shm_idx].eids[shm_eid_idx].enclave_type = enclave_type;
        enclave_shm[shm_idx].eids[shm_eid_idx].eid = eid;
        spin_unlock(&shm_eid_idx_lock);
        spin_unlock(&shm_idx_lock);

        spin_lock(&shm_ownership_lock);
        enclave->shm_ownership = 0;
        spin_unlock(&shm_ownership_lock);

        return 0;
      }
    }
    printm("[SM@%s]error: shm eid has been fully used!\n", __func__);
    spin_unlock(&shm_idx_lock);
    return -1; // 共享内存关联的Enclave已满
  }
  printm("[SM@%s]shmid=%d is not exist.\n", __func__, shmid);
  spin_unlock(&shm_idx_lock);
  return -2; // 共享内存不存在
}

// 根据key中shmid和Enclave类型, 找到指定的Enclave ID
int32_t sm_getshm_eid(uint32_t shmid, uint32_t enclave_type){
  // uint32_t shm_key = key & SHM_KEY_MASK;
  // uint32_t enclave_type = key & ENCLAVE_TYPE_MASK;
  // printm("[SM@%s] enclave_type = %d.\n", __func__, enclave_type);

  // int32_t shmid = sm_get_shmid(key);

  unsigned int eid_next = -1;
  spin_lock(&shm_idx_lock);
  shm_idx = shmid;
  if (enclave_shm[shm_idx].used){
    spin_lock(&shm_eid_idx_lock);
    for (shm_eid_idx = 0; shm_eid_idx < NUM_EACH_SHM; shm_eid_idx++){
      if (enclave_shm[shm_idx].eids[shm_eid_idx].used && enclave_shm[shm_idx].eids[shm_eid_idx].enclave_type == enclave_type){
        eid_next = enclave_shm[shm_idx].eids[shm_eid_idx].eid;
        spin_unlock(&shm_eid_idx_lock);
        spin_unlock(&shm_idx_lock);
        printm("[SM@%s] enclave_type=%d, its eid = %d\n", __func__, enclave_type, eid_next);
        return eid_next;
      }
    }
    if (shm_eid_idx == NUM_EACH_SHM) {
      // printm("[SM@%s] enclave_type  %d  Enclave not exist.\n", __func__, enclave_type);
      spin_unlock(&shm_eid_idx_lock);
      spin_unlock(&shm_idx_lock);
    }
  }
  return eid_next; // -1 被转移的Enclave不存在
}

int32_t sm_transfer_shm(uint32_t shmid, uint32_t eid_next, u8 pt_perm){
  printm("[SM@%s]------ start-----\n", __func__);
  unsigned long paddr = 0, shm_size = 0;

  spin_lock(&shm_idx_lock);
  shm_idx = shmid;
  // printm("[SM@%s]enclave_shm[%lu].used = %d.\n", __func__, shm_idx, enclave_shm[shm_idx].used);
  if (enclave_shm[shm_idx].used){
    paddr = enclave_shm[shm_idx].paddr;
    shm_size = enclave_shm[shm_idx].size;
  }
  /*
  printm("[SM@%s]shm: paddr=%lx, size=%lu.\n", __func__, \
        paddr,\
        shm_size);
  */
  spin_unlock(&shm_idx_lock);

  struct enclave_t* enclave01, *enclave02;
  uint32_t eid = get_enclave_id();
  enclave01 = get_enclave(eid);
  enclave02 = get_enclave(eid_next);

  int ret = 0; 
  ret = map_pa2va(enclave01, enclave01->shm_ptr, (physical_addr_t) paddr, shm_size, PTE_NO_PERM);

  uintptr_t shm_pa = get_enclave_paddr_from_va(enclave01->root_page_table, enclave01->shm_ptr);
 
  // printm("[sm.c@%s] get_enclave_paddr_from_va return shm_pa 0x%lx \n", __func__, (long int)shm_pa);
  if (shm_pa != paddr || ret != 0){
	  printm("[sm.c@%s] error: close eid = %d pt_perm failed.\n", __func__, eid);
    return -1;
  }

  spin_lock(&shm_ownership_lock);
  enclave01->shm_ownership = 0;
  spin_unlock(&shm_ownership_lock);

  printm("[SM@%s] eid = %d pt_perm close.\n", __func__, enclave01->eid);


  ret = 0; 
  ret = map_pa2va(enclave02, enclave02->shm_ptr, (physical_addr_t) paddr, shm_size, (pt_perm | PTE_U) << 1);

  shm_pa = get_enclave_paddr_from_va(enclave02->root_page_table, enclave02->shm_ptr);
 
  // printm("[sm.c@%s] get_enclave_paddr_from_va return shm_pa 0x%lx \n", __func__, (long int)shm_pa);
  if (shm_pa != paddr || ret != 0){
	  printm("[sm.c@%s] error: open eid = %d pt_perm failed.\n", __func__, eid);
    return -2; 
  }

  spin_lock(&shm_ownership_lock);
  enclave02->shm_ownership = 1;
  spin_unlock(&shm_ownership_lock);

  printm("[SM@%s] eid = %d pt_perm open.\n", __func__, enclave02->eid);


  return 0; // succeed!
}


uint32_t sm_get_shm(uint32_t shmid){
  struct enclave_t* enclave;
  unsigned int eid = get_enclave_id();
  enclave = get_enclave(eid);
  
  spin_lock(&shm_ownership_lock);
  if (enclave->shm_ownership == 1){
      spin_unlock(&shm_ownership_lock);
      return 1;
  }
  spin_unlock(&shm_ownership_lock);
  return 0;
}


int32_t sm_get_key_size(virtual_addr_t key, virtual_addr_t size){
  struct enclave_t* enclave;
  unsigned int eid = get_enclave_id();
  enclave = get_enclave(eid);

  unsigned long* var_key_pa = (unsigned long*)get_enclave_paddr_from_va(enclave->root_page_table, key);
  *var_key_pa = enclave->key;
  unsigned long* var_size_pa = (unsigned long*)get_enclave_paddr_from_va(enclave->root_page_table, size);
  *var_size_pa = enclave->rw_size;
  return 1;
}


/*
uint64_t sm_clock_start(){
  uint64_t time = csr_read(CSR_TIME);
  printm("[SM@%s] clock_start = %lu.\n", __func__, time);
  return time;
}

uint64_t sm_clock_end(){
  uint64_t time = csr_read(CSR_TIME);
  printm("[SM@%s] clock_end = %lu.\n", __func__, time);
  return time;
}
*/


uint64_t sm_clock_start(){
  // csr_clear(CSR_MIE, MIP_MTIP);


  // csr_clear(CSR_MSTATUS, MSTATUS_MIE);
  // csr_clear(CSR_MSTATUS, MSTATUS_SIE);
  // spin_lock(&clock_lock);
	csr_clear(CSR_MIP, MIP_STIP);
	csr_clear(CSR_MIP, MIP_MTIP);
  csr_clear(CSR_MIE, MIP_STIP);
	csr_clear(CSR_MIE, MIP_MTIP);

  uint64_t time = csr_read(CSR_TIME);
  printm("[SM@%s] clock_start = %lu.\n", __func__, time);
  // return sbi_timer_value();
  return time;
}

uint64_t sm_clock_end(){
  // while (!spin_lock_check(&clock_lock));
  // uint64_t clock_end = sbi_timer_value();

  // csr_set(CSR_MSTATUS, MSTATUS_SIE);
  // csr_set(CSR_MSTATUS, MSTATUS_MIE);

  uint64_t time = csr_read(CSR_TIME);
  printm("[SM@%s] clock_end = %lu.\n", __func__, time);
  csr_set(CSR_MIE, MIP_STIP);
  csr_set(CSR_MIE, MIP_MTIP);
  // spin_unlock(&clock_lock);
  // return clock_end;
  return time;
}
