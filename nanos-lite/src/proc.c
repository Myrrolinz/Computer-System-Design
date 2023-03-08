#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
PCB* fg_pcb;

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void *entry);
void context_uload(PCB *pcb, const char *filename);
void register_pcb(PCB *pcb);

//时间片, 在switch_boot_pcb里初始化为0, 在switch_fgpcb也可以初始化为0，这样就能立马切换程序.
int time_piece = 0; 

void switch_boot_pcb() {
  current = &pcb_boot;
  time_piece = 0;
}

void switch_fgpcb(int index){
	assert(index < 4);
	Log("fg_pcb switch to %d", index);
	fg_pcb = &pcb[index];
	time_piece = 0;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

void init_proc() {

  Log("Initializing processes...");
  //context_kload(&pcb[0], (void *)hello_fun);
  
  context_uload(&pcb[0], "/bin/hello");
  context_uload(&pcb[1], "/bin/init");
  context_uload(&pcb[2], "/bin/pal");
  context_uload(&pcb[3], "/bin/events");
  switch_fgpcb(1);
  //context_uload(&pcb[1], "/bin/pal");
  //context_uload(&pcb[0], "/bin/dummy");
  //naive_uload(NULL, "/bin/dummy");
  switch_boot_pcb();
  
  // load program here
  // naive_uload(NULL, "/bin/init");
  
}

_Context* schedule(_Context *prev) {
  current->cp = prev;
  // 每个进程的时间片不一样，fg_pcb优先级较高
  if(time_piece <= 0){
  	current = (current == &pcb[0] ? fg_pcb : &pcb[0]);
  	time_piece = (current == &pcb[0] ? 1 : 100);
  }
  -- time_piece;
  return current->cp;
}
