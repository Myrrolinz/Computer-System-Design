#include "nemu.h"
#include "isa/mmu.h"

#define CR0_PG         	 0x80000000  // Paging
#define PTXSHFT          12      // Offset of PTX in a linear address
#define PDXSHFT          22      // Offset of PDX in a linear address
#define PDX(va)          (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)          (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)          ((uint32_t)(va) & 0xfff)
#define PTE_ADDR(pte)    ((uint32_t)(pte) & ~0xfff)

paddr_t page_translate(vaddr_t addr) {
	if(!cpu.cr0.paging) return addr;
	PDE pde;
	pde.val = paddr_read(PTE_ADDR(cpu.cr3.val) + PDX(addr) * 4, 4);
	Assert(pde.present, "pde: %d, addr: 0x%08x", pde.val, addr);
	PTE pte;
	pte.val = paddr_read(PTE_ADDR(pde.val) + PTX(addr) * 4 , 4);
	Assert(pte.present, "pte: %d, addr: 0x%08x", pte.val, addr);
	paddr_t paddr = PTE_ADDR(pte.val) | OFF(addr);
	return paddr;
}

uint32_t isa_vaddr_read(vaddr_t addr, int len) {
  // data cross the page boundary
  if( OFF(addr) + len > PAGE_SIZE) {
    //Log("data cross the page read. addr: 0x%08x", addr);
  	uint32_t low_addr, high_addr;
  	//Log("lowaddr: 0x%08x", addr);
  	int low_len = PAGE_SIZE - OFF(addr);
  	int high_len = len - low_len;
  	low_addr = page_translate(addr);
  	addr |= PAGE_MASK;
  	addr ++;
  	//Log("high_addr: 0x%08x", addr);
  	high_addr = page_translate(addr);
  	return (paddr_read(high_addr, high_len) << (low_len * 8)) | paddr_read(low_addr, low_len);
  }
  //Log("data not cross the page read. addr: 0x%08x", addr);
  paddr_t paddr = page_translate(addr);
  
  return paddr_read(paddr, len);
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len) {
  if( OFF(addr) + len > PAGE_SIZE) {
  	assert(0);
  	/*
  	uint32_t low_addr, high_addr;
  	int low_len = PAGE_SIZE - OFF(addr);
  	int high_len = len - low_len;
  	low_addr = page_translate(addr);
  	addr &= PAGE_MASK;
  	addr += PAGE_SIZE;
  	high_addr = page_translate(addr);
  	paddr_write(low_addr, data & ((1 << (low_len * 8)) - 1), low_len);
  	paddr_write(high_addr, data >> (low_len * 8), high_len);
  	*/
  }
  paddr_t paddr = page_translate(addr);
  paddr_write(paddr, data, len);
}
