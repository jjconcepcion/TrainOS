
#include <kernel.h>


PCB pcb[MAX_PROCS];


PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
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
}
