#include "rtl/rtl.h"

#define IRQ_TIMER 32          // for x86

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
	/* TODO: Trigger an interrupt/exception with ``NO''.
	 * That is, use ``NO'' to index the IDT.
	 */
    
    Assert(NO <= cpu.IDTR.limit, "interrupt NO is outof bound.");
	rtl_push(&cpu.eflags.val);
	rtl_push(&cpu.cs);
    rtl_push(&ret_addr);
    
    cpu.eflags.IF = 0;	// disable the interrupt
    
	uint32_t low = vaddr_read(cpu.IDTR.base + 8*NO, 4) & 0x0000ffff;
	uint32_t high = vaddr_read(cpu.IDTR.base + 8*NO + 4, 4) & 0xffff0000;
	
	rtl_j(high | low);
}

bool isa_query_intr(void) {
  if ( cpu.INTR && cpu.eflags.IF ) {
    cpu.INTR = false;
    // cpu.pc has been updated
    raise_intr(IRQ_TIMER, cpu.pc);
    return true;
  }
  return false;
}
