#include "proc.h"
#include "fs.h"
#include <elf.h>


#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

typedef uint32_t PTE;
typedef uint32_t PDE;
#define PDX(va)          (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)          (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)          ((uint32_t)(va) & 0xfff)
#define ROUNDDOWN(a, sz) ((((uintptr_t)a)) & ~((sz)-1))
#define PTE_ADDR(pte)    ((uint32_t)(pte) & ~0xfff)
#define PGSIZE 4096
#define PTE_P          0x001   // Present
#define PGSHFT         12      // log2(PGSIZE)
#define PTXSHFT        12      // Offset of PTX in a linear address
#define PDXSHFT        22      // Offset of PDX in a linear address

#define MIN(a, b) (a < b ? a : b)

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
size_t ramdisk_read(void *buf, size_t offset, size_t len);
void context_uload(PCB *pcb, const char *filename);
void switch_boot_pcb();

PCB* pcb_load;
extern PCB* fg_pcb;


void load_single(int fd, PCB *pcb, uint32_t va, uint32_t bin_size, uint32_t sgsize)
{
    //Log("va: 0x%08x, bin_size: %d, sgsize: %d", va, bin_size, sgsize);
	uint32_t offset = va - ROUNDDOWN(va, PGSIZE);
	uint32_t size = 0;
	/* 第一部分, 不对齐要单独处理*/ 
    if(offset > 0) {
    	size = PGSIZE - offset;
		void *pa = new_page(1);
	    // va和va - offset 都行，用不到低12位
	    _map(&(pcb->as), (void *)(uintptr_t)va, pa, PTE_P);
	    fs_read(fd, pa, MIN(size, bin_size));
    }
    uint32_t i;
	for (i = size; i < bin_size; i += PGSIZE) {
        void *pa = new_page(1);
        _map(&(pcb->as), (void *)(uintptr_t)(va + i), pa, PTE_P);
		fs_read(fd, pa, MIN(PGSIZE, bin_size - i));
    }
    /*第三部分, 物理页填充0*/
    while (i < sgsize) {
        void *pa = new_page(1);
        _map(&(pcb->as), (void *)(uintptr_t)(va + i), pa, PTE_P);
		memset((void *)(uintptr_t)pa, 0, PGSIZE);
        i += PGSIZE;
    }
}

void register_pcb(PCB *pcb_t) {
    printf("pcb_load: 0x%08x\n", pcb_t);
	pcb_load = pcb_t;
}

static uintptr_t loader(PCB *pcb, const char *filename) {
    _protect(&(pcb->as));
    //Log("_protect success. pcb: 0x%08x, pcb->as->ptr: 0x%08x", pcb, (pcb->as).ptr);
	int fd = fs_open(filename, 0, 0);
	Elf_Ehdr ehdr;
	fs_lseek(fd, 0, SEEK_SET);
	fs_read(fd, (void *)&ehdr, sizeof(Elf_Ehdr));
	
	// check whether the file format is ELF
	if (!( ehdr.e_ident[EI_MAG0] == ELFMAG0 &&
		ehdr.e_ident[EI_MAG1] == ELFMAG1 &&
		ehdr.e_ident[EI_MAG2] == ELFMAG2 &&
		ehdr.e_ident[EI_MAG3] == ELFMAG3) ) {
		
		assert(0);
	}
  
	Elf_Phdr phdr; 
	int num = ehdr.e_shnum;
	for(int i = 0; i < num; ++ i){
	    //Log("start read Phdr");
		fs_lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
		fs_read(fd, &phdr, sizeof(Elf_Phdr));
		//Log("read Phdr success");
		if(phdr.p_type != PT_LOAD) continue; 
		
		fs_lseek(fd, phdr.p_offset, SEEK_SET);
		load_single(fd, pcb, phdr.p_vaddr, phdr.p_filesz, phdr.p_memsz);
		Log("load success. va: [0x%08x, 0x%08x)", phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz);
		//memset((void *)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
	}
	
	fs_close(fd);
	Log("load %s success. PCB: 0x%08x", filename, pcb);
	return ehdr.e_entry;
}

/* 支持开机菜单程序的运行 (选做) (旧版本)
 * Date: 2020/08/21
 * 大概完成了这个功能。在pro.c中调用register函数，即设置pcb_load为 &pcb[1], 这个pcb_load专门用于加载程序
 * 然后pcb[0] 就是正常的/bin/init即可，schedule函数中pcb[0] 和 pcb[1]相互切换即可. 
 * device.c中把读写函数中的_yield去掉，程序就能一直执行下去了.
 */
 
 /* 支持开机菜单程序的运行 (选做) (更新)
 * Date: 2020/08/21
 * 大概完成了这个功能。在native_uload中调用register函数，即设置pcb_load, 这个pcb_load专门用于加载程序
 * pcb_load和加载/bin/init的要pcb要保持一致. 这里直接使用fg_pcb了. 
 * 注意要调用switch_boot_pcb，不然current还是之前的pcb,这样就不对了
 */
 
void naive_uload(PCB *pcb, const char *filename) {
  if(pcb == NULL) {
    register_pcb(fg_pcb);
  	context_uload(pcb_load, filename);
  	pcb_load->max_brk = 0;	// 注意要清零
  	switch_boot_pcb();
    _yield();
  	return ;
  }
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);
  
  pcb->cp = _kcontext(stack, entry, NULL);
  /* PA4.2 
   * Date: 2020/08/21
   * 修复缺页错误 (选做)
   */
  _protect(&(pcb->as));
  pcb->cp->as = &(pcb->as);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
