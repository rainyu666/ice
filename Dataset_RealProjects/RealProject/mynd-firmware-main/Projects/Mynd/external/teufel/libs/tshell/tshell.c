#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "tshell.h"
#include "tshell_printf.h"

#define KEY_ESC (0x1BU)
#define KET_DEL (0x7FU)

#define SHELL_EVENT_DATA_ARRIVED (1U << 0)
#define SHELL_EVENT_DATA_SENT    (1U << 1)

#define TSHELL_MAX(_a_, _b_) ((_a_) > (_b_) ? (_a_) : (_b_))

#define SHELL_SPRINTF_BUFFER_SIZE (64U)

#define SHELL_CMD_HELP_PRINTED (1)

#define TSHELL_ALIGN(var, alignbytes) __attribute__((aligned(alignbytes))) var

TSHELL_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle = &s_shellHandleBuffer[0];

static int (*s_t_putchar)(char ch);

/*! @brief A type for the handle special key. */
typedef enum _fun_key_status
{
    kSHELL_Normal   = 0U, /*!< Normal key */
    kSHELL_Special  = 1U, /*!< Special key */
    kSHELL_Function = 2U, /*!< Function key */
} fun_key_status_t;

/*! @brief Data structure for Shell environment. */
typedef struct _shell_context_handle
{
    char                *prompt;                                     /*!< Prompt string */
    char                 line[SHELL_BUFFER_SIZE];                    /*!< Consult buffer */
    char                 hist_buf[SHELL_HISTORY_COUNT][SHELL_BUFFER_SIZE]; /*!< History buffer*/
    char                 printBuffer[SHELL_SPRINTF_BUFFER_SIZE];           /*!< Buffer for print */
    uint32_t             printLength;                                      /*!< All length has been printed */
    uint16_t             hist_current;                                     /*!< Current history command in hist buff*/
    uint16_t             hist_count;                                       /*!< Total history command in hist buff*/
    enum _fun_key_status stat;                                             /*!< Special key status */
    uint8_t              l_pos;                                            /*!< Total line position */
    uint8_t              c_pos;                                            /*!< Current line position */
    uint8_t              printBusy;                                        /*!< Print is busy */

    struct shell_static_entry active_cmd; /*!< Currently executed command.*/
} shell_context_handle_t;

static int32_t __attribute__((unused)) SHELL_StringCompare(const char *str1, const char *str2, int32_t count); /*!< compare string command */

// static void SHELL_AutoComplete(shell_context_handle_t *shellContextHandle); /*!< auto complete command */

static void SHELL_WriteWithCopy(shell_handle_t shellHandle, const char *buffer, uint32_t length);

static inline bool tshell_help_request(const char *str)
{
    return (!strcmp(str, "-h") || !strcmp(str, "--help"));
}

static void shell_internal_help_print(shell_context_handle_t *ctx);

// static char s_paramBuffer[SHELL_BUFFER_SIZE];

extern const union shell_cmd_entry __shell_root_cmds_start[];
extern const union shell_cmd_entry __shell_root_cmds_end[];

extern const union shell_cmd_entry __shell_subcmds_start[];
extern const union shell_cmd_entry __shell_subcmds_end[];

static inline const union shell_cmd_entry *shell_root_cmd_get(uint32_t id)
{
    return &__shell_root_cmds_start[id];
}

/* Determine if entry is a section command by checking if address is within
 * subcommands memory section.
 */
static inline bool is_section_cmd(const union shell_cmd_entry *entry)
{
    return (entry >= __shell_subcmds_start) && (entry < __shell_subcmds_end);
}

static inline uint32_t shell_root_cmd_count(void)
{
    return ((uint8_t *) __shell_root_cmds_end - (uint8_t *) __shell_root_cmds_start) / sizeof(union shell_cmd_entry);
}

/* Function returning pointer to parent command matching requested syntax. */
const struct shell_static_entry *root_cmd_find(const char *syntax)
{
    const size_t                 cmd_count = shell_root_cmd_count();
    const union shell_cmd_entry *cmd;

    for (size_t cmd_idx = 0; cmd_idx < cmd_count; ++cmd_idx)
    {
        cmd = shell_root_cmd_get(cmd_idx);
        if (strcmp(syntax, cmd->entry->syntax) == 0)
        {
            return cmd->entry;
        }
    }

    return NULL;
}

