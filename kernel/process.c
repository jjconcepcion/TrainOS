
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
	PORT p;

	new_proc = next_free_pcb;
	next_free_pcb = next_free_pcb->next;

	/* Initiliaze PCB entry */
	new_proc->magic 		= MAGIC_PCB;
	new_proc->used 			= TRUE;
	new_proc->state 		= STATE_READY;
	new_proc->priority		= prio;
	new_proc->first_port	= NULL;
	new_proc->name			= name;
	/* new_proc gets first_port */
	create_new_port(new_proc);


	/* Computes starting address of process' stack frame */
	esp = 640 * 1024 - (new_proc - pcb) * 16 * 1024;

#define PUSH(x)    esp -= 4; \
                   poke_l (esp, (LONG) x);

    /* Initialize the stack for the new process */
    PUSH (param);		/* First data */
    PUSH (new_proc);		/* Self */
    PUSH (0);			/* dummy return address */
    PUSH (ptr_to_new_proc);	/* EIP */
    PUSH (0);			/* EAX */
    PUSH (0);			/* ECX */
    PUSH (0);			/* EDX */
    PUSH (0);			/* EBX */
    PUSH (0);			/* EBP */
    PUSH (0);			/* ESI */
    PUSH (0);			/* EDI */

#undef PUSH

	/* Save process stack pointer */
	new_proc->esp = esp;

	add_ready_queue(new_proc);

	return new_proc->first_port;
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}




void print_process_details(WINDOW* wnd, PROCESS p)
{
	static const char *state[] = {
		"READY          ",
		"SEND_BLOCKED   ",
		"REPLY_BLOCKED  ",
		"RECEIVE_BLOCKED",
		"MESSAGE_BLOCKED",
		"INTR_BLOCKED   "
	};

	output_string(wnd, state[p->state]);
	if (p == active_proc)
		output_string(wnd, " *       ");
	else
		output_string(wnd, "         ");
	/* print priority in ASCII */
	output_char(wnd, p->priority + 48);
	output_string(wnd, " ");
	output_string(wnd, p->name);
	output_char(wnd, '\n');

}


void print_process_header(WINDOW* wnd) {
	output_string(wnd, "State           Active Prio Name\n");
	output_string(wnd, "---------------------------------------\n");
}


void print_process(WINDOW* wnd, PROCESS p) {
	print_process_header(wnd);
	print_process_details(wnd, p);
}


void print_all_processes(WINDOW* wnd)
{
	int i;

	print_process_header(wnd);
	for (i = 0; i < MAX_PROCS; i++) {
		if (pcb[i].used == TRUE)
			print_process_details(wnd, &pcb[i]);
	}
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
