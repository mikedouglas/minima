/* user.c : User processes
 */

#include <syscall.h>

//extern sprintf(char *, char *, ...);

void hello(void *cntx) {
  kprintf("Hello %x\n", cntx);
}

void helper() {
  syssleep(10);
  syskill(1, 20);
  syskill(1, 18);
}

void root() {
  sysputs("HELLO WORLD\n");

  sysputs("\n2. Opening keyboard.\n");
  int fd = sysopen(0);

  sysputs("\n3. Reads characters, one at a time until 10 characters are read.\n");
  char ch;
  int x;
  for (x = 0; x < 10; x++)
    sysread(fd, &ch, 1);

  sysputs("\n4. Attempts to open the “no echo” keyboard.\n");
  sysopen(1);
  sysputs("\n5. Attempts to open the “echo” keyboard.\n");
  sysopen(0);

  sysputs("\n6. Closes keyboard.\n");
  sysclose(fd);

  sysputs("\n7. 7. Opens the \“no echo\”.\n");
  fd = sysopen(1);

  sysputs("\n8. Does three reads of 25 characters and has the application print them (Each typed line is to be at least 30 characters long.)\n");
  char *msg = (char*)kmalloc(sizeof(char) * 76);
  int one = sysread(fd, msg, 25);
  int two = sysread(fd, msg + one, 25);
  sysread(fd, msg + one + two, 25);
  msg[75] = '\0';

  sysputs(msg);

  sysputs("\n9. Continues reading characters until an EOF indication is received.\n");
  while(sysread(fd, &ch, 1));

  sysputs("\n10. Closes keyboard and opens the “echo” keyboard.\n");
  sysclose(fd);
  fd = sysopen(0);

  sysputs("\n11. Installs a signal handler for signal 18 that simply prints that it was called.\n");
  syssighandler(18, &hello);
  syskill(1, 18);

  sysputs("\n12. Creates a new process that sleeps for 10 ticks and then sends a signal 20 to the root process followed by signal 18.\n");
  syscreate(&helper, 4096);

  sysputs("\n14. The read should be interrupted by the signal and get back an error, print out a progress indicator.\n");
  kprintf("value of interrupted read: %d\n", sysread(fd, &ch, 1));

  sysputs("\n15. Read input until an EOF indication is returned.\n");
  while("%d\n", sysread(fd, &ch, 1));

  sysputs("\n16. Attempt to read again and print the result that read returns.\n");
  kprintf("\nread after EOF: %d\n", sysread(fd, &ch, 1));

  sysputs("\n\nGOODBYE!\n");
}

void idleproc() {
  for (;;) {
    asm("hlt");
  }
}
