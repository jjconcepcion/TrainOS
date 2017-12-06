
#include <kernel.h>

#define TRAIN_ID "20\0"
#define CMD_DELAY_TIME 15
#define BUFFER_SIZE 8
#define CMD_TERMINATOR '\015'


void strcat(char *dest, char *src)
{
    int i, j;

    i = j = 0;
    while (dest[i] != '\0')
        i++;
    while(src[j] != '\0') {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';
}

void send_train_command(char *cmd, char *response, int response_len)
{
    COM_Message msg;

    msg.output_buffer = cmd;
    msg.input_buffer = response;
    msg.len_input_buffer = response_len;

    sleep(CMD_DELAY_TIME);
    send(com_port, &msg);
}


void set_switch(char *id, char *setting)
{
    char cmd[BUFFER_SIZE];

    cmd[0] = '\0';
    strcat(cmd, "M\0");
    strcat(cmd, id);
    strcat(cmd, setting);
    cmd[k_strlen(cmd)] = CMD_TERMINATOR;

    send_train_command(cmd, 0, 0);
}


void set_train_speed(char *speed)
{
    char cmd[BUFFER_SIZE];

    cmd[0] = '\0';
    strcat(cmd, "L\0");
    strcat(cmd, TRAIN_ID);
    strcat(cmd, "S\0");
    strcat(cmd, speed);
    cmd[k_strlen(cmd)] = CMD_TERMINATOR;

    send_train_command(cmd, 0, 0);
}

void set_outer_loop_switches()
{
    set_switch("5\0", "G\0");
    set_switch("8\0", "G\0");
    set_switch("9\0", "R\0");
    set_switch("4\0", "G\0");
    set_switch("1\0", "G\0");
}


void train_process(PROCESS self, PARAM param)
{
    int window_id;


    window_id = wm_create(5, 5, 40, 15);
    wm_print(window_id, "TOS Train Application\n");

    init_train();

    while(1) {
    }
}


void init_train()
{
    /* Ensure rogue train doesn't run of track */
    set_outer_loop_switches();
}

void start_train_app()
{
    create_process(train_process, 5, 0, "Train process");
    resign();
}
