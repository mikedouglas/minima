/* create.c : create a process
 */

#include <kernel.h>
#include <syscall.h>
#include <errno.h>

extern struct proc_ctrl_blk *stopped, *ready;
extern struct proc_ctrl_blk *push(struct proc_ctrl_blk*, struct proc_ctrl_blk*);
extern struct proc_ctrl_blk *pop(struct proc_ctrl_blk*);

void push_s(int **stack, int val);

/* create - creates a new proc, executing from func.
 * returns pid or -1 (if creation failed) */
int create(void (*func)(), int stack) {
  struct proc_ctrl_blk *new = stopped;

  if (new == NULL) // reached proc limit
    return ENOMEM;

  stopped = pop(stopped);
  ready = push(ready, new);

  new->state = READY;
  new->next = NULL;
  new->sigmask = 0;
  new->sigpending = 0;
  new->sigpriority = ~0;
  for (int i = 0; i < SIGNUM; i++)
    new->sighandler[i] = NULL;
  new->s_size = stack;
  new->s_start = (int*)kmalloc(stack);
  // stack starts at largest adr
  new->esp = new->s_start + stack;

  // prepare stack for context switch
  push_s(&new->esp, (int)*sysstop); // final return calls sysstop()
  push_s(&new->esp, 0x3200); // flags (privileged IO space & int enabled)
  push_s(&new->esp, getCS());
  push_s(&new->esp, (int)func);
  new->esp -= 8; // make room for a popa
  
  return getpid(new);
}

void push_s(int **stack, int val) {
  (*stack)--;
  (*(*stack)) = val;
}
