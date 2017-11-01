
#include <kernel.h>

#include "disptable.c"


PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 */
PCB *ready_queue [MAX_READY_QUEUES];

/*
 * The bits in ready_procs tell which ready queue is empty.
 * The MSB of ready_procs corresponds to ready_queue[7].
 */
unsigned ready_procs;

/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by proc is put the ready queue.
 * The appropiate ready queue is determined by proc->priority.
 */

void add_ready_queue (PROCESS proc)
{
    assert(proc != NULL);
    assert(proc->magic == MAGIC_PCB);
    assert(proc->priority >= 0
        && proc->priority < MAX_READY_QUEUES);
    int prio;
    PROCESS queue_head, queue_tail;

    prio = proc->priority;
    queue_head = ready_queue[prio];
    if (queue_head == NULL) {
	/* The only process on this priority level */
        ready_queue[prio] = proc;
        proc->prev = proc->next = proc;
    	ready_procs |= 1 << prio;
    } else {
	/* Some other processes on this priority level */
        queue_tail = queue_head->prev;
        queue_tail->next = proc;
        proc->prev = queue_tail;
        proc->next = queue_head;
        queue_head->prev = proc;
    }
    proc->state = STATE_READY;
}



/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc)
{
    assert(proc != NULL);
    assert(proc->magic == MAGIC_PCB);
    assert(proc->priority >= 0
        && proc->priority < MAX_READY_QUEUES);
    int prio;

    prio = proc->priority;
    if (proc == proc->next) {
        /* remove solitar   y process */
        ready_queue[prio] = NULL;
    	ready_procs &= ~(1 << prio);
    } else {
    	ready_queue [prio] = proc->next;
    	proc->next->prev   = proc->prev;
    	proc->prev->next   = proc->next;
    }
}



/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PROCESS dispatcher()
{
    PROCESS      new_proc;
    unsigned     i;

    /* Find queue with highest priority that is not empty */
    i = table[ready_procs];
    assert (i != -1);
    if (i == active_proc->priority)
	/* Round robin within the same priority level */
	new_proc = active_proc->next;
    else
	/* Dispatch a process at a different priority level */
	new_proc = ready_queue [i];
    return new_proc;
}



/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */
void resign()
{
    /* Save context of current process */
    asm("pushl %eax");
    asm("pushl %ecx");
    asm("pushl %edx");
    asm("pushl %ebx");
    asm("pushl %ebp");
    asm("pushl %esi");
    asm("pushl %edi");
    /* Save the stack pointer to the PCB */
    asm ("movl %%esp,%0" : "=r" (active_proc->esp) : );
    /* Select a new process to run */
    active_proc = dispatcher();
    /* Load the stack pointer from the PCB */
    asm ("movl %0,%%esp" : : "r" (active_proc->esp));
    /* Restore context of new process */
    asm("popl %edi");
    asm("popl %esi");
    asm("popl %ebp");
    asm("popl %ebx");
    asm("popl %edx");
    asm("popl %ecx");
    asm("popl %eax");
    /*return to new process  */
    asm("ret");
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher()
{
    int i;
    for(i = 0; i < MAX_READY_QUEUES; i++) {
        ready_queue[i] = NULL;
    }

    ready_procs = 0;
    /* Add first process */
    add_ready_queue(active_proc);
}
