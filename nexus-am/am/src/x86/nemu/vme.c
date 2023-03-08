#include <am.h>
#include <x86.h>
#include <nemu.h>
#include <klib.h> 

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN = {};
static PTE kptabs[(PMEM_SIZE + MMIO_SIZE) / PGSIZE] PG_ALIGN = {};
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;


static _Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE},
  {.start = (void*)MMIO_BASE,  .end = (void*)(MMIO_BASE + MMIO_SIZE)}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    //printf("[0x%08x 0x%08x]\n", segments[i].start, segments[i].end);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return 0;
}

int _protect(_AddressSpace *as) {
  PDE *updir = (PDE*)(pgalloc_usr(1));
  as->ptr = updir;
  //printf("_protecting... as: 0x%08x, as->ptr: 0x%08x\n", as, as->ptr);
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
  	updir[i] = kpdirs[i];
    //PTE pte = PGADDR(i, 0, 0);
    //PTE pte_end = PGADDR(i + 1, 0, 0);
    //printf("copy: [0x%08x, 0x%08x), pde: %d\n", pte, pte_end, updir[i]);
  }

  return 0;
}

void _unprotect(_AddressSpace *as) {
}

static _AddressSpace *cur_as = NULL;
void __am_get_cur_as(_Context *c) {
  c->as = cur_as;
}

void __am_switch(_Context *c) {
  if (vme_enable) {
    set_cr3(c->as->ptr);
    cur_as = c->as;
  }
}


int _map(_AddressSpace *as, void *va, void *pa, int prot) {
  assert(get_cr0() & CR0_PG);
  assert(as->ptr);
  PDE *pgdir = as->ptr;
  if(!(pgdir[PDX(va)] & PTE_P )) {
    // 分配页面地址已经与4KB对齐了，因此不需要使用PTE_ADDR了
    uintptr_t pa = (uintptr_t)pgalloc_usr(1);
    assert(pa % PGSIZE == 0);
  	pgdir[PDX(va)] = pa | PTE_P;
  }
  PDE pde = pgdir[PDX(va)];
  //printf("_map: va: 0x%08x, pde: %d\n", va, pde);
  PTE *pgtable = (PTE *)(PTE_ADDR(pde));
  if(pgtable[PTX(va)] & PTE_P){
  	printf("remapped! srcva: 0x%08x -> pa: 0x%08x, curva: 0x%08x -> pa: 0x%08x\n", va, PTE_ADDR(pgtable[PTX(va)]), va, PTE_ADDR(pa));
  }
  pgtable[PTX(va)] = (uintptr_t)PTE_ADDR(pa) | prot ;
  //printf("_map success. va: 0x%08x, pa: 0x%08x, prot: %d\n", va, pa, prot);
  return 0;
}

_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack, void *entry, void *args) {
   //printf("start: %x, end: %x\n", ustack.start, ustack.end);
  _Context *cp = (_Context *)(ustack.end - 12) - 1;
  cp->as = as;
  cp->cs = 8;
  cp->eip = (uintptr_t)entry;
  cp->eflags = (1 << 9);
  return cp;
}