const struct shell_static_entry *tshell_cmd_get(const struct shell_static_entry *parent, size_t idx,
                                                 struct shell_static_entry *dloc)
{
    (void) dloc;

    const struct shell_static_entry *res = NULL;

    if (parent == NULL)
    {
        return (idx < shell_root_cmd_count()) ? shell_root_cmd_get(idx)->entry : NULL;
    }

    if (parent->subcmd)
    {
        const struct shell_static_entry *entry_list;

        if (is_section_cmd(parent->subcmd))
        {
            /* First element is null */
            entry_list = (const struct shell_static_entry *) parent->subcmd;
            idx++;
        }
        else
        {
            entry_list = parent->subcmd->entry;
        }

        if (entry_list[idx].syntax != NULL)
        {
            res = &entry_list[idx];
        }
    }

    return res;
}

/* Function returns pointer to a command matching given pattern.
 *
 * @param cmd		Pointer to commands array that will be searched.
 * @param lvl		Root command indicator. If set to 0 shell will search
 *			for pattern in parent commands.
 * @param cmd_str	Command pattern to be found.
 * @param dloc	Shell static command descriptor.
 *
 * @return		Pointer to found command.
 */
const struct shell_static_entry *t_shell_find_cmd(const struct shell_static_entry *parent, const char *cmd_str,
                                                  struct shell_static_entry *dloc)
{
    const struct shell_static_entry *entry;
    struct shell_static_entry        parent_cpy;
    size_t                           idx = 0;

    /* Dynamic command operates on shared memory. If we are processing two
     * dynamic commands at the same time (current and subcommand) they
     * will operate on the same memory region what can cause undefined
     * behaviour.
     * Hence we need a separate memory for each of them.
     */
    if (parent)
    {
        memcpy(&parent_cpy, parent, sizeof(struct shell_static_entry));
        parent = &parent_cpy;
    }

    while ((entry = tshell_cmd_get(parent, idx++, dloc)) != NULL)
    {
        if (strcmp(cmd_str, entry->syntax) == 0)
        {
            return entry;
        }
    }

    return NULL;
}

static char make_argv(char **ppcmd, uint8_t c)
{
    char *cmd   = *ppcmd;
    char  quote = 0;

    while (1)
    {
        c = *cmd;

        if (c == '\0')
        {
            break;
        }

        if (!quote)
        {
            switch (c)
            {
                case '\\':
                    memmove(cmd, cmd + 1, t_shell_strlen(cmd));
                    cmd += 1;
                    continue;

                case '\'':
                case '\"':
                    memmove(cmd, cmd + 1, t_shell_strlen(cmd));
                    quote = c;
                    continue;
                default:
                    break;
            }
        }

        if (quote == c)
        {
            memmove(cmd, cmd + 1, t_shell_strlen(cmd));
            quote = 0;
            continue;
        }

        if (quote && c == '\\')
        {
            char t = *(cmd + 1);

            if (t == quote)
            {
                memmove(cmd, cmd + 1, t_shell_strlen(cmd));
                cmd += 1;
                continue;
            }

            if (t == '0')
            {
                uint8_t i;
                uint8_t v = 0U;

                for (i = 2U; i < (2 + 3); i++)
                {
                    t = *(cmd + i);

                    if (t >= '0' && t <= '7')
                    {
                        v = (v << 3) | (t - '0');
                    }
                    else
                    {
                        break;
                    }
                }

                if (i > 2)
                {
                    memmove(cmd, cmd + (i - 1), t_shell_strlen(cmd) - (i - 2));
                    *cmd++ = v;
                    continue;
                }
            }

            if (t == 'x')
            {
                uint8_t i;
                uint8_t v = 0U;

                for (i = 2U; i < (2 + 2); i++)
                {
                    t = *(cmd + i);

                    if (t >= '0' && t <= '9')
                    {
                        v = (v << 4) | (t - '0');
                    }
                    else if ((t >= 'a') && (t <= 'f'))
                    {
                        v = (v << 4) | (t - 'a' + 10);
                    }
                    else if ((t >= 'A') && (t <= 'F'))
                    {
                        v = (v << 4) | (t - 'A' + 10);
                    }
                    else
                    {
                        break;
                    }
                }

                if (i > 2)
                {
                    memmove(cmd, cmd + (i - 1), t_shell_strlen(cmd) - (i - 2));
                    *cmd++ = v;
                    continue;
                }
            }
        }

        if (!quote && isspace((int) c))
        {
            break;
        }

        cmd += 1;
    }
    *ppcmd = cmd;

    return quote;
}

