/* Intel 8026 keyboard implementation */
#include <kernel.h>
#include <kbd.h>

#define MIN(X,Y) ((X > Y) ? Y : X)

#define CMD_PORT 0x64
#define DATA_PORT 0x60
#define KBD_MAX 4

extern unsigned int kbtoa(unsigned char code);

static char kbd_buf[KBD_MAX];
static char *p_buf;
static int p_idx = 0;
static int p_len = 0;
static int echo = 0;
static int idx = 0;
static int eof = 4;
static int eof_signaled = 0;
static int open = 0;

int kbd_handle_int() {
  if (eof_signaled)
    return 0;
  
  int key = kbtoa(inb(DATA_PORT));

  if (key == 256)
    return -1;

  if ((p_len - p_idx) > 0) {
    p_buf[p_idx++] = key;

    if ((p_len == p_idx) || key == eof) {
      // bring proc back to life
      if (key == eof) {
        eof_signaled = 1;
        return p_idx - 1;
      }
      if (echo)
        kprintf("%c", key);

      return p_idx;
    }
  }
  else if (idx != KBD_MAX) {
    kbd_buf[idx++] = key;
  }

  if (echo)
    kprintf("%c", key);
  return -1;
}

int kbd_open() {
  if (open)
    return -1;

  open = 1;
  eof_signaled = 0;
  outb(0xAE, 0x64);
  enable_irq(1, 0);

  return 0;
}

int kbd_close() {
  open = 0;
  outb(0xAD, 0x64);
  enable_irq(1, 1);

  return 0;
}

// keyboard read
int kbd_read(void *buf, int buflen) {
  // copy leftovers in driver buffer
  int copied = MIN(buflen, idx);
  if (copied > 0) {
    blkcopy(buf, kbd_buf, copied);
    buflen -= copied;
    idx -= copied;
  }

  if (buflen == 0)
    return copied;

  p_idx = 0;
  p_buf = buf;
  p_len = buflen;
  echo = 0;

  return -2; // signal wait
}

// kbd_read function for echoing keyboards
int kbde_read(void *buf, int buflen) {
  int ret = kbd_read(buf, buflen);
  echo = 1;

  return ret;
}

// keyboard ioctl
int kbd_ioctl(unsigned long command, va_list args) {
  int neof = va_arg(args, int);

  if (command == 88 && neof > -1 && neof < 128)
    eof = neof;
  else
    return SYSERR;

  return 0;
}
