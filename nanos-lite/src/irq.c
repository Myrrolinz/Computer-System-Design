#include "common.h"


extern _RegSet* do_syscall(_RegSet *r);
extern _RegSet* schedule(_RegSet*);
static _RegSet* do_event(_Event e, _RegSet* r) 
{
  switch (e.event) {
	case _EVENT_SYSCALL:do_syscall(r);break;
	case _EVENT_TRAP:return schedule(r);break;
	case _EVENT_IRQ_TIME:return schedule(r);break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
