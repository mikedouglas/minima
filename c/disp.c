/* disp.c : dispatcher
 */

#include <i386.h>
#include <kernel.h>
#include <stdarg.h>

struct proc_ctrl_blk *ready;
struct proc_ctrl_blk *stopped;
//struct proc_ctrl_blk *blocked;

struct proc_ctrl_blk procs[PROCS_NUM];

void procinit() {
  int i;
  for (i = 0; i < PROCS_NUM; i++) {
    procs[i].state = STOPPED;
    procs[i].sender = NULL;
    procs[i].sysret = 0;
    procs[i].next = ((i+1) < PROCS_NUM) ? &procs[i+1] : NULL;
  }

  ready = NULL;    // list of ready procs
  stopped = procs; // list of stopped procs
  //blocked = NULL;  // list of procs waiting
}

struct proc_ctrl_blk *push(struct proc_ctrl_blk *lst, struct proc_ctrl_blk *p) {
  struct proc_ctrl_blk *last;

  p->next = NULL;
  if (lst) {
    for (last = lst; last->next != NULL; last = last->next);
    last->next = p;
    return lst;
  }
  else
    return p;
}

struct proc_ctrl_blk *pop(struct proc_ctrl_blk *lst) {
  if (!lst)
    return NULL;
  
  return lst->next;
}

struct proc_ctrl_blk *remove(struct proc_ctrl_blk *lst, struct proc_ctrl_blk *p) {
  if (!lst)
    return NULL;

  if (lst == p)
    return lst->next;

  struct proc_ctrl_blk *i;
  for (i = lst; i->next != p || i->next != NULL; i = i->next) {
    i->next = p->next;
  }

  return lst;
}

int getpid(struct proc_ctrl_blk *p) {
  return (p - procs);
}

int getcpid() {
  return getpid(ready);
}

struct proc_ctrl_blk *getproc(int id) {
  return &procs[id];
}

char *getstate(enum proc_state s) {
  char *ready = "READY";
  char *stopped = "STOPPED";
  char *blocked = "BLOCKED";

  switch (s) {
  case READY:
    return ready;
  case STOPPED:
    return stopped;
  case BLOCKED:
    return blocked;
  default:
    return NULL;
  }
}

void procinfo(struct proc_ctrl_blk *p) {
  kprintf("\n\n"
          "SAY HELLO TO PROC %d\n"
          "STATE: %s\n"
          "ESP: %d\n"
          "NEXT: %d\n\n\n", getpid(p), getstate(p->state), p->esp, p->next);
}

void add_to_ready(struct proc_ctrl_blk *p) {
  p->state = READY;
  ready = push(ready, p);
}

struct proc_ctrl_blk *yield(struct proc_ctrl_blk *p) {
  ready = pop(ready);
  add_to_ready(p);
  return ready;
}

