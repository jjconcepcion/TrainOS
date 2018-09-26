#include <kernel.h>

BOOL interrupts_initialized = FALSE;

IDT idt [MAX_INTERRUPTS];
PROCESS interrupt_table [MAX_INTERRUPTS];


void load_idt (IDT* base)
{
    unsigned short           limit;
    volatile unsigned char   mem48 [6];
    volatile unsigned       *base_ptr;
    volatile short unsigned *limit_ptr;

    limit      = MAX_INTERRUPTS * IDT_ENTRY_SIZE - 1;
    base_ptr   = (unsigned *) &mem48[2];
    limit_ptr  = (short unsigned *) &mem48[0];
    *base_ptr  = (unsigned) base;
    *limit_ptr = limit;
    asm ("lidt %0" : "=m" (mem48));
}


void init_idt_entry (int intr_no, void (*isr) (void))
{
    idt[intr_no].offset_0_15    = (unsigned) isr & 0xFFFF;
    idt[intr_no].selector       = CODE_SELECTOR;
    idt[intr_no].dword_count    = 0;
    idt[intr_no].unused         = 0;
    idt[intr_no].type           = 0xE;
    idt[intr_no].dt             = 0;
    idt[intr_no].dpl            = 0;
    idt[intr_no].p              = 1;
    idt[intr_no].offset_16_31   = ((unsigned) isr >> 16) && 0xFFFF;
}


void fatal_isr(int n)
{
    WINDOW error_window = {0, 24, 80, 1, 0, 0, ' '};

    wprintf (&error_window, "Fatal interrupt %d (%s)", n, active_proc->name);
    assert(0);
}


void interrupt0 ()
{
    fatal_isr(0);
}



void interrupt1 ()
{
    fatal_isr(1);
}



void interrupt2 ()
{
    fatal_isr(2);
}


void interrupt3 ()
{
    fatal_isr(3);
}


void interrupt4 ()
{
    fatal_isr(4);
}


void interrupt5 ()
{
    fatal_isr(5);
}


void interrupt6 ()
{
    fatal_isr(6);
}


void interrupt7 ()
{
    fatal_isr(7);
}


void interrupt8 ()
{
    fatal_isr(8);
}


void interrupt9 ()
{
    fatal_isr(9);
}


void interrupt10 ()
{
    fatal_isr(10);
}


void interrupt11 ()
{
    fatal_isr(11);
}



void interrupt12 ()
{
    fatal_isr(12);
}


void interrupt13 ()
{
    fatal_isr(13);
}


void interrupt14 ()
{
    fatal_isr(14);
}



void interrupt15 ()
{
    fatal_isr(15);
}


void interrupt16 ()
{
    fatal_isr(16);
}


void isr();
void dummy_isr ()
{
    asm ("isr:");
    asm ("push %eax; push %ecx; push %edx");
    asm ("push %ebx; push %ebp; push %esi; push %edi");

    /* react to the interrupt */

    /* reset interrupt controller */
    asm ("movb $0x20,%al");
    asm ("outb %al,$0x20");

    asm ("pop %edi; pop %esi; pop %ebp; pop %ebx");
    asm ("pop %edx; pop %ecx; pop %eax");
    asm ("iret");
}




/*
 * Timer ISR
 */
void isr_timer ();
void isr_timer_wrapper()
{
    /*
     *	PUSHL	%EAX		; Save process' context
     *  PUSHL   %ECX
     *  PUSHL   %EDX
     *  PUSHL   %EBX
     *  PUSHL   %EBP
     *  PUSHL   %ESI
     *  PUSHL   %EDI
     */
    asm ("isr_timer:");
    asm ("pushl %eax;pushl %ecx;pushl %edx");
    asm ("pushl %ebx;pushl %ebp;pushl %esi;pushl %edi");
    /* Save the context pointer ESP to the PCB */
    asm ("movl %%esp,%0" : "=m" (active_proc->esp) : );
    /* Call the actual implementation of the ISR */
    asm ("call isr_timer_impl");
    /* Restore context pointer ESP */
    asm ("movl %0,%%esp" : : "m" (active_proc->esp) );
    /*
     *	MOVB  $0x20,%AL	; Reset interrupt controller
     *	OUTB  %AL,$0x20
     *	POPL  %EDI      ; Restore previously saved context
     *  POPL  %ESI
     *  POPL  %EBP
     *  POPL  %EBX
     *  POPL  %EDX
     *  POPL  %ECX
     *  POPL  %EAX
     *	IRET		; Return to new process
     */
    asm ("movb $0x20,%al;outb %al,$0x20");
    asm ("popl %edi;popl %esi;popl %ebp;popl %ebx");
    asm ("popl %edx;popl %ecx;popl %eax");
    asm ("iret");
}

