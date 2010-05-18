/* device independent stuff */
#include <kernel.h>
#include <kbd.h>

// global device table
struct dev_ent dev_table[DEVNUM];

void deviceinit() {
  // setup echo keyboard
  dev_table[0].open = &kbd_open;
  dev_table[0].close = &kbd_close;
  dev_table[0].read = &kbde_read;
  dev_table[0].write = NULL;
  dev_table[0].ioctl = &kbd_ioctl;

  // setup no-echo keyboard
  dev_table[1].open = &kbd_open;
  dev_table[1].close = &kbd_close;
  dev_table[1].read = &kbd_read;
  dev_table[1].write = NULL;
  dev_table[1].ioctl = &kbd_ioctl;
}
