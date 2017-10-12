
#include <kernel.h>

PORT_DEF ports[MAX_PORTS];
PORT_DEF *next_free_port;

PORT create_port()
{
    return create_new_port(active_proc);
}


PORT create_new_port (PROCESS owner)
{
    assert(owner->magic == MAGIC_PCB);
    assert(next_free_port != NULL);
    PORT p;

    p = next_free_port;
    next_free_port = p->next;
    p->magic = MAGIC_PORT;
    p->used = TRUE;
    p->open = TRUE;
    p->owner = owner;
    p->blocked_list_head = NULL;
    p->blocked_list_tail = NULL;
    if (owner->first_port == NULL)
        p->next = NULL;
    else
        p->next = owner->first_port;
    owner->first_port = p;

    return p;
}



void open_port (PORT port)
{
    assert(port->magic == MAGIC_PORT);
    port->open = TRUE;
}



void close_port (PORT port)
{
    assert(port->magic == MAGIC_PORT);
    port->open = FALSE;
}


void send (PORT dest_port, void* data)
{
}


void message (PORT dest_port, void* data)
{
}



void* receive (PROCESS* sender)
{
}


void reply (PROCESS sender)
{
}


void init_ipc()
{
    int i;

    /* Initialize ports array */
    for (i = 0; i < MAX_PORTS; i++) {
        ports[i].magic = MAGIC_PORT;
        ports[i].used = FALSE;
        ports[i].open = FALSE;
    }

    /* Create list of available pors */
    for (i = 0; i < MAX_PORTS - 1; i++)
        ports[i].next = &ports[i + 1];
    ports[MAX_PORTS - 1].next = NULL;
    next_free_port = ports;
}
