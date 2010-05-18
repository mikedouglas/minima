/* syscall.c : syscalls
 */

#include <kernel.h>
#include <stdarg.h>

/* syscall - stores call id and the start of va in registers
 * %ebx and %edx, respectively. Then calls the generic syscall
 * interrupt handler. */
int syscall(enum proc_req call, ...) {
  int rt;
  va_list ap;

  va_start(ap, call);
  asm volatile("movl %1, %%ebx\n"
               "movl %2, %%edx\n"
               "int $0x80\n"
               "movl %%eax, %0\n"
               : "=g"(rt)
               : "g"(call), "g"(ap)
               : "%eax", "%ebx", "%edx"
              );
  va_end(ap);
  
  return rt;
}

int sysyield() {
  return syscall(YIELD);
}

int syscreate(void(*func)(), int stack) {
  return syscall(CREATE, func, stack);
}

int sysstop() {
  return syscall(STOP);
}

int sysgetpid() {
  return getcpid();
}

/* msging */
int sysputs(char *str) {
  return syscall(PUTS, str);
}

int syssend(int to, void *buffer, unsigned int len) {
  if (to < 0 || to > PROCS_NUM-1)
    return SYSERR;
  else
    return syscall(SEND, to, buffer, len);
}

int sysrecv(int *from, void *buffer, unsigned int len) {
  return syscall(RECV, from, buffer, len);
}

int syssleep(int length) {
  return syscall(SLEEP, length);
}

/* device handling */
int sysopen(int devno) {
  if ((devno < 0) || (devno > DEVNUM)) {
    return SYSERR;
  }

  return syscall(OPEN, devno);
}

int sysclose(int fd) {
  return syscall(CLOSE, fd);
}

int syswrite(int fd, void *buf, int buflen) {
  if (buflen < 0)
    return SYSERR;

  return syscall(WRITE, fd, buf, buflen);
}

int sysread(int fd, void *buf, int buflen) {
  if (buflen < 0)
    return SYSERR;

  // syscall(READ,...) returns:
  //  1. number of chars left to read.
  //  2. -1, if bad fd.
  return syscall(READ, fd, buf, buflen);
}

int sysioctl(int fd, unsigned long command, ...) {
  va_list ap;

  va_start(ap, command);
  int ret = syscall(IOCTL, fd, command, ap);
  va_end(ap);
  return ret;
}

int syssighandler(int signal, void (*handler)(void *)) {
  return syscall(SIGHANDLER, signal, handler);
}

int syskill(int pid, int sig) {
  return syscall(KILL, pid, sig);
}

int syssigwait() {
  return syscall(SIGWAIT);
}

int syssigreturn(int *old_esp, int opm) {
  return syscall(SIGRET, old_esp, opm);
}
