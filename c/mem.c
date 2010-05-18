/* mem.c : memory manager
 */

#include <i386.h>
#include <kernel.h>

/*
 * MEMORY LAYOUT
 * =============
 * 0x000000-freemem : reserved
 * freemem-HOLESTART : available
 * HOLESTART-HOLEEND : reserved
 * HOLEEND-maxaddr : available
 */

extern unsigned char *maxaddr; // set by i386.c
extern long freemem;

struct mem_header *free_lst;

void kmeminit() {
  // after the hole
  free_lst = (struct mem_header*)HOLEEND;
  free_lst->size = 0x50000;
  free_lst->prev = NULL;

  // before the hole
  /*
  free_lst->next = (struct mem_header*)freemem;
  free_lst->next->prev = free_lst;
  free_lst->next->size = HOLESTART - (int)free_lst->next->data;
  free_lst->next->next = NULL;
  */
}

void *kmalloc(unsigned int size) {
  if (!free_lst)
    goto out_of_mem;

  unsigned int size_blks = size/16 + ((size%16)?1:0);
  unsigned int to_alloc = size_blks*16 + sizeof(struct mem_header);

  // first-fit algorithm
  struct mem_header *cur;
  for(cur = free_lst; cur != NULL && cur->size < size_blks*16; cur = cur->next);

  if (cur) {
    struct mem_header *new = cur;

    if (cur->size > to_alloc) { // must lay down a new mem_header
      cur = cur + to_alloc;
      cur->size = new->size - to_alloc;

      cur->prev = new->prev;
      cur->next = new->next;
      if (cur->next) cur->next->prev = cur;
      if (cur->prev) cur->prev->next = cur;

      if (new == free_lst)
        free_lst = cur;
    }
    else { 
      if (new->prev) new->prev->next = new->next;
      if (new->next) new->next->prev = new->prev;

      if (new == free_lst)
        free_lst = cur->next;
    }

    new->sanity = (char*)0xBEEF;
    new->size = size_blks*16;
    new->prev = NULL;
    new->next = NULL;

    return new->data;
  }
  else {
    // SHOULD coalesce contiguous free blks
out_of_mem:
    kprintf("OUT OF MEMORY!\n");
    return NULL;
  }
}

void kfree(void *ptr) {
  struct mem_header *mem = (struct mem_header*)((char*)ptr - sizeof(struct mem_header));

  if (mem->sanity != (char*)0xBEEF) return;
  if (free_lst == NULL) {
    free_lst = mem;
    return;
  }
  
  struct mem_header *last_free;
  for (last_free = free_lst; last_free->next != NULL;
       last_free = last_free->next);

  last_free->next = mem;
  mem->prev = last_free;
  mem->next = NULL;
}

void prnfmem() {
  struct mem_header *i = free_lst;
  int x = 0;
  kprintf("NUM: %d\nSIZE: 0x%x\nLOC: 0x%x\n", x, i->size, i);
/*  
  for (i = free_lst; i != NULL; i = i->next) {
    kprintf("NUM: %d\nSIZE: 0x%x\nLOC: 0x%x\n", x, i->size, i);
    x++;
  }*/
}
