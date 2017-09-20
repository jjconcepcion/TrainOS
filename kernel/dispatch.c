
#include <kernel.h>

#include "disptable.c"


PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 */
PCB *ready_queue [MAX_READY_QUEUES];



/*
 * insert_ready_queue
 *----------------------------------------------------------------------------
 * Insertion of proc into ready_queue maintains the doubly-linked list
 */

void insert_ready_queue(PROCESS proc) {
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
}

/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by proc is put the ready queue.
 * The appropiate ready queue is determined by proc->priority.
 */

void add_ready_queue (PROCESS proc)
{
    assert(proc != NULL);
    proc->state = STATE_READY;
    insert_ready_queue(proc);
}



/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc)
{
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
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher()
{
}
