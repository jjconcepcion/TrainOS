
#include <kernel.h>

#define TRAIN_ID "20\0"
#define CMD_DELAY_TIME 15
#define BUFFER_SIZE 8
#define CMD_TERMINATOR '\015'
#define OCCUPIED 1
#define UNOCCUPIED 0


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


/*
 * Handles procedure for sending commands to model train via serial port
 */
void send_train_command(char *cmd, char *response, int response_len)
{
    COM_Message msg;

    msg.output_buffer = cmd;
    msg.input_buffer = response;
    msg.len_input_buffer = response_len;

    sleep(CMD_DELAY_TIME);
    send(com_port, &msg);
}

/*
 * Change setting of model train track switch.
 * command format: "M{id}{setting}\015"
 *      where {id} is the switch id and {setting} is either R or G
 */
void set_switch(char *id, char *setting)
{
    char cmd[BUFFER_SIZE];

    k_memset(cmd, 0, BUFFER_SIZE);
    cmd[0] = 'M';
    cmd[1] = '\0';
    strcat(cmd, id);
    strcat(cmd, setting);
    cmd[k_strlen(cmd)] = CMD_TERMINATOR;

    send_train_command(cmd, 0, 0);
}

/*
 * Change the speed of the model train which we control
 * command format: "L{TRAIN_ID}S#\015"
 *      where {TRAIN_ID} is 20 and # is a number 0-5
 */
void set_train_speed(char *speed)
{
    char cmd[BUFFER_SIZE];

    k_memset(cmd, 0, BUFFER_SIZE);
    cmd[0] = 'L';
    cmd[1] = '\0';
    strcat(cmd, TRAIN_ID);
    strcat(cmd, "S\0");
    strcat(cmd, speed);
    cmd[k_strlen(cmd)] = CMD_TERMINATOR;

    send_train_command(cmd, 0, 0);
}

/*
 * Change the direction of travel of the model train which we control
 * command format: "L{TRAIN_ID}D\015"
 *      where {TRAIN_ID} is 20
 */
void change_direction()
{
    char cmd[BUFFER_SIZE];

    k_memset(cmd, 0, BUFFER_SIZE);
    cmd[0] = 'L';
    cmd[1] = '\0';
    strcat(cmd, TRAIN_ID);
    strcat(cmd, "D\0");
    cmd[k_strlen(cmd)] = CMD_TERMINATOR;

    send_train_command(cmd, 0, 0);
}


/*
 * Clears the memory buffer of the s88 which controls the model train contacts
 * command format: "R\015"
 */
void clear_s88_buffer()
{
    char cmd[BUFFER_SIZE];

    k_memset(cmd, 0, BUFFER_SIZE);
    cmd[0] = 'R';
    cmd[1] = CMD_TERMINATOR;

    send_train_command(cmd, 0, 0);
}


/*
 * Probes a track segment sensor indicating the presence of some vehicle
 * command format: "C{id}\015"
 *      where {id} is the contact id
 * response format: "*X\015"
 *      where X is 1 if a vehicle is present, otherwise X is 0
 */
int status_of_contact(char *id)
{
    static const int len = 3;
    int status;
    char cmd[BUFFER_SIZE];
    char response[len];

    k_memset(cmd, 0, BUFFER_SIZE);
    cmd[0] = 'C';
    cmd[1] = '\0';
    strcat(cmd, id);
    cmd[k_strlen(cmd)] = CMD_TERMINATOR;

    /* must clear buffer before every inquiry */
    clear_s88_buffer();
    send_train_command(cmd, response, len);

    status = (response[0] == '*' && response[1] == '1') ? OCCUPIED : UNOCCUPIED;

    return status;
}


/*
 * Configures track switches to bypass dead-end track segment to create a route
 * loop
 */
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

    /* Ensure rogue train if present doesn't run of track */
    set_outer_loop_switches();

    while(1) {
    }
}


void init_train()
{
    create_process(train_process, 5, 0, "Train process");
}