/* dispatch - controls the process lists and schedules */
void dispatch() {
  struct proc_ctrl_blk *cproc = ready;
  struct proc_ctrl_blk *i, *kbd_proc = NULL;
  // TODO: clean up handling of args
  void(*func)();
  int stack;
  char *str;

  va_list args;
  int to, devno, fd, pid, signo, opm;
  unsigned int len;
  unsigned long command;
  int *from;
  void *buf, *esp;

  for (;;) {
    if (cproc == NULL) {
      kprintf("Nothing left to do. Bye.\n");
      asm("HLT");
    }

    if (cproc->sigpending & cproc->sigmask) {
      setuptramp(cproc);
    }

    enum proc_req request = contextswitch(cproc);

    switch(request) {
    case STOP:
      // handle unrequited sender
      while (cproc->sender != NULL) {
        add_to_ready(cproc->sender);
        cproc->sender = cproc->sender->next;
      }

      cproc->state = STOPPED;
      ready = pop(ready);
      stopped = push(stopped, cproc);
      kfree(cproc->s_start);
      cproc = ready;
      break;

    case CREATE:
      // assumes cproc->args has been properly initialized
      // by syscall()
      func = (void(*)()) va_arg(cproc->args, char*);
      stack = va_arg(cproc->args, int);
      cproc->sysret = create(func, stack);
      break;

    case YIELD:
      cproc = yield(cproc);
      break;

    case TIMER_INT:
      // yield
      tick();
      cproc = yield(cproc);
      end_of_intr();
      break;

    case SLEEP:
      len = va_arg(cproc->args, unsigned int);
      ready = pop(ready);
      cproc->sysret = sleep(cproc, len);
      cproc = ready;
      break;

    case PUTS:
      str = va_arg(cproc->args, char*);
      kprintf(str);
      break;

    case SEND:
      to = va_arg(cproc->args, int);
      buf = va_arg(cproc->args, char*);
      len = va_arg(cproc->args, unsigned int);

      if ((cproc->sysret = send(to, buf, len, cproc)) == -1) {
        // yield, and block
        ready = pop(ready);
        cproc->next = NULL;
        cproc->state = BLOCKED;
        cproc = ready;
      }
      else if (cproc->sysret == -2) // bad proc specified
        cproc->sysret = -1;
      else {
        i = getproc(to);
        *(int*)i->sysret = getcpid();
        i->sysret = cproc->sysret;
        add_to_ready(i);
      }
      break;

    case RECV:
      from = va_arg(cproc->args, int*);
      buf = va_arg(cproc->args, void*);
      len = va_arg(cproc->args, unsigned int);

      if ((len = recv(from, buf, len, cproc)) == -1) {
        // yield and block
        ready = pop(ready);
        cproc->next = NULL;
        cproc->state = BLOCKED;
        cproc = ready;
      }
      else {
        cproc->sysret = len;
        add_to_ready(getproc(*from));
      }
      break;

    case OPEN:
      devno = va_arg(cproc->args, int);
      cproc->sysret = di_open(cproc, devno); // returns fd
      break;

    case CLOSE:
      fd = va_arg(cproc->args, int);
      cproc->sysret = di_close(cproc, fd); // returns status
      break;

    case READ:
      fd = va_arg(cproc->args, int);
      buf = va_arg(cproc->args, void*);
      len = va_arg(cproc->args, int);
      cproc->sysret = di_read(cproc, fd, buf, len);

      if (cproc->sysret == -2) {
        kbd_proc = cproc;
        ready = pop(ready);
        cproc->next = NULL;
        cproc->state = BLOCKED;
        cproc = ready;
      }
      break;

    case WRITE:
      fd = va_arg(cproc->args, int);
      buf = va_arg(cproc->args, void*);
      len = va_arg(cproc->args, int);
      cproc->sysret = di_write(cproc, fd, buf, len);
      break;

    case IOCTL:
      fd = va_arg(cproc->args, int);
      command = va_arg(cproc->args, unsigned long);
      args = va_arg(cproc->args, va_list);
      cproc->sysret = di_ioctl(cproc, fd, command, args);
      break;

    case KBD_INT:
      if (kbd_proc) {
        kbd_proc->sysret = kbd_handle_int();
        if (kbd_proc->sysret != -1) {
          add_to_ready(kbd_proc);
          kbd_proc = NULL;
        }
      }
      else
        kbd_handle_int();

      end_of_intr();
      break;

    case SIGHANDLER:
      signo = va_arg(cproc->args, int);
      func = (void(*)()) va_arg(cproc->args, char*);
      cproc->sysret = sigreg(cproc, func, signo);
      break;

    case KILL:
      pid = va_arg(cproc->args, int);
      signo = va_arg(cproc->args, int);
      i = getproc(pid);

      if (i == NULL || i->state == STOPPED)
        cproc->sysret = SYSERR;
      else if ((signo < 0) || (signo > 31))
        cproc->sysret = SYSERR;
      else {
        if (i->state == BLOCKED)
          add_to_ready(i);
        cproc->sysret = signal(pid, signo);
      }
      break;

    case SIGWAIT:
      ready = pop(ready);
      cproc->next = NULL;
      cproc->state = BLOCKED;
      cproc = ready;
      break;

    case SIGRET:
      esp = va_arg(cproc->args, void*);
      opm = va_arg(cproc->args, int);
      sigreturn(cproc, esp, opm);
    }
  }
}
