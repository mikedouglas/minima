/* kernel.h - disable, enable, halt, restore, isodd, min, max */
#include <stdarg.h>

/* Symbolic constants used throughout Xinu */

typedef	char		Bool;		/* Boolean type			*/
#define	FALSE		0		/* Boolean constants		*/
#define	TRUE		1
#define	EMPTY		(-1)		/* an illegal gpq		*/
#define	NULL		0		/* Null pointer for linked lists*/
#define	NULLCH		'\0'		/* The null character		*/
#define INF_LOOP for(;;);

/* Universal return constants */

#define	OK		 1		/* system call ok		*/
#define	SYSERR		-1		/* system call failed		*/
#define	EOF		-2		/* End-of-file (usu. from read)	*/
#define	TIMEOUT		-3		/* time out  (usu. recvtim)	*/
#define	INTRMSG		-4		/* keyboard "intr" key pressed	*/
					/*  (usu. defined as ^B)	*/
#define	BLOCKERR	-5		/* non-blocking op would block	*/

#define PROCS_NUM 64                    // number of procs allowed
#define FD_NUM 4

enum proc_state { READY, STOPPED, BLOCKED };
enum proc_req { TIMER_INT, CREATE, YIELD, STOP, PUTS, SEND, RECV, SLEEP, 
                OPEN, CLOSE, READ, WRITE, IOCTL, KBD_INT, SIGHANDLER,
                KILL, SIGWAIT, SIGRET };
struct fd_ent {
  int status;
  struct dev_ent *dev;
};

struct proc_ctrl_blk {
  enum proc_state state;         // current state
  int *s_start;                  // start of stack
  int s_size;                    // size of stack
  int *esp;                      // stack pointer while in kernel

  int sysret;                    // return value of last system call
  va_list args;                  // pointer to arguments for a syscall

  void *buffer;                  // pointer to IPC buffer
  unsigned int buflen;           // length of IPC buffer

  int fd_used : 8;               // bitmask of fd usage (1 -> used, 0 -> free)
  struct fd_ent fd[FD_NUM];      // file descriptors

  int sighandler[32];         // array of signal handler pointers
  int sigmask;                   // registered signals
  int sigpending;                // pending signals
  int sigpriority;               // priority mask
  struct proc_ctrl_blk *sender;  // PID of sender, otherwise -1
  struct proc_ctrl_blk *next;    // next in list (either ready, blocked, or stopped)
};

struct dev_ent {
  int (*open)();
  int (*close)();
  int (*read)();
  int (*write)();
  int (*ioctl)();  
};

struct mem_header {
  unsigned long size;             // size of memory in blk, to the next mult. of 16
  struct mem_header *prev, *next; // for free list
  char *sanity;                   // 0xBEEF (avoid freeing bad mem)
  unsigned char data[0];          // start of data
};

// supplied
extern void enable_irq(int, int);
extern int inb(int);
extern void outb(int, int);
extern void end_of_intr();
extern void initPIT(int);
extern int getCS();
extern void blkcopy(void*, void*, int len);

// defined in kprintf.c
extern int kprintf(char *fmt, ...);

// defined in mem.c
extern void kmeminit();
extern void *kmalloc(unsigned int size);
extern void kfree(void *ptr);

// defined in disp.c
extern void procinit();
extern int getpid(struct proc_ctrl_blk *);
extern int getcpid();
extern struct proc_ctrl_blk* getproc(int id);
extern void add_to_ready(struct proc_ctrl_blk *);
extern void dispatch();

// defined in create.c
extern int create(void(*func)(), int stack);

// defined in ctsw.c
extern void contextinit();
extern enum proc_req contextswitch(struct proc_ctrl_blk*);

// defined in user.c
extern void root();
extern void idleproc();

// defined in sleep.c
extern int sleep(struct proc_ctrl_blk *p, int val);
extern void tick();

// defined in msg.c
int send(unsigned int, void*, unsigned int, struct proc_ctrl_blk*);
int recv(int*, void*, unsigned int, struct proc_ctrl_blk*);

// device handling
#define DEVNUM 2
// defined in di_calls.c
extern int di_open(struct proc_ctrl_blk *cproc, int devno);
extern int di_close(struct proc_ctrl_blk *cproc, int fd);
extern int di_write(struct proc_ctrl_blk *cproc, int fd, void *buf, int buflen);
extern int di_read(struct proc_ctrl_blk *cproc, int fd, void *buf, int buflen);
extern int di_ioctl(struct proc_ctrl_blk *cproc, int fd, unsigned long cmd, va_list args);
// defined in dev.c
extern struct dev_ent dev_table[DEVNUM];
extern void deviceinit();

// defined in kbd.c
extern int kbd_handle_int();

// defined in signal.c
#define SIGNUM 32
extern int signal(int pid, int signo);
extern void sigreturn(struct proc_ctrl_blk *p, void *osp, int opm);
extern int sigtramp(void (*handler)(void*), void *cntx, void *osp, int opm);
extern void setuptramp(struct proc_ctrl_blk *p);
extern int sigreg(struct proc_ctrl_blk *p, void (*func)(void*), int signo);
