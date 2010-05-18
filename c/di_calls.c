/* device independent calls */
#include <kernel.h>

int get_fd(struct proc_ctrl_blk *p) {
  for (int i = 0; i < FD_NUM; i++) {
    if (p->fd_used & (1 << i))
      continue;
    return i;
  }
  return -1;
}

int valid_fd(struct proc_ctrl_blk *cproc, int fd) {
  if ((fd < 0) || (fd > 3))
    return 0;
  else
    return cproc->fd[fd].status;
}

int valid_devno(int devno) {
  if ((devno < 0) || (devno >= DEVNUM))
    return 0;
  else
    return 1;
}

int di_open(struct proc_ctrl_blk *cproc, int devno) {
  int fd = get_fd(cproc);
  if (fd == -1 || !valid_devno(devno))
    return SYSERR;

  cproc->fd_used |= 1 << fd;
  cproc->fd[fd].status = 1;
  cproc->fd[fd].dev = &dev_table[devno];
  
  int ret = (*(cproc->fd[fd].dev->open))();

  // device already open
  if (ret == -1) {
    cproc->fd_used |= 1 << fd;
    cproc->fd[fd].status = 0;
    fd = -1;
  }

  return fd;
}

int di_close(struct proc_ctrl_blk *cproc, int fd) {
  cproc->fd[fd].status = 0;
  cproc->fd_used = cproc->fd_used - (1 << fd);
  return (*(cproc->fd[fd].dev->close))();
}

int di_write(struct proc_ctrl_blk *cproc, int fd, void *buf, int buflen) {
  // 1. verify fd is valid
  // 2. fd -> (*write_func)(buf, buflen)
  // 3. return correct value
  if (!valid_fd(cproc, fd))
    return SYSERR;
  return (*(cproc->fd[fd].dev->write))(buf, buflen);
}

int di_read(struct proc_ctrl_blk *cproc, int fd, void *buf, int buflen) {
  // 1. verify fd is valid
  // 2. fd -> (*read_func)(buf, buflen)
  // 3. return correct value
  if (!valid_fd(cproc, fd))
    return SYSERR;
  return (*(cproc->fd[fd].dev->read))(buf, buflen);
}

int di_ioctl(struct proc_ctrl_blk *cproc, int fd, unsigned long cmd, va_list args) {
  // 1. verify fd is valid
  // 2. fd -> (*ioctl_func)(...)
  // 3. return correct value
  if (!valid_fd(cproc, fd))
    return SYSERR;

  return (*(cproc->fd[fd].dev->ioctl))(cmd, args);
}