char t_shell_make_argv(size_t *argc, const char **argv, char *cmd, uint8_t max_argc)
{
    char quote = 0;
    char c;

    *argc = 0;
    do
    {
        c = *cmd;
        if (c == '\0')
        {
            break;
        }

        if (isspace((int) c))
        {
            *cmd++ = '\0';
            continue;
        }

        argv[(*argc)++] = cmd;
        if (*argc == max_argc)
        {
            break;
        }
        quote = make_argv(&cmd, c);
    } while (true);

    return quote;
}

#define SHELL_OPT_ARG_RAW (0xFE)

static void active_cmd_prepare(const struct shell_static_entry *entry, struct shell_static_entry *active_cmd,
                               struct shell_static_entry *help_entry, size_t *lvl, size_t *handler_lvl,
                               size_t *args_left)
{
    if (entry->handler)
    {
        *handler_lvl = *lvl;
        *active_cmd  = *entry;
        if ((entry->subcmd == NULL) && entry->args.optional == SHELL_OPT_ARG_RAW)
        {
            *args_left = entry->args.mandatory - 1;
            *lvl       = *lvl + 1;
        }
    }
    if (entry->help)
    {
        *help_entry = *entry;
    }
}

static int exec_cmd(shell_context_handle_t *ctx, size_t argc, const char **argv,
                    const struct shell_static_entry *help_entry)
{
    int ret_val = 0;

    if (ctx->active_cmd.handler == NULL)
    {
        if ((help_entry != NULL))
        { // && IS_ENABLED(CONFIG_SHELL_HELP)
            if (help_entry->help == NULL)
            {
                return -2;
            }
            if (help_entry->help != ctx->active_cmd.help)
            {
                ctx->active_cmd = *help_entry;
            }
            // shell_internal_help_print(shell);
            printf("print help command (internal)\r\n");
            return SHELL_CMD_HELP_PRINTED;
        }
        else
        {
            // z_shell_fprintf(shell, SHELL_ERROR,
            //                 SHELL_MSG_SPECIFY_SUBCOMMAND);
            printf("ERROR: specify subcommand!\r\n");
            return -2;
        }
    }

    if (ctx->active_cmd.args.mandatory)
    {
        uint32_t mand = ctx->active_cmd.args.mandatory;
        uint8_t  opt8 = ctx->active_cmd.args.optional;
        (void) mand;
        (void) opt8;
        // uint32_t opt = (opt8 == SHELL_OPT_ARG_CHECK_SKIP) ?
        //                UINT16_MAX : opt8;
        // const bool in_range = IN_RANGE(argc, mand, mand + opt);
        //
        // /* Check if argc is within allowed range */
        // ret_val = cmd_precheck(shell, in_range);
    }

    if (!ret_val)
    {
#if CONFIG_SHELL_GETOPT
        getopt_init();
#endif

        // z_flag_cmd_ctx_set(shell, true);
        /* Unlock thread mutex in case command would like to borrow
         * shell context to other thread to avoid mutex deadlock.
         */
        // k_mutex_unlock(&shell->ctx->wr_mtx);
        ret_val = ctx->active_cmd.handler((void *) ctx, argc, (char **) argv);
        /* Bring back mutex to shell thread. */
        // k_mutex_lock(&shell->ctx->wr_mtx, K_FOREVER);
        // z_flag_cmd_ctx_set(shell, false);
    }

    return 0;

    return ret_val;
}

#define CONFIG_SHELL_ARGC_MAX 5U

