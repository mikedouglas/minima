/* msg.c : messaging system (assignment 2)
 */

#include <kernel.h>

#define MIN(X,Y) ((X < Y) ? X : Y)

int send(unsigned int to, void *buffer, unsigned int len, struct proc_ctrl_blk *fr_p) {
  struct proc_ctrl_blk *to_p = getproc(to);
  struct proc_ctrl_blk *i;

  if (to_p->state == BLOCKED) { // ready to send
    blkcopy(to_p->buffer, buffer, MIN(len, to_p->buflen));
    to_p->state = READY;
    return MIN(len, fr_p->buflen);
  }
  else if (to_p->state == READY) {
    fr_p->buffer = buffer;
    fr_p->buflen = len;

    // add to sender linked list
    i = to_p->sender;
    if (i != NULL) {
      for (; i->next != NULL; i = i->next);
      i->next = fr_p;
    }
    else
      to_p->sender = fr_p;

    return -1; // yield until ready
  }
  else {
    return -2; // bad pid specified
  }
}

int recv(int *from, void *buffer, unsigned int len, struct proc_ctrl_blk *to_p) {
  struct proc_ctrl_blk *fr_p;

  if (to_p->sender != NULL) { // ready to receive
    fr_p = to_p->sender;
    blkcopy(buffer, fr_p->buffer, MIN(len, fr_p->buflen));
    *from = getpid(fr_p);
    fr_p->sysret = MIN(len, fr_p->buflen);
    fr_p->state = READY;
    to_p->sender = to_p->sender->next; // reset sender
    return MIN(len, fr_p->buflen);
  }
  else {
    to_p->sysret = (int)from; // store pointer to 'from' arg in sysret
    to_p->buflen = len;
    to_p->buffer = buffer;
    return -1;
  }
}
