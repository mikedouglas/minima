/* signal code */
#include <kernel.h>
#include <syscall.h>

// sigreturn - removes highest priority signal and resets esp
void sigreturn(struct proc_ctrl_blk *p, void *osp, int opm) {
  // identify highest pri signal
  int i;
  for (i = 0; i < SIGNUM; i++) {
    if (p->sigpending & (1 << i))
      break;
  }
  // remove it from mask
  p->sigpending -= (1 << i);
  p->sigpriority = opm;
  p->esp = (int*)osp;
}

// sigtramp - calls handler, exits using special sigreturn
int sigtramp(void (*handler)(void *), void *ctnx, void *osp, int opm) {
  handler(ctnx);
  syssigreturn(osp, opm);

  return 0;
}

// signal - signals process if handler setup
int signal(int pid, int signo) {
  struct proc_ctrl_blk *p = getproc(pid);

  if (p->sigmask & (1 << signo))
    p->sigpending |= (1 << signo);

  return 0;
}

// setuptramp - setups stack for signal
void setuptramp(struct proc_ctrl_blk *p) {
  int *esp = p->esp;
  int npm = ~0;

  int i;
  for (i = 0; i < SIGNUM; i++) {
    if (p->sigpending & (1 << i))
      break;
    npm = npm << 1;
  }

  *(esp) = p->sigpriority;   // priority mask
  *(esp - 1) = (int)esp;         // old sp
  *(esp - 2) = 0xDEADBEEF;       // what should go here?
  *(esp - 3) = p->sighandler[i]; // handler
  *(esp - 5) = 0x3200;
  *(esp - 6) = getCS();
  *(esp - 7) = (int)sigtramp;    // return to sigtramp
  p->esp -= 7+8;

  p->sigpriority = ~npm;
}

int sigreg(struct proc_ctrl_blk *p, void (*func)(void*), int signo) {
  if ((signo < 0) || (signo > 31))
    return SYSERR;
  
  p->sighandler[signo] = (int)func;

  if (func)
    p->sigmask |= (1 << signo);
  else
    p->sigmask -= (1 << signo);

  return 0;
}
