
#include <kernel.h>


PCB pcb[MAX_PROCS];
PCB *next_free_pcb;

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
	assert(next_free_pcb != NULL);
	assert(prio >= 0 && prio <= MAX_READY_QUEUES);
	PROCESS new_proc;
	MEM_ADDR esp;

	new_proc = next_free_pcb;
	next_free_pcb = next_free_pcb->next;

	/* Initiliaze PCB entry */
	new_proc->magic 		= MAGIC_PCB;
	new_proc->used 			= TRUE;
	new_proc->state 		= STATE_READY;
	new_proc->priority		= prio;
	new_proc->first_port	= NULL;
	new_proc->name			= name;

	/* Computes starting address of process' stack frame */
	esp = 640 * 1024 - (new_proc - pcb) * 16 * 1024;

	/* Initialize stack frame */
	esp -= 4;
	poke_l(esp, param);				/* PARAM */
	esp -= 4;
	poke_l(esp, new_proc);			/* self */
	esp -= 4;
	poke_l(esp, 0);					/* dummy return address*/
	esp -= 4;
	poke_l(esp, ptr_to_new_proc);	/* EIP */
	esp -= 4;
	poke_l(esp, 0);					/* EAX */
	esp -= 4;
	poke_l(esp, 0);					/* ECX */
	esp -= 4;
	poke_l(esp, 0);					/* EDX */
	esp -= 4;
	poke_l(esp, 0);					/* EBX */
	esp -= 4;
	poke_l(esp, 0);					/* EBP */
	esp -= 4;
	poke_l(esp, 0);					/* ESI */
	esp -= 4;
	poke_l(esp, 0);					/* EDI */

	/* Save process stack pointer */
	new_proc->esp = esp;

	add_ready_queue(new_proc);

	return NULL;
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}




void print_process(WINDOW* wnd, PROCESS p)
{
}

void print_all_processes(WINDOW* wnd)
{
}



void init_process()
{
	/* Initialize boot process */
	pcb[0].magic = MAGIC_PCB;
	pcb[0].used = TRUE;
	pcb[0].state = STATE_READY;
	pcb[0].priority = 1;
	pcb[0].name = "Boot process";
	active_proc = pcb;

	int i;
	/* Initialize remaining PCBs */
	for (i = 1; i < MAX_PROCS; i++) {
		pcb[i].magic = 0;
		pcb[i].used = FALSE;
	}

	/* create list of available PCBs */
	for (i = 1; i < MAX_PROCS - 1; i++)
		pcb[i].next = &pcb[i + 1];
	pcb[MAX_PROCS - 1].next = NULL;
	next_free_pcb = &pcb[1];
}