static int shell_execute(shell_context_handle_t *ctx)
{
    struct shell_static_entry        dloc;                            /* Memory for dynamic commands. */
    const char                      *argv[CONFIG_SHELL_ARGC_MAX + 1]; /* +1 reserved for NULL */
    const struct shell_static_entry *parent = NULL; // selected_cmd_get(shell);   selected mode?! in command
    const struct shell_static_entry *entry  = NULL;
    struct shell_static_entry        help_entry;
    size_t                           cmd_lvl              = 0;
    size_t                           cmd_with_handler_lvl = 0;
    // bool                             wildcard_found       = false;
    size_t                           argc = 0, args_left = SIZE_MAX;
    char                             quote;
    const char                     **argvp;
    char                            *cmd_buf          = ctx->line; /// rename to ctx->cmd_buff;
    bool                             has_last_handler = false;

    help_entry.help = NULL;
    argvp           = &argv[0];

    /* Below loop is analyzing subcommands of found root command. */
    while ((argc != 1) && (cmd_lvl < CONFIG_SHELL_ARGC_MAX) && args_left > 0)
    {
        quote   = t_shell_make_argv(&argc, argvp, cmd_buf, 2);
        cmd_buf = (char *) argvp[1];

        if (argc == 0)
        {
            return -2;
        }
        else if ((argc == 1) && (quote != 0))
        {
            printf("ERROR: not terminated: %c\r\n", quote);
            return -2;
        }

        // If IS_ENABLED(CONFIG_SHELL_HELP)
        if ((cmd_lvl > 0) && tshell_help_request(argvp[0])) {
            /* Command called with help option, so it makes no sense
             * to search deeper commands.
             */
            if (help_entry.help) {
                ctx->active_cmd = help_entry;
                shell_internal_help_print(ctx);
                return SHELL_CMD_HELP_PRINTED;
            }
        }

        if (has_last_handler == false)
        {
            entry = t_shell_find_cmd(parent, argvp[0], &dloc);
        }

        argvp++;
        args_left--;
        if (entry)
        {
            active_cmd_prepare(entry, &ctx->active_cmd, &help_entry, &cmd_lvl, &cmd_with_handler_lvl, &args_left);
            parent = entry;
        }
        else
        {
            if (cmd_lvl == 0)
            {
                printf("ERROR: command not found: %s\r\n", argv[0]);
            }

            /* last handler found - no need to search commands in
             * the next iteration. */
            has_last_handler = true;
        }

        if (args_left || (argc == 2))
        {
            cmd_lvl++;
        }
    }

    if ((cmd_lvl >= CONFIG_SHELL_ARGC_MAX) && (argc == 2))
    {
        /* argc == 2 indicates that when command string was parsed
         * there was more characters remaining. It means that number of
         * arguments exceeds the limit.
         */
        printf("ERROR: too many args (please, check CONFIG_SHELL_ARGC_MAX definition)\r\n");
        return -2;
    }

    /* terminate arguments with NULL */
    argv[cmd_lvl] = NULL;
    /* Executing the deepest found handler. */
    return exec_cmd(ctx, cmd_lvl - cmd_with_handler_lvl, &argv[cmd_with_handler_lvl], &help_entry);

    return 0;
}

void tshell_process_buffer(const uint8_t *buffer, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        tshell_process_char(buffer[i]);
    }
}