void isr_timer_impl ()
{
    PROCESS p;

    /* Add waiting process back on the ready queu */
    p = interrupt_table[TIMER_IRQ];
    if (p && p->state == STATE_INTR_BLOCKED)
        add_ready_queue(p);

    /* Dispatch new process */
    active_proc = dispatcher();
}



/*
 * COM1 ISR
 */
void isr_com1 ();
void wrapper_isr_com1 ()
{
    /*
     *	PUSHL	%EAX		; Save process' context
     *  PUSHL   %ECX
     *  PUSHL   %EDX
     *  PUSHL   %EBX
     *  PUSHL   %EBP
     *  PUSHL   %ESI
     *  PUSHL   %EDI
     */
    asm ("isr_com1:");
    asm ("pushl %eax;pushl %ecx;pushl %edx");
    asm ("pushl %ebx;pushl %ebp;pushl %esi;pushl %edi");
    /* Save the context pointer ESP to the PCB */
    asm ("movl %%esp,%0" : "=m" (active_proc->esp) : );
    /* Call the actual implementation of the ISR */
    asm ("call isr_com1_impl");
    /* Restore context pointer ESP */
    asm ("movl %0,%%esp" : : "m" (active_proc->esp) );
    /*
     *	MOVB  $0x20,%AL	; Reset interrupt controller
     *	OUTB  %AL,$0x20
     *	POPL  %EDI      ; Restore previously saved context
     *  POPL  %ESI
     *  POPL  %EBP
     *  POPL  %EBX
     *  POPL  %EDX
     *  POPL  %ECX
     *  POPL  %EAX
     *	IRET		; Return to new process
     */
    asm ("movb $0x20,%al;outb %al,$0x20");
    asm ("popl %edi;popl %esi;popl %ebp;popl %ebx");
    asm ("popl %edx;popl %ecx;popl %eax");
    asm ("iret");
}

void isr_com1_impl()
{
    PROCESS p = interrupt_table[COM1_IRQ];

    if (p == NULL) {
	panic ("service_intr_0x64: Spurious interrupt");
    }

    if (p->state != STATE_INTR_BLOCKED) {
	panic ("service_intr_0x64: No process waiting");
    }

    /* Add event handler to ready queue */
    add_ready_queue (p);

    active_proc = dispatcher();
}


/*
 * Keyboard ISR
 */
void isr_keyb();
void wrapper_isr_keyb()
{
    /*
     *	PUSHL	%EAX		; Save process' context
     *  PUSHL   %ECX
     *  PUSHL   %EDX
     *  PUSHL   %EBX
     *  PUSHL   %EBP
     *  PUSHL   %ESI
     *  PUSHL   %EDI
     */
    asm ("isr_keyb:");
    asm ("pushl %eax;pushl %ecx;pushl %edx");
    asm ("pushl %ebx;pushl %ebp;pushl %esi;pushl %edi");
    /* Save the context pointer ESP to the PCB */
    asm ("movl %%esp,%0" : "=m" (active_proc->esp) : );
    /* Call the actual implementation of the ISR */
    asm ("call isr_keyb_impl");
    /* Restore context pointer ESP */
    asm ("movl %0,%%esp" : : "m" (active_proc->esp) );
    /*
     *	MOVB  $0x20,%AL	; Reset interrupt controller
     *	OUTB  %AL,$0x20
     *	POPL  %EDI      ; Restore previously saved context
     *  POPL  %ESI
     *  POPL  %EBP
     *  POPL  %EBX
     *  POPL  %EDX
     *  POPL  %ECX
     *  POPL  %EAX
     *	IRET		; Return to new process
     */
    asm ("movb $0x20,%al;outb %al,$0x20");
    asm ("popl %edi;popl %esi;popl %ebp;popl %ebx");
    asm ("popl %edx;popl %ecx;popl %eax");
    asm ("iret");
}

