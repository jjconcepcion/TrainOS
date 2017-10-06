
#include <kernel.h>

#include "disptable.c"


PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 */
PCB *ready_queue [MAX_READY_QUEUES];



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
    PROCESS queue_head, queue_tail;

    queue_head = ready_queue[proc->priority];
    if (queue_head == NULL) {
    /* insert into empty queue */
        ready_queue[proc->priority] = proc;
        proc->prev = proc->next = proc;
    } else {
    /* insert into tail of queue */
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
    PROCESS queue_head,
            queue_tail,
            queue_new_head;

    queue_head = ready_queue[proc->priority];
    if (proc == queue_head) {
        if (proc == queue_head->next) {
        /* remove solitary process */
            ready_queue[proc->priority] = NULL;
        } else {
            queue_new_head = queue_head->next;
            queue_tail = queue_head->prev;
            queue_new_head->prev = queue_tail;
            queue_tail->next = queue_new_head;
            ready_queue[proc->priority] = queue_new_head;
        }
    } else {
    /* removal of process not at head of queue */
        proc->prev->next = proc->next;
        proc->next->prev = proc->prev;
    }

    /* unset dangling links */
    proc->prev = proc->next = NULL;
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
    PROCESS new_proc;
    int p, prio;

    for (p = 0; p < MAX_READY_QUEUES; p++) {
        if (ready_queue[p] != NULL)
            prio = p;
    }

    if (prio == active_proc->priority)
        new_proc = active_proc->next;
    else
        new_proc = ready_queue[prio];

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

    /* Add first process */
    add_ready_queue(active_proc);
}