void tshell_process_char(uint8_t ch)
{
    shell_context_handle_t *shellContextHandle = (shell_context_handle_t *) s_shellHandle;

    if (!shellContextHandle)
        return;

    /* Special key */
    if (ch == KEY_ESC)
    {
        shellContextHandle->stat = kSHELL_Special;
        return;
    }
    else if (shellContextHandle->stat == kSHELL_Special)
    {
        /* Function key */
        if ((char) ch == '[')
        {
            shellContextHandle->stat = kSHELL_Function;
            return;
        }
        shellContextHandle->stat = kSHELL_Normal;
    }
    else if (shellContextHandle->stat == kSHELL_Function)
    {
        shellContextHandle->stat = kSHELL_Normal;

        switch ((char) ch)
        {
            /* History operation here */
            case 'A': /* Up key */
                /*SHELL_GetHistoryCommand(shellContextHandle, (uint8_t) shellContextHandle->hist_current);*/
                if (shellContextHandle->hist_current < (shellContextHandle->hist_count - 1U))
                {
                    shellContextHandle->hist_current++;
                }
                break;
            case 'B': /* Down key */
                /*SHELL_GetHistoryCommand(shellContextHandle, (uint8_t) shellContextHandle->hist_current);*/
                if (shellContextHandle->hist_current > 0U)
                {
                    shellContextHandle->hist_current--;
                }
                break;
            case 'D': /* Left key */
                if ((bool) shellContextHandle->c_pos)
                {
                    SHELL_WriteWithCopy(shellContextHandle, "\b", 1);
                    shellContextHandle->c_pos--;
                }
                break;
            case 'C': /* Right key */
                if (shellContextHandle->c_pos < shellContextHandle->l_pos)
                {
                    tshell_write(shellContextHandle, &shellContextHandle->line[shellContextHandle->c_pos], 1);
                    shellContextHandle->c_pos++;
                }
                break;
            default:
                /* MISRA C-2012 Rule 16.4 */
                break;
        }
        return;
    }
    /* Handle tab key */
    else if ((char) ch == '\t')
    {
#if SHELL_AUTO_COMPLETE
        /* Move the cursor to the beginning of line */
        uint32_t i;
        for (i = 0; i < (uint32_t) shellContextHandle->c_pos; i++)
        {
            SHELL_WriteWithCopy(shellContextHandle, "\b", 1);
        }
        /* Do auto complete */
        /*SHELL_AutoComplete(shellContextHandle);*/
        /* Move position to end */
        shellContextHandle->l_pos = (uint8_t) strlen(shellContextHandle->line);
        shellContextHandle->c_pos = shellContextHandle->l_pos;
#endif
        return;
    }
    /* Handle backspace key */
    else if ((ch == KET_DEL) || ((char) ch == '\b'))
    {
        /* There must be at last one char */
        if (shellContextHandle->c_pos == 0U)
        {
            return;
        }

        shellContextHandle->l_pos--;
        shellContextHandle->c_pos--;

        if (shellContextHandle->l_pos > shellContextHandle->c_pos)
        {
            (void) memmove(&shellContextHandle->line[shellContextHandle->c_pos],
                           &shellContextHandle->line[shellContextHandle->c_pos + 1U],
                           (uint32_t) shellContextHandle->l_pos - (uint32_t) shellContextHandle->c_pos);
            shellContextHandle->line[shellContextHandle->l_pos] = '\0';
            SHELL_WriteWithCopy(shellContextHandle, "\b", 1);
            tshell_write(shellContextHandle, &shellContextHandle->line[shellContextHandle->c_pos],
                        strlen(&shellContextHandle->line[shellContextHandle->c_pos]));
            SHELL_WriteWithCopy(shellContextHandle, "  \b", 3);

            /* Reset position */
            uint32_t i;
            for (i = (uint32_t) shellContextHandle->c_pos; i <= (uint32_t) shellContextHandle->l_pos; i++)
            {
                SHELL_WriteWithCopy(shellContextHandle, "\b", 1);
            }
        }
        else /* Normal backspace operation */
        {
            SHELL_WriteWithCopy(shellContextHandle, "\b \b", 3);
            shellContextHandle->line[shellContextHandle->l_pos] = '\0';
        }
        return;
    }
    else
    {
        /* MISRA C-2012 Rule 15.7 */
    }

    /* Input too long */
    if (shellContextHandle->l_pos >= (SHELL_BUFFER_SIZE - 1U))
    {
        shellContextHandle->l_pos = 0U;
    }

    /* Handle end of line, break */
    if (((char) ch == '\r') || ((char) ch == '\n'))
    {
        static char endoflinechar = '\0';

        if (((uint8_t) endoflinechar != 0U) && ((uint8_t) endoflinechar != ch))
        {
            return;
        }
        else
        {
            endoflinechar = (char) ch;
            /* Print new line. */
            SHELL_WriteWithCopy(shellContextHandle, "\r\n", 2U); /* MISRA C-2012 Rule 7.4 */
            /* If command line is not NULL, will start process it. */
            if (0U != strlen(shellContextHandle->line))
            {
                // SHELL_ProcessCommand(shellContextHandle, shellContextHandle->line);
                shell_execute(shellContextHandle);
            }
            /* Print prompt. */
            tshell_write(shellContextHandle, shellContextHandle->prompt, strlen(shellContextHandle->prompt));
            /* Reset all params */
            shellContextHandle->c_pos = shellContextHandle->l_pos = 0;
            shellContextHandle->hist_current                      = 0;
            (void) memset(shellContextHandle->line, 0, sizeof(shellContextHandle->line));
            return;
        }
    }

    /* Normal character */
    if (shellContextHandle->c_pos < shellContextHandle->l_pos)
    {
        (void) memmove(&shellContextHandle->line[shellContextHandle->c_pos + 1U],
                       &shellContextHandle->line[shellContextHandle->c_pos],
                       (uint32_t) shellContextHandle->l_pos - (uint32_t) shellContextHandle->c_pos);
        shellContextHandle->line[shellContextHandle->c_pos] = (char) ch;
        tshell_write(shellContextHandle, &shellContextHandle->line[shellContextHandle->c_pos],
                    strlen(&shellContextHandle->line[shellContextHandle->c_pos]));
        /* Move the cursor to new position */
        uint32_t i;
        for (i = (uint32_t) shellContextHandle->c_pos; i < (uint32_t) shellContextHandle->l_pos; i++)
        {
            SHELL_WriteWithCopy(shellContextHandle, "\b", 1);
        }
    }
    else
    {
        shellContextHandle->line[shellContextHandle->l_pos] = (char) ch;
        tshell_write(shellContextHandle, &shellContextHandle->line[shellContextHandle->l_pos], 1);
    }

    ch = 0;
    shellContextHandle->l_pos++;
    shellContextHandle->c_pos++;
}

