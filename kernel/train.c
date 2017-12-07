
#include <kernel.h>

#define TRAIN_ID "20\0"
#define CMD_DELAY_TIME 15
#define BUFFER_SIZE 8
#define CMD_TERMINATOR '\015'
#define OCCUPIED 1
#define UNOCCUPIED 0
#define ROGUE_SEARCH_DURATION 12
#define ROGUE_SEARCH_LOCATION "7\0"
#define NUM_TRAIN_START_POSITIONS 4
#define TRAIN_LOG_FLAG 1

typedef enum _train_configuration {
    UNKNOWN, CONFIG1_NO_ROGUE, CONFIG2_NO_ROGUE, CONFIG3_NO_ROGUE,
    CONFIG4_NO_ROGUE, CONFIG1_ROGUE, CONFIG2_ROGUE, CONFIG3_ROGUE,
    CONFIG4_ROGUE
} TRAIN_CONFIGURATION;


void config1_route_plan();
void config2_route_plan();
void config3_route_plan();
void config4_route_plan();
void config1_rogue_route_plan();
void config2_rogue_route_plan();
void config3_rogue_route_plan();
void config4_rogue_route_plan();


static int *TRAIN_LOG_WINDOW_ID = NULL;

/*
 * Helper function for printing out train app messages
 */
void train_log(char *msg)
{
    if (TRAIN_LOG_FLAG)
        wm_print(*TRAIN_LOG_WINDOW_ID, "%s", msg);
}


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

    train_log("\nSet switch: ");
    train_log(id);
    train_log(": ");
    train_log(setting);
    train_log("\n");
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

    train_log("\nSet train speed: ");
    train_log(speed);
    train_log("\n");
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

    train_log("\nChange train direction.\n");
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

    train_log("\nClear s88 buffer.\n");
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

    train_log("\nProbe contact: ");
    train_log(id);

    send_train_command(cmd, response, len);

    status = (response[0] == '*' && response[1] == '1') ? OCCUPIED : UNOCCUPIED;

    if (status) train_log(": OCCUPIED\n");
        else  train_log(": UNOCCUPIED\n");

    return status;
}


/*
 * Configures track switches to bypass dead-end track segment to create a route
 * loop
 */
void set_outer_loop_switches()
{
    train_log("\n>>Setting switches for outer loop...\n");
    set_switch("5\0", "G\0");
    set_switch("8\0", "G\0");
    set_switch("9\0", "R\0");
    set_switch("4\0", "G\0");
    set_switch("1\0", "G\0");
}


/*
 * Monitor a track segment [ROUGE_SEARCH_LOCATION] for a duration of time
 * [ROGUE_SEARCH_DURATION] with the intention of detecting the presence of
 * a rogue train car.
 *
 * returns 1 if rogue train is present, otherwise 0.
 */
int detect_rogue_train()
{
    char *monitoring = ROGUE_SEARCH_LOCATION;
    int duration, detected;

    detected = 0;
    duration = ROGUE_SEARCH_DURATION;
    while (duration > 0) {
        train_log("\n>>Monitor track segment for rogue train");
        if (detected = (status_of_contact(monitoring) == OCCUPIED))
            break;
        duration--;
    }

    train_log("\n>>Rogue train: ");
    if (!detected) train_log(" NOT ");
    train_log("DETECTED\n");

    return detected;
}


/*
 * Returns the configuration of the model train via the scenario argument.
 */
void identify(TRAIN_CONFIGURATION *scenario)
{
    train_log("\n>>Indetifying train configuration...\n");

    int config[NUM_TRAIN_START_POSITIONS+1];
    int rogue_exists;

    rogue_exists = detect_rogue_train();
    config[1] = status_of_contact("8\0");
    config[2] = status_of_contact("12\0");
    config[3] = status_of_contact("5\0");
    config[4] = status_of_contact("16\0");

    train_log("\n>>Scenario: ");
    if (!rogue_exists) {
        if (config[1]) {
            *scenario = CONFIG1_NO_ROGUE;
            train_log("Configuration 1\n");
        } else if (config[2]) {
            *scenario = CONFIG2_NO_ROGUE;
            train_log("Configuration 2\n");
        } else if (config[3]) {
            *scenario = CONFIG3_NO_ROGUE;
            train_log("Configuration 3\n");
        } else if (config[4]) {
            *scenario = CONFIG4_NO_ROGUE;
            train_log("Configuration 4\n");
        } else {
            *scenario = UNKNOWN;
        }
    } else {
        if (config[1]) {
            *scenario = CONFIG1_ROGUE;
            train_log("Configuration 1 with rogue train\n");
        } else if (config[2]) {
            *scenario = CONFIG2_ROGUE;
            train_log("Configuration 2 with rogue train\n");
        } else if (config[3]) {
            *scenario = CONFIG3_ROGUE;
            train_log("Configuration 3 with rogue train\n");
        } else if (config[4]) {
            *scenario = CONFIG4_ROGUE;
            train_log("Configuration 4 with rogue train\n");
        } else {
            *scenario = UNKNOWN;
            train_log("Unidentified\n");
        }
    }
}


void execute_route_for(TRAIN_CONFIGURATION scenario)
{
    train_log("\n>>Executing train route...\n");
    switch (scenario) {
        case CONFIG1_NO_ROGUE:
            config1_route_plan();
            break;
        case CONFIG2_NO_ROGUE:
            config2_route_plan();
            break;
        case CONFIG3_NO_ROGUE:
            config3_route_plan();
            break;
        case CONFIG4_NO_ROGUE:
            config4_route_plan();
            break;
        case CONFIG1_ROGUE:
            config1_rogue_route_plan();
            break;
        case CONFIG2_ROGUE:
            config2_rogue_route_plan();
            break;
        case CONFIG3_ROGUE:
            config3_rogue_route_plan();
            break;
        case CONFIG4_ROGUE:
            config4_rogue_route_plan();
            break;
        default:
            break;
    }


}

