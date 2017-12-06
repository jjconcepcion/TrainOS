#include <kernel.h>

#define CMD_BUF_SIZE 128
#define CMD_HIST_SIZE 32
#define SHELL_CURSOR '$'
#define ASCII_CR 13
#define ASCII_BS 8
#define CMD_HIST_INDEX(cmd_no) cmd_no % CMD_HIST_SIZE - 1


typedef struct _CMD_HIST_ENTRY {
    char *cmd;
    int no;
} CMD_HIST_ENTRY;


void shell_process(PROCESS self, PARAM param);
char* execute_cmd(char *cmd_buffer, int window_id, CMD_HIST_ENTRY *cmd_history);


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
        "train    --model train controller\n"
        "!<num>   --repeat command from history with given num\n"
        "about    --print about message";

    wm_print(window_id, "%s", help1);
    wm_print(window_id, "%s", help2);
    wm_print(window_id, "%s", help3);
}


void print_about(int window_id)
{
    static const *about = "\nTOS Shell \n\n(c) 2017, Jan Joseph Concepcion";
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
 * Returns the decimal value ascii encoded unsiged number
 */
int atoi(char *str) {
        int val;

        val = 0;
        while (*str != '\0') {
                val = val * 10 + (int) (*str - '0');
                str++;
        }

        return val;
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


void print_history(int window_id, CMD_HIST_ENTRY *cmd_history)
{
    int i = 0;
    while (cmd_history[i].cmd != NULL && i < CMD_HIST_SIZE){
        wm_print(window_id, "\n   %d  %s", cmd_history[i].no
                                        , cmd_history[i].cmd);
        i++;
    }
}


char* repeat_cmd(int window_id, int cmd_no, CMD_HIST_ENTRY *cmd_history)
{
    char *old_cmd;

    old_cmd = NULL;
    old_cmd = cmd_history[CMD_HIST_INDEX(cmd_no)].cmd;
    if (old_cmd != NULL)
        old_cmd = execute_cmd(old_cmd, window_id, cmd_history);
    else
        wm_print(window_id, "\n!%d: event not found", cmd_no);

    return old_cmd;
}


void print_ps_output(int window_id)
{
	static const char *state[] = {
		"READY          ",
		"SEND_BLOCKED   ",
		"REPLY_BLOCKED  ",
		"RECEIVE_BLOCKED",
		"MESSAGE_BLOCKED",
		"INTR_BLOCKED   "
	};
	int i;
    PROCESS p;

	wm_print(window_id, "\nState           Active Prio Name\n");
	wm_print(window_id, "---------------------------------------\n");
	for (i = 0; i < MAX_PROCS; i++) {
        /* print process details */
		if (pcb[i].used == TRUE) {
            p = &pcb[i];
        	wm_print(window_id, "%s", state[p->state]);
        	if (p == active_proc)
        		wm_print(window_id, " *       ");
        	else
        		wm_print(window_id, "         ");
            wm_print(window_id, "%c %s\n", p->priority + 48, p->name);
        }
	}
}


char* execute_cmd(char *cmd_buffer, int window_id, CMD_HIST_ENTRY *cmd_history)
{
    char *cmd;

    cmd = parse_cmd(cmd_buffer);

    if (str_match(cmd, "help")) {
        print_help(window_id);
    } else if (str_match(cmd, "cls")) {
        clear_shell(window_id);
    } else if (str_match(cmd, "shell")) {
        start_shell();
    } else if (str_match(cmd, "pong")) {
        start_pong();
    } else if (str_match(cmd, "ps")) {
        print_ps_output(window_id);
    } else if (str_match(cmd, "history")) {
        print_history(window_id, cmd_history);
    } else if (str_match(cmd, "train")) {
        init_train();
    } else if (*cmd == '!') {
        int cmd_no = atoi((cmd+1));
        cmd = repeat_cmd(window_id, cmd_no, cmd_history);
    } else if (str_match(cmd, "about")) {
        print_about(window_id);
    } else {
        if (*cmd != '\0')
            wm_print(window_id, "\n%s: command not found", cmd);
    }

    return cmd;
}

void update_cmd_history(char *cmd, CMD_HIST_ENTRY *cmd_history, int cmd_no)
{
    int i;

    i = CMD_HIST_INDEX(cmd_no);
    cmd_history[i].cmd = cmd;
    cmd_history[i].no = cmd_no;
}

void shell_process(PROCESS self, PARAM param)
{
    CMD_HIST_ENTRY cmd_history[CMD_HIST_SIZE];
    int window_id, bytes_read, i, cmd_no;
    char *cmd_buffer, *ch, *cmd;

    window_id = wm_create(10, 3, 50, 17);
    print_shell_heading(window_id);
    print_shell_prompt(window_id);

    /* Initialize command history */
    for (i = 0; i < CMD_HIST_SIZE; i++)
        cmd_history[i].cmd = NULL;

    /* allocate input buffer*/
    cmd_buffer = malloc(CMD_BUF_SIZE);
    ch = cmd_buffer;
    cmd_no = 1;
    bytes_read = 0;
    /* Parse each character typed */
    while (1) {
        if (bytes_read < CMD_BUF_SIZE - 1) {
            *ch = keyb_get_keystroke(window_id, TRUE);
            switch (*ch) {
            case ASCII_CR:
            /* Parsed complete line */
                *(ch) = '\0'; /* discard carriage return */
                /* Skip emty lines */
                if (bytes_read > 0) {
                    cmd = execute_cmd(cmd_buffer, window_id, cmd_history);
                    update_cmd_history(cmd, cmd_history, cmd_no);
                    cmd_no++;
                    cmd_buffer = malloc(CMD_BUF_SIZE);
                    ch = cmd_buffer;
                    bytes_read = 0;
                }
                wm_print(window_id, "\n");
                print_shell_prompt(window_id);
                continue;
            case ASCII_BS:
            /* discard leading backspace */
                if (bytes_read > 0) {
                    wm_print(window_id, "%c", *ch);
                    bytes_read--;
                    ch--;
                }
                continue;
            }
            /* output character read */
            wm_print(window_id, "%c", *ch);
            ch++;
            bytes_read++;
        }
    }
    free(cmd_buffer);
    /* deallocate command history */
    for (i = 0; i < CMD_HIST_SIZE; i++)
        if (cmd_history[i].cmd)
            free(cmd_history[i].cmd);
}


void start_shell()
{
    create_process(shell_process, 5, 0, "Shell process");
    resign();
}
