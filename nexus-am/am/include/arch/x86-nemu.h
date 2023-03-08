#ifndef __ARCH_H__
#define __ARCH_H__

/* PA3.1
 * Date: 2020/08/10
 */
struct _Context {
  struct _AddressSpace *as;
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  int irq;
  uintptr_t eip, cs, eflags;
};

/* PA3.2
 * Date: 2020/8/14
 * refers to navy-apps/libs/libos/src/nanos.c
 */
#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif
