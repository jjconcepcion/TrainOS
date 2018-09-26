#include <kernel.h>

PORT timer_port;

void timer_notifier(PROCESS self, PARAM param)
{
    while(1) {
        wait_for_interrupt(TIMER_IRQ);
        message(timer_port, 0);
    }
}

void timer_process(PROCESS self, PARAM param)
{
    int ticks_remaining[MAX_PROCS];
    int i;
    Timer_Message *msg;
    PROCESS sender;

    for (i = 0; i < MAX_PROCS; i++)
        ticks_remaining[i] = 0;

    create_process(timer_notifier, 7, 0, "Timer notifier");

    while(1) {
        msg = (Timer_Message*) receive(&sender);
        /* Timer notifier message */
        if (msg == NULL) {
            for (i = 0; i < MAX_PROCS; i++)
            {
                if (ticks_remaining[i] == 0)
                    continue;
                /* Wake up client process */
                if (--ticks_remaining[i] == 0)
                    reply(&pcb[i]);
            }
        } else {
        /* Message from client process */
            i = sender - pcb;
            assert(sender == &pcb[i] && pcb[i].magic == MAGIC_PCB);
            ticks_remaining[i] = msg->num_of_ticks;
        }

    }
}


void sleep(int ticks)
{
    Timer_Message msg;
    msg.num_of_ticks = ticks;
    send(timer_port, &msg);
}


void init_timer ()
{
    timer_port = create_process(timer_process, 6, 0, "Timer process");
    resign();
}
