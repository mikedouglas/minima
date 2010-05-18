/* ctsw.c : context switcher
 */

#include <kernel.h>

static void *kstack;
static int *ESP;
static int rc;
static enum proc_req call;
static va_list args;
static int timer_bit;
extern int set_evec(int, long);

/* contextswitch - saves kernel context, switches to proc */
enum proc_req contextswitch(struct proc_ctrl_blk *proc) {
  ESP = proc->esp;
  rc = proc->sysret;

  asm volatile("pushf\n"          // save kernel flags
               "pusha\n"          // save kernel regs
               "movl %%esp, %0\n" // save kernel %esp
               "movl %7, %%esp\n" // load proc %esp
               "popa\n"           // load proc regs (from proc stack)
               "movl %8, %%eax\n" // get return value of syscall
               "iret\n"           // switch to proc
          "_kbd_entry_point:\n"
               "cli\n"            // disable interrupts
               "pusha\n"          // save proc regs
               "movl %%eax, %5\n" // save %eax
               "movl $2, %%eax\n" // enable kbd bit
               "jmp _common_entry_point\n"
          "_timer_entry_point:\n"
               "cli\n"            // disable interrupts
               "pusha\n"          // save proc regs
               "movl %%eax, %5\n" // save %eax
               "movl $1, %%eax\n" // enable timer bit
               "jmp _common_entry_point\n"
          "_syscall_entry_point:\n"
               "cli\n"            // disable interrupts
               "pusha\n"          // save proc regs
               "movl $0, %%eax\n" // disable timer bit
          "_common_entry_point:\n"
               "movl %%esp, %1\n" // save proc %esp
               "movl %%ebx, %2\n" // save call type
               "movl %%edx, %3\n" // save args
               "movl %%eax, %4\n" // save timer_bit
               "movl %6, %%esp\n" // restore kernel %esp
               "popa\n"           // restore kernel regs (from kstack)
               "popf"             // restore kernel flags
               : "=m"(kstack), "=m"(ESP), "=m"(call), "=m"(args), "=m"(timer_bit), "=m"(rc)
               : "m"(kstack), "m"(ESP), "m"(rc)
               : "%edx", "%ebx", "%eax"
               );
  proc->esp = ESP;
  proc->args = args;
  proc->sysret = rc;

  if (timer_bit == 2)
    return KBD_INT;
  if (timer_bit)
    return TIMER_INT;
  else
    return call;
}

void contextinit() {
  // accesses address of contextswitch entry points (see contextswitch())
  extern void _syscall_entry_point();
  set_evec(0x80, (long)&_syscall_entry_point);

  // setup timer
  extern void _timer_entry_point();
  set_evec(0x20, (long)&_timer_entry_point);
  initPIT(100);

  // setup keyboard
  extern void _kbd_entry_point();
  set_evec(0x21, (long)&_kbd_entry_point);
}
