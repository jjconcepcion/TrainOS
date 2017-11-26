#include <kernel.h>

#define CMD_BUF_SIZE 128
#define SHELL_CURSOR '$'
#define ASCII_CR 13


void print_shell_heading(int window_id) {
    wm_print(window_id, "TOS shell\n----------\n");
}


void print_shell_prompt(int window_id) {
    wm_print(window_id, "%c ", SHELL_CURSOR);
}


char* parse_cmd(char *buf)
{
    char *cmd;

    /* skip leading whitespace */
    while (*buf == ' ')
        ++buf;
    cmd = buf;
    /* isolate command string */
    while (*buf != ' ')
        ++buf;
    *buf = '\0';

    return cmd;
}


void shell_process(PROCESS self, PARAM param)
{
    int window_id;
    char *cmd_buffer, *ch, *cmd;

    window_id = wm_create(10, 3, 50, 17);
    print_shell_heading(window_id);
    print_shell_prompt(window_id);

    /* allocate input buffer*/
    cmd_buffer = malloc(CMD_BUF_SIZE);
    /* Read line of keyboard input */
    ch = cmd_buffer;
    while (1) {
        *ch = keyb_get_keystroke(window_id, TRUE);
        if (*ch == ASCII_CR) {
            *(ch) = '\0';

            cmd = parse_cmd(cmd_buffer);
            wm_print(window_id, "\ncmd: %s\n", cmd);

            print_shell_prompt(window_id);
            ch = cmd_buffer;
            continue;
        }
        wm_print(window_id, "%c", *ch);
        ch++;
    }
    free(cmd_buffer);
}


void start_shell()
{
    create_process(shell_process, 5, 0, "Shell process");
    resign();
}
