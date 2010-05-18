/* Intel 8026 keyboard implementation */

int kbd_open();
int kbd_close();
int kbd_read(void *buf, int buflen);
int kbde_read(void *buf, int buflen);
int kbd_ioctl(unsigned long command, va_list args);
