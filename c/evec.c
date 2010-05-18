/* evec.c -- initevec, set_evec */

#include <i386.h>    
#include <string.h>

extern struct idt idt[256];
extern struct segtr idtr;

extern long	defevec[];
extern int	userret();

initevec()
{
	int		i;

        /* bzero( idt, sizeof( struct idt ) * 256 ); */
	memset( idt, 0, sizeof( struct idt ) * 256 ); 

	for (i=0; i<NID; ++i)
		set_evec(i, (long)defevec[i]);

	lidt();
	init8259();

}


/*------------------------------------------------------------------------
 * set_evec - set exception vector to point to an exception handler
 *------------------------------------------------------------------------
 */
int set_evec(xnum, handler)
unsigned int	xnum;
unsigned long	handler;	/* int value of function pointer */
{
	struct	idt	*pidt;

	pidt = &idt[xnum];
	pidt->igd_loffset = handler;
	pidt->igd_segsel = 0x8;		/* kernel code segment */
	pidt->igd_mbz = 0;
	pidt->igd_type = IGDT_TRAPG;
	pidt->igd_dpl = 0;
	pidt->igd_present = 1;
	pidt->igd_hoffset = handler >> 16;
}

char *inames[17] = {
	"divided by zero",
	"debug exception",
	"NMI interrupt",
	"breakpoint",
	"overflow",
	"bounds check failed",
	"invalid opcode",
	"coprocessor not available",
	"double fault",
	"coprocessor segment overrun",
	"invalid TSS",
	"segment not present",
	"stack fault",
	"general protection violation",
	"page fault",
	"coprocessor error",
};

static long *fp;

/*------------------------------------------------------------------------
 * trap -- print some debugging info when a trap occurred 
 * Note: Need some more work.
 *------------------------------------------------------------------------
 */
trap(inum)
int	inum;
{
	long		*sp, *fp;

	disable();
	asm("movl	%ebp,fp");

	sp = fp + 15;	/* eflags/CS/eip/ebp/regs/trap#/Xtrap/ebp */

	kprintf("trap!\n");
	if (inum < 16) {
		kprintf("exception %d (%s)\n", inum, inames[inum] );
	} else {
		kprintf("exception %d\n", inum );
	}

	kprintf("eflags %X ", *sp--);
	sp--;	/* skip OLD CS */
	kprintf("eip %X ", *sp);
	sp--;
	if (inum == 8 || (inum >= 10 && inum <= 14)) {
		kprintf("error code %08x (%u)\n", *sp, *sp);
		sp--;
	}
	sp--;	/* skip FP for Xint call */
	kprintf("register dump:\n");
	kprintf("eax %08X (%u)\n", *sp, *sp);
	sp--;
	kprintf("ecx %08X (%u)\n", *sp, *sp);
	sp--;
	kprintf("edx %08X (%u)\n", *sp, *sp);
	sp--;
	kprintf("ebx %08X (%u)\n", *sp, *sp);
	sp--;
	kprintf("esp %08X (%u)\n", *sp, *sp);
	sp--;
	kprintf("ebp %08X (%u)\n", *sp, *sp);
	fp = sp;
	sp--;
	kprintf("esi %08X (%u)\n", *sp, *sp);
	sp--;
	kprintf("edi %08X (%u)\n", *sp, *sp);
	sp--;

	kprintf("\nHalting.....\n");
        for(;;);
}