static int32_t SHELL_StringCompare(const char *str1, const char *str2, int32_t count)
{
    while ((bool) (count--))
    {
        if (*str1++ != *str2++)
        {
            return (int32_t) (*(str1 - 1) - *(str2 - 1));
        }
    }
    return 0;
}


int tshell_init(const struct tshell_config *conf, char *prompt)
{
    shell_context_handle_t *shellContextHandle;

    s_t_putchar = conf->t_putchar;

    assert(s_shellHandle);
#if !(!defined(SDK_DEBUGCONSOLE_UART) && (defined(SDK_DEBUGCONSOLE) && (SDK_DEBUGCONSOLE != 1)))
    // assert(serialHandle);
#endif
    assert(prompt);
    assert(SHELL_HANDLE_SIZE >= sizeof(shell_context_handle_t));

    shellContextHandle = (shell_context_handle_t *) s_shellHandle;

    /* memory set for shellHandle */
    (void) memset(s_shellHandle, 0, SHELL_HANDLE_SIZE);

    shellContextHandle->prompt       = prompt;

    set_putchar(conf->t_putchar);

    // tshell_print_prompt(shellContextHandle);

    return 0;
}

int tshell_write(shell_handle_t shellHandle, char *buffer, uint32_t length)
{
    shell_context_handle_t *shellContextHandle;
    // uint32_t                primask;
    int          status;

    assert(shellHandle);
    assert(buffer);

    if (!(bool) length)
    {
        return 0;
    }

    shellContextHandle = (shell_context_handle_t *) shellHandle;

    // primask = DisableGlobalIRQ();
    if ((bool) shellContextHandle->printBusy)
    {
        // EnableGlobalIRQ(primask);
        return -1;
    }
    shellContextHandle->printBusy = 1U;
    // EnableGlobalIRQ(primask);

    status = 0;
    for (uint32_t index = 0; index < length; index++)
    {
        (void) s_t_putchar(buffer[index]);
    }

    shellContextHandle->printBusy = 0U;

    return status;
}

/* For MISRA to fix const */
static void SHELL_WriteWithCopy(shell_handle_t shellHandle, const char *buffer, uint32_t length)
{
    char s_shellWriteCopyBuffer[128];

    assert(length <= 128UL);

    memcpy(s_shellWriteCopyBuffer, buffer, length);
    tshell_write(shellHandle, s_shellWriteCopyBuffer, length);
}

void tshell_change_prompt(shell_handle_t shellHandle, char *prompt)
{
    shell_context_handle_t *shellContextHandle;
    assert(shellHandle);
    assert(prompt);

    shellContextHandle = (shell_context_handle_t *) shellHandle;

    shellContextHandle->prompt = prompt;
    tshell_print_prompt(shellContextHandle);
}

void tshell_print_prompt(shell_handle_t shellHandle)
{
    shell_context_handle_t *shellContextHandle;
    assert(shellHandle);

    shellContextHandle = (shell_context_handle_t *) shellHandle;

    SHELL_WriteWithCopy(shellContextHandle, "\r\n", 2U); /* MISRA C-2012 Rule 7.4 */
    tshell_write(shellContextHandle, shellContextHandle->prompt, strlen(shellContextHandle->prompt));
}

