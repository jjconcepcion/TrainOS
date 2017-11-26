#include <kernel.h>

#define CMD_BUF_SIZE 128
#define SHELL_CURSOR '$'
#define ASCII_CR 13

void shell_process(PROCESS self, PARAM param);

void print_shell_heading(int window_id)
{
    wm_print(window_id, "TOS shell\n----------\n");
}


void print_shell_prompt(int window_id)
{
    wm_print(window_id, "%c ", SHELL_CURSOR);
}

void print_help(int window_id)
{
    static const char *help1 =
        "\nhelp    --lists supported TSO commands\n"
        "cls    --clears the window\n"
        "shell    --lauches another shell\n";
    static const char *help2 =
        "pong   --launches PONG game\n"
        "ps   --print process table\n"
        "history   --print command history\n";
    static const char *help3 =
        "!<num>   --repeat command from history with given num\n"
        "about    --print about message\n";

    wm_print(window_id, "%s", help1);
    wm_print(window_id, "%s", help2);
    wm_print(window_id, "%s", help3);
}


void print_about(int window_id)
{
    static const *about = "\nTOS Shell \n\n(c) 2017, Jan Joseph Concepcion\n";
    wm_print(window_id, "%s", about);
}

void clear_shell(int window_id)
{
    wm_clear(window_id);
}


/*
 * Returns 1 if the str1 and str2 contain the same string. Returns 0 otherwise.
 */
int str_match(const char *str1, const char *str2)
{
    int match = 1;

    while (*str1 && *str2) {
        if (*str1++ != *str2++) {
            match = 0;
            break;
        }
    }
    /* strings are unequal length */
    if (match && (*str1 || *str2))
        match = 0;

    return match;
}


/*
 * Inserts null terminator into buffer to delimit the command name from command
 * arguments. Returns pointer to string containing the command name.
 */
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

void execute_cmd(char *cmd_buffer, int window_id)
{
    char *cmd;

    cmd = parse_cmd(cmd_buffer);

    if (str_match(cmd, "help")) {
        print_help(window_id);
    } else if (str_match(cmd, "cls")) {
        clear_shell(window_id);
    } else if (str_match(cmd, "shell")) {
    } else if (str_match(cmd, "pong")) {
    } else if (str_match(cmd, "ps")) {
    } else if (str_match(cmd, "history")) {
    } else if (*cmd == '!') {
    } else if (str_match(cmd, "about")) {
        print_about(window_id);
    } else {
        wm_print(window_id, "\n%s: command not found\n", cmd);
    }
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
    ch = cmd_buffer;
    /* Parse each character typed */
    while (1) {
        *ch = keyb_get_keystroke(window_id, TRUE);
        /* Parsed complete line */
        if (*ch == ASCII_CR) {
            *(ch) = '\0'; /* discard carriage return */
            execute_cmd(cmd_buffer, window_id);
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
