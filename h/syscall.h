#ifndef _SYSCALL_H_
#define _SYSCALL_H_

int sysyield();
int syscreate(void(*func)(), int stack);
int sysstop();

int sysgetpid();

int sysputs(char*);

int syssend(int, void*, unsigned int);
int sysrecv(unsigned int*, void*, unsigned int);

int syssleep(int);

int sysopen(int);
int sysclose(int);
int syswrite(int, void*, int);
int sysread(int, void*, int);

int syssighandler(int signal, void (*handler)(void *));
int syskill(int pid, int sig);
int syssigwait();
int syssigreturn();

#endif // _SYSCALL_H_
