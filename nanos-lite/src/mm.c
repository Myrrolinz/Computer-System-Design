#include "memory.h"
#include "proc.h"

#ifndef PTE_P
#define PTE_P 0x001
#endif
#define PTE_ADDR(pte)    ((uint32_t)(pte) & ~0xfff)
#define PDX(va)          (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)          (((uint32_t)(va) >> 12) & 0x3ff)

static void *pf = NULL;

int is_mapped(_AddressSpace *as, uintptr_t va);

void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  assert(pf < (void *)_heap.end);
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

int is_mapped(_AddressSpace *as, uintptr_t va)
{
    //Log("check is mapped: 0x%08x", va);
	uint32_t *pgdir = as->ptr;
	if(!(pgdir[PDX(va)] & PTE_P )) {
		return 0;
	}
	uint32_t pde = pgdir[PDX(va)];
	uint32_t *pgtable = (uint32_t *)(uintptr_t)(PTE_ADDR(pde));
	if( pgtable[PTX(va)] & PTE_P ){
		return 1;	
	}
	return 0;
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk, intptr_t increment) {
  //Log("brk: 0x%08x increment: %d", brk, increment);
  if(current->max_brk == 0){
    current->max_brk = brk;
    //Log("register max_brk: %d", brk);
  }

  if(brk + increment > current->max_brk)
  {
  	uintptr_t va = current->max_brk;
  	// 内存不对齐的情况，同时该地址有没有映射到物理页上
  	if(!is_mapped(&(current->as), va) && va % PGSIZE != 0) {
  	    //Log("check is_mapped false: 0x%08x", va);
  		_map(&(current->as), (void *)va, new_page(1), PTE_P);
  	} 
  	va = PGROUNDUP(va);
  	while(va < brk + increment) {
  		_map(&(current->as), (void *)va, new_page(1), PTE_P);
  		va += PGSIZE;
  	}
  	current->max_brk = brk + increment;
  	//Log("max_brk update to: %d", brk + increment);
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _vme_init(new_page, free_page);
}