void print_app_heading(int window_id)
{
    wm_print(window_id, "**********************\n");
    wm_print(window_id, "TOS Train Application\n");
    wm_print(window_id, "**********************\n");
    if (!TRAIN_LOG_FLAG)
        wm_print(window_id, "\nLog messages disabled\n");
}


void train_process(PROCESS self, PARAM param)
{
    int window_id;
    TRAIN_CONFIGURATION train_scenario;

    window_id = wm_create(5, 5, 40, 15);
    if (TRAIN_LOG_FLAG) TRAIN_LOG_WINDOW_ID = &window_id;

    print_app_heading(window_id);

    /* Ensure rogue train if present doesn't run of track */
    set_outer_loop_switches();
    identify(&train_scenario);
    execute_route_for(train_scenario);

    while(1);
}


void init_train()
{
    create_process(train_process, 5, 0, "Train process");
}


/*
 *
 * Train route strategy implementations
 *
 */


/*
 * Scenario: train located on contact 8, wagon on contact 11, no rogue train
 */
void config1_route_plan()
{
    set_switch("4\0", "R\0");
    set_switch("3\0", "G\0");
    set_switch("8\0", "R\0");
    set_switch("5\0", "R\0");
    set_switch("6\0", "R\0");
    set_train_speed("4\0");
    while (status_of_contact("12\0") == UNOCCUPIED);
    set_switch("8\0", "G\0");
    while (status_of_contact("7\0") == UNOCCUPIED);
    set_train_speed("0\0");
    change_direction(TRAIN_ID);
    set_train_speed("4\0");
    while (status_of_contact("8\0") == UNOCCUPIED);
    set_train_speed("0\0");
}


/*
 * Scenario: train located on contact 12, wagon on contact 2, no rogue train
 */
void config2_route_plan()
{
    set_switch("1\0", "R\0");
    set_switch("2\0", "G\0");
    set_switch("5\0", "R\0");
    set_switch("6\0", "G\0");
    set_switch("9\0", "R\0");
    set_train_speed("4\0");
    while (status_of_contact("14\0") == UNOCCUPIED);
    set_train_speed("0\0");
    change_direction(TRAIN_ID);
    set_train_speed("4\0");
    while (status_of_contact("12\0") == UNOCCUPIED);
    set_train_speed("0\0");
}


/*
 * Scenario: train located on contact 5, wagon on contact 11, no rogue train
 */
void config3_route_plan()
{
    set_switch("3\0", "R\0");
    set_switch("4\0", "R\0");
    set_switch("5\0", "R\0");
    set_switch("6\0", "G\0");
    set_switch("8\0", "R\0");
    set_train_speed("5\0");
    while (status_of_contact("11\0") == OCCUPIED );
    set_switch("8\0", "G\0");
    while (status_of_contact("7\0") == UNOCCUPIED);
    set_train_speed("4\0");
    while (status_of_contact("5\0") == UNOCCUPIED);
    set_train_speed("0\0");
}


/*
 * Scenario: train located on contact 16, wagon on contact 9, no rogue train
 */
void config4_route_plan()
{
    set_switch("8\0", "G\0");
    set_switch("9\0", "G\0");
    set_switch("5\0", "R\0");
    set_switch("6\0", "G\0");
    set_train_speed("4\0");
    while (status_of_contact("7\0") == UNOCCUPIED );
    set_train_speed("0\0");
    change_direction(TRAIN_ID);
    set_train_speed("4\0");
    while (status_of_contact("14\0") == UNOCCUPIED);
    set_train_speed("0\0");
    change_direction(TRAIN_ID);
    set_train_speed("4\0");
    while (status_of_contact("16\0") == UNOCCUPIED);
    set_train_speed("0\0");
}


/*
 * Scenario: train located on contact 8, wagon on contact 11 with a rogue train
 * present.
 */
void config1_rogue_route_plan()
{
    set_switch("6\0", "G\0");
    set_switch("7\0", "R\0");
    set_switch("2\0", "R\0");
    while (status_of_contact("7\0") == UNOCCUPIED);
    while (status_of_contact("7\0") == OCCUPIED);
    set_switch("5\0", "R\0");
    set_train_speed("5\0");
    while (status_of_contact("7\0") == UNOCCUPIED);
    set_train_speed("0\0");
    change_direction();
    set_train_speed("5\0");
    while (status_of_contact("7\0") == OCCUPIED);
    set_switch("5\0", "G\0");
    while (status_of_contact("9\0") == OCCUPIED);
    set_train_speed("0\0");
    change_direction();
    while (status_of_contact("13\0") == UNOCCUPIED);
    set_train_speed("5\0");
    while (status_of_contact("7\0") == UNOCCUPIED);
    set_switch("1\0", "R\0");
    while (status_of_contact("7\0") == OCCUPIED);
    set_switch("5\0", "R\0");
    set_switch("6\0", "R\0");
    set_train_speed("4\0");
    while (status_of_contact("8\0") == UNOCCUPIED);
    set_train_speed("0\0");
}


void config2_rogue_route_plan()
{
}


void config3_rogue_route_plan()
{
}


void config4_rogue_route_plan()
{
}
