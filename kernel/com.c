/*
 * Internet ressources:
 *
 * http://workforce.cup.edu/little/serial.html
 *
 * http://www.lammertbies.nl/comm/info/RS-232.html
 *
 */


#include <kernel.h>

PORT com_port;


void init_uart()
{
    /* LineControl disabled to set baud rate */
    outportb (COM1_PORT + 3, 0x80);
    /* lower byte of baud rate */
    outportb (COM1_PORT + 0, 0x30);
    /* upper byte of baud rate */
    outportb (COM1_PORT + 1, 0x00);
    /* 8 Bits, No Parity, 2 stop bits */
    outportb (COM1_PORT + 3, 0x07);
    /* Interrupt enable*/
    outportb (COM1_PORT + 1, 1);
    /* Modem control */
    outportb (COM1_PORT + 4, 0x0b);
    inportb (COM1_PORT);
}


void com_reader_process (PROCESS self, PARAM param)
{
    PORT         reply_port;
    PROCESS      sender_proc;
    COM_Message *msg;
    int          i;

    reply_port = (PORT) param;
    while (1) {
         /* receive message from COM process */
        msg =  (COM_Message*) receive(&sender_proc);
        /* read as many bytes requested  */
        for (i = 0; i < msg->len_input_buffer; i++) {
            wait_for_interrupt(COM1_IRQ);
            msg->input_buffer[i] = inportb(COM1_PORT);
        }
        /* signal to COM process that all bytes have been read */
        message(reply_port, &msg);
    }
}


void send_cmd_to_com (char* cmd)
{
    while (*cmd != '\0') {
    	/* Wait for the UART to accept the next byte */
    	while (!(inportb (COM1_PORT + 5) & (1 << 5))) ;
        outportb (COM1_PORT, *cmd);
    	cmd++;
    }
}


void com_process (PROCESS self, PARAM param)
{

    PORT com_reader_port;
    COM_Message *msg;
    PROCESS sender_proc,
            recv_proc;

    com_reader_port = create_process(com_reader_process, 7,
                    (PARAM) self->first_port, "COM reader");

    while (1) {
        /* receive message from user process. */
        msg = (COM_Message*) receive(&sender_proc);
        /* forward message to COM reader process */
        message(com_reader_port, msg);
        /* write all bytes contained in COM_Message.output_buffer to COM1 */
        send_cmd_to_com (msg->output_buffer);
        /* wait for message from COM reader process that signals that
           all bytes have been read */
        receive(&recv_proc);
        /* signal to user process that all I/O has been completed */
        reply(sender_proc);
    }

}



void init_com ()
{
    init_uart();
    com_port = create_process(com_process, 6, 0, "COM process");
    resign();
}
