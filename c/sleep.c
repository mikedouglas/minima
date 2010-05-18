/* sleep.c : sleep device (assignment 2)
 */

#include <kernel.h>

#define PARENT(X) ((int)((X-1)/2))
#define CHILD_L(X) (2*X+1)
#define CHILD_R(X) (2*X+2)

// the sleep queue is implemented as a binary min heap.
struct queue_item {
  struct proc_ctrl_blk *item;
  int val;
};

// using an array to implement the heap
struct queue_item sleep_queue[PROCS_NUM];
unsigned int size = 0;

static void swap(int i, int j) {
  struct queue_item temp;
  temp.item = sleep_queue[i].item;
  temp.val = sleep_queue[i].val;

  sleep_queue[i].item = sleep_queue[j].item;
  sleep_queue[i].val = sleep_queue[j].val;

  sleep_queue[j].item = temp.item;
  sleep_queue[j].val = temp.val;
}

static void insert(struct proc_ctrl_blk *p, int val) {
  struct queue_item new;
  new.item = p;
  new.val = val;

  int loc = size++;
  sleep_queue[loc].item = p;
  sleep_queue[loc].val = val;

  while (loc != 0 && sleep_queue[PARENT(loc)].val > val) {
    swap(PARENT(loc), loc);
    loc = PARENT(loc);
  }
}

static void min_heapify(int i) {
  int largest = i;

  if (CHILD_L(i) < size && sleep_queue[CHILD_L(i)].val < sleep_queue[i].val)
    largest = CHILD_L(i);
  
  if (CHILD_R(i) < size && sleep_queue[CHILD_R(i)].val < sleep_queue[i].val)
    largest = CHILD_R(i);

  if (largest != i) {
    swap(i, largest);
    min_heapify(largest);
  }
}

static struct proc_ctrl_blk *pop() {
  // save top of heap, to return later
  struct proc_ctrl_blk *top = sleep_queue[0].item;

  sleep_queue[0].item = sleep_queue[size-1].item;
  sleep_queue[0].val = sleep_queue[size-1].val;

  min_heapify(0);
  size--;
  return top;
}

int sleep(struct proc_ctrl_blk *proc, int len) {
  insert(proc, len);
  return len;
}

void tick() {
  int i;
  for(i = 0; i < size; i++) {
    sleep_queue[i].val--;
  }

  struct proc_ctrl_blk *p;
  while(size > 0 && sleep_queue[0].val < 1) {
    p = pop();
    add_to_ready(p);
  }
}
