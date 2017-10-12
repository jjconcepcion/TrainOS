
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


void add_to_send_block_list(PORT port, PROCESS proc) {
    assert(port->magic == MAGIC_PORT);
    assert(proc->magic == MAGIC_PCB);
    if (port->blocked_list_head == NULL)
        port->blocked_list_head = proc;
    else
        port->blocked_list_tail->next_blocked = proc;
    port->blocked_list_tail = proc;
    proc->next_blocked = NULL;
}


void send (PORT dest_port, void* data)
{
    PROCESS dest;
    assert(dest_port->magic == MAGIC_PORT);
    dest = dest_port->owner;
    assert(dest->magic == MAGIC_PCB);

    if (dest->state == STATE_RECEIVE_BLOCKED && dest_port->open) {
    /* Destination process waiting for message. Message is delivered
    immediately */
        dest->param_proc = active_proc;
        dest->param_data = data;
        active_proc->state = STATE_REPLY_BLOCKED;
        add_ready_queue(dest);
    } else {
    /* Destination process not ready to recieve message. Process is is queued
    on the block list */
        add_to_send_block_list(dest_port, active_proc);
        active_proc->param_data = data;
        active_proc->state = STATE_SEND_BLOCKED;
    }
    remove_ready_queue(active_proc);
    resign();
}


void message (PORT dest_port, void* data)
{
    PROCESS dest;
    assert(dest_port->magic == MAGIC_PORT);
    dest = dest_port->owner;
    assert(dest->magic == MAGIC_PCB);

    if (dest->state == STATE_RECEIVE_BLOCKED && dest_port->open) {
    /* Destination process waiting for message. Message is delivered
    immediately */
        dest->param_proc = active_proc;
        dest->param_data = data;
        add_ready_queue(dest);
    } else {
    /* Destination process not ready to recieve message. Process is is queued
    on the block list */
        add_to_send_block_list(dest_port, active_proc);
        active_proc->param_data = data;
        active_proc->state = STATE_MESSAGE_BLOCKED;
        remove_ready_queue(active_proc);
    }
    resign();
}


void* receive (PROCESS* sender)
{
    PROCESS send_proc;
    PORT port;
    void *data;

    port = active_proc->first_port;
    assert(port != NULL);
    send_proc = NULL;
    data = NULL;
    /* Scan ports owned for process on send block list*/
    while (port != NULL) {
        assert(port->magic == MAGIC_PORT);
        if (port->open && port->blocked_list_head != NULL) {
            send_proc = port->blocked_list_head;
            /* dequeue process found on send block list */
            port->blocked_list_head = port->blocked_list_head->next;
            if (port->blocked_list_head == NULL)
                port->blocked_list_tail = NULL;
            break;
        }
        port = port->next;
    }

    /* No available message. Reciever blocks until message arrives */
    if (send_proc == NULL) {
        active_proc->param_data = NULL;
        active_proc->state = STATE_RECEIVE_BLOCKED;
        remove_ready_queue(active_proc);
        resign();
        assert(active_proc->param_proc->magic == MAGIC_PCB);
        *sender = active_proc->param_proc;
        data = active_proc->param_data;
    } else {
    /* Message stored in blocked process PCB */
        assert(send_proc->magic == MAGIC_PCB);
        *sender = send_proc;
        data = send_proc->param_data;
        if (send_proc->state == STATE_MESSAGE_BLOCKED) {
            add_ready_queue(send_proc);
        }
        if (send_proc->state == STATE_SEND_BLOCKED) {
            send_proc->state = STATE_REPLY_BLOCKED;
        }
    }
    return data;
}


void reply (PROCESS sender)
{
    assert(sender->state == STATE_REPLY_BLOCKED);
    add_ready_queue(sender);
    resign();
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
