/* initialize.c - initproc */

#include <i386.h>
#include <kernel.h>

extern int  entry();  /* start of kernel image, use &start    */
extern int  end();    /* end of kernel image, use &end        */
extern long freemem;  /* start of free memory (set in i386.c) */
extern char *maxaddr; /* max memory address (set in i386.c)   */
extern void deviceinit();
extern void test();   /* testing functions */

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc()				/* The beginning */
{
  procinit();
  kmeminit();
  contextinit();
  deviceinit();
  kprintf("\n\nCPSC 415, 2010\nLocated at: %x to %x\n", &entry, &end);

  create(&idleproc, 4096);
  //create(&root, 4096);
  create(&test, 4096);

  dispatch();
}