void tshell_fprintf(const struct shell *sh, const char *fmt, ...)
{
    (void) sh;

    static char buf[64];
    static char final_buf[128];
    int         len;

    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, 64, fmt, args);
    va_end(args);

    len = snprintf(final_buf, 128, "%s", buf);
    (void) len;

    // log_info_raw("%s", final_buf);
}

static void help_item_print(const struct shell *shell, const char *item_name, uint16_t item_name_width,
                            const char *item_help)
{
    (void) shell;
    (void) item_name_width;

    if ((item_name == NULL) || (item_name[0] == '\0'))
    {
        return;
    }

    if (item_help)
        tshell_printf("  %s - %s\r\n", item_name, item_help);
    else
        tshell_printf("  %s\r\n", item_name);

#if 0 // TODO: add indentation
    uint16_t tmp = item_name_width - strlen(item_name);
    char space = ' ';

    t_shell_fprintf(shell, "%s%s", tabulator, item_name);

    if (item_help) {
        for (uint16_t i = 0; i < tmp; i++) {
            (void)__io_putchar(space);
        }
    }
#endif
}

void z_shell_help_cmd_print(const struct shell *shell,
                            const struct shell_static_entry *cmd)
{
    (void) shell;

    // static const char cmd_sep[] = " - "; /* commands separator */
    // uint16_t field_width;
    // field_width = strlen(cmd->syntax) + strlen(cmd_sep);

    if (!cmd || !cmd->syntax)
        return;

    if (cmd->help)
        tshell_printf("%s - %s\r\n", cmd->syntax, cmd->help);
    else
        tshell_printf("%s\r\n", cmd->syntax);
}

/* Function prints all subcommands of the parent command together with their
 * help string
 */
static void t_shell_help_subcmd_print(const struct shell *shell, const struct shell_static_entry *parent,
                               const char *description)
{
    const struct shell_static_entry *entry = NULL;
    struct shell_static_entry        dloc;
    uint16_t                         longest = 0U;
    size_t                           idx     = 0;

    /* Searching for the longest subcommand to print. */
    while ((entry = tshell_cmd_get(parent, idx++, &dloc)) != NULL)
    {
        longest = TSHELL_MAX(longest, t_shell_strlen(entry->syntax));
    }

    /* No help to print */
    if (longest == 0)
    {
        return;
    }

    if (description != NULL)
    {
        tshell_printf(description);
    }

    /* Printing subcommands and help string (if exists). */
    idx = 0;

    while ((entry = tshell_cmd_get(parent, idx++, &dloc)) != NULL)
    {
        help_item_print(shell, entry->syntax, longest, entry->help);
    }
}

static void shell_internal_help_print(shell_context_handle_t *ctx)
{
    // if (!IS_ENABLED(CONFIG_SHELL_HELP)) {
    //     return;
    // }

    z_shell_help_cmd_print(NULL, &ctx->active_cmd);
    t_shell_help_subcmd_print(NULL, &ctx->active_cmd, "\r\nSubcommands:\r\n");
}

static int cmd_help(const struct shell *shell, size_t argc, char **argv)
{
    (void) (argc);
    (void) (argv);

#if defined(CONFIG_SHELL_TAB)
    shell_print(shell, "Please press the <Tab> button to see all available "
                       "commands.");
#endif

#if defined(CONFIG_SHELL_TAB_AUTOCOMPLETION)
    shell_print(shell, "You can also use the <Tab> button to prompt or auto-complete"
                       " all commands or its subcommands.");
#endif

#if defined(CONFIG_SHELL_HELP)
    shell_print(shell, "You can try to call commands with <-h> or <--help> parameter"
                       " for more information.");
#endif

#if defined(CONFIG_SHELL_METAKEYS)
    shell_print(shell, "\nShell supports following meta-keys:\n"
                       "  Ctrl + (a key from: abcdefklnpuw)\n"
                       "  Alt  + (a key from: bf)\n"
                       "Please refer to shell documentation for more details.");
#endif

    t_shell_help_subcmd_print(shell, NULL, "\nAvailable commands:\r\n");

    return 0;
}

SHELL_CMD_ARG_REGISTER(help, NULL, "Prints the help message.", cmd_help, 1, 0);