void isr_keyb_impl()
{
    PROCESS p = interrupt_table[KEYB_IRQ];

    if (p == NULL) {
	panic ("service_intr_0x61: Spurious interrupt");
    }

    if (p->state != STATE_INTR_BLOCKED) {
	panic ("service_intr_0x61: No process waiting");
    }

    /* Add event handler to ready queue */
    add_ready_queue (p);

    active_proc = dispatcher();
}

void wait_for_interrupt (int intr_no)
{
    volatile int saved_if;

    DISABLE_INTR (saved_if);
    if (interrupt_table[intr_no] != NULL)
        panic ("wait_for_interrupt(): ISR busy");
    interrupt_table[intr_no] = active_proc;
    remove_ready_queue(active_proc);
    active_proc->state = STATE_INTR_BLOCKED;
    resign();
    interrupt_table[intr_no] = NULL;
    ENABLE_INTR (saved_if);
}


void delay ()
{
    asm ("nop;nop;nop");
}

void re_program_interrupt_controller ()
{
    /* Shift IRQ Vectors so they don't collide with the
       x86 generated IRQs */

    // Send initialization sequence to 8259A-1
    asm ("movb $0x11,%al;outb %al,$0x20;call delay");
    // Send initialization sequence to 8259A-2
    asm ("movb $0x11,%al;outb %al,$0xA0;call delay");
    // IRQ base for 8259A-1 is 0x60
    asm ("movb $0x60,%al;outb %al,$0x21;call delay");
    // IRQ base for 8259A-2 is 0x68
    asm ("movb $0x68,%al;outb %al,$0xA1;call delay");
    // 8259A-1 is the master
    asm ("movb $0x04,%al;outb %al,$0x21;call delay");
    // 8259A-2 is the slave
    asm ("movb $0x02,%al;outb %al,$0xA1;call delay");
    // 8086 mode for 8259A-1
    asm ("movb $0x01,%al;outb %al,$0x21;call delay");
    // 8086 mode for 8259A-2
    asm ("movb $0x01,%al;outb %al,$0xA1;call delay");
    // Don't mask IRQ for 8259A-1
    asm ("movb $0x00,%al;outb %al,$0x21;call delay");
    // Don't mask IRQ for 8259A-2
    asm ("movb $0x00,%al;outb %al,$0xA1;call delay");
}

void init_interrupts()
{
    int i;

    assert(sizeof (IDT) == IDT_ENTRY_SIZE);

    load_idt(idt);

    for (i = 0; i < MAX_INTERRUPTS; i++)
    	init_idt_entry(i, dummy_isr);

    init_idt_entry(0, interrupt0);
    init_idt_entry(1, interrupt1);
    init_idt_entry(2, interrupt2);
    init_idt_entry(3, interrupt3);
    init_idt_entry(4, interrupt4);
    init_idt_entry(5, interrupt5);
    init_idt_entry(6, interrupt6);
    init_idt_entry(7, interrupt7);
    init_idt_entry(8, interrupt8);
    init_idt_entry(9, interrupt9);
    init_idt_entry(10, interrupt10);
    init_idt_entry(11, interrupt11);
    init_idt_entry(12, interrupt12);
    init_idt_entry(13, interrupt13);
    init_idt_entry(14, interrupt14);
    init_idt_entry(15, interrupt15);
    init_idt_entry(16, interrupt16);
    init_idt_entry (TIMER_IRQ, isr_timer);
    init_idt_entry (KEYB_IRQ, isr_keyb);
    init_idt_entry (COM1_IRQ, isr_com1);

    re_program_interrupt_controller();

    for (i = 0; i < MAX_INTERRUPTS; i++)
        interrupt_table[i] = NULL;

    interrupts_initialized = TRUE;
    asm ("sti");
}
