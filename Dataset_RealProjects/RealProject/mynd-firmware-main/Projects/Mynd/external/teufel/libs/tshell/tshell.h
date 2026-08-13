#pragma once

#include <string.h>

#include "tshell_printf.h"
#include "tshell_args_parser.h"

/*! @brief Macro to set on/off auto-complete feature. */
#define SHELL_AUTO_COMPLETE (1U)

/*! @brief Macro to set console buffer size. */
#ifndef SHELL_BUFFER_SIZE
#define SHELL_BUFFER_SIZE (64U)
#endif

/*! @brief Macro to set maximum arguments in command. */
#define SHELL_MAX_ARGS (8U)

/*! @brief Macro to set maximum count of history commands. */
#ifndef SHELL_HISTORY_COUNT
#define SHELL_HISTORY_COUNT (3U)
#endif

/*! @brief Macro to bypass arguments check */
#define SHELL_IGNORE_PARAMETER_COUNT (0xFF)

/*! @brief The handle size of the shell module. It is the sum of the SHELL_HISTORY_COUNT * SHELL_BUFFER_SIZE +
 * SHELL_BUFFER_SIZE + SERIAL_MANAGER_READ_HANDLE_SIZE + SERIAL_MANAGER_WRITE_HANDLE_SIZE*/
#define SHELL_HANDLE_SIZE (160U + SHELL_HISTORY_COUNT * SHELL_BUFFER_SIZE + SHELL_BUFFER_SIZE)

/*! @brief The handle of the shell module */
typedef void *shell_handle_t;

struct tshell_config
{
    int (*t_putchar)(char ch);
};

static inline uint16_t t_shell_strlen(const char *str)
{
    return str == NULL ? 0U : (uint16_t) strlen(str);
}

#define T_SHELL_STATIC_ENTRY_PADDING 0

#define T_SHELL_T_STRINGIFY(x) #x
#define T_SHELL_STRINGIFY(s)   T_SHELL_T_STRINGIFY(s)

#define UTIL_CAT(a, ...)           UTIL_PRIMITIVE_CAT(a, __VA_ARGS__)
#define UTIL_PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

struct shell;

typedef int (*shell_cmd_handler)(void *shell, size_t argc, char **argv);

struct shell_static_args
{
    uint8_t mandatory; /*!< Number of mandatory arguments. */
    uint8_t optional;  /*!< Number of optional arguments. */
};

/*
 * @brief Shell static command descriptor.
 */
struct shell_static_entry
{
    const char                  *syntax;  /*!< Command syntax strings. */
    const char                  *help;    /*!< Command help string. */
    const union shell_cmd_entry *subcmd;  /*!< Pointer to subcommand. */
    shell_cmd_handler            handler; /*!< Command handler. */
    struct shell_static_args     args;    /*!< Command arguments. */
    uint8_t                      padding[T_SHELL_STATIC_ENTRY_PADDING];
};

/**
 * @brief Shell command descriptor.
 */
union shell_cmd_entry
{
    /*!< Placeholder for dynamic entry. */

    /*!< Pointer to array of static commands. */
    const struct shell_static_entry *entry;
};

/**
 * @brief Macro for defining and adding a root command (level 0) with required
 * number of arguments.
 *
 * @note Each root command shall have unique syntax. If a command will be called
 * with wrong number of arguments shell will print an error message and command
 * handler will not be called.
 *
 * @param[in] syntax	Command syntax (for example: history).
 * @param[in] subcmd	Pointer to a subcommands array.
 * @param[in] help	Pointer to a command help string.
 * @param[in] handler	Pointer to a function handler.
 * @param[in] mandatory	Number of mandatory arguments including command name.
 * @param[in] optional	Number of optional arguments.
 */
#ifndef TSHELL_HELP_DISABLED
#define SHELL_CMD_ARG_REGISTER(syntax, subcmd, help, handler, mandatory, optional)                                     \
    static const struct shell_static_entry UTIL_CAT(_shell_, syntax) =                                                 \
        SHELL_CMD_ARG(syntax, subcmd, help, handler, mandatory, optional);                                             \
    static const union shell_cmd_entry UTIL_CAT(shell_cmd_, syntax)                                                    \
        __attribute__((section("." T_SHELL_STRINGIFY(UTIL_CAT(shell_root_cmd_, syntax)))))                             \
        __attribute__((used)) = {.entry = &UTIL_CAT(_shell_, syntax)}
#else
#define SHELL_CMD_ARG_REGISTER(syntax, subcmd, help, handler, mandatory, optional)                                     \
    static const struct shell_static_entry UTIL_CAT(_shell_, syntax) =                                                 \
        SHELL_CMD_ARG(syntax, subcmd, "", handler, mandatory, optional);                                               \
    static const union shell_cmd_entry UTIL_CAT(shell_cmd_, syntax)                                                    \
        __attribute__((section("." T_SHELL_STRINGIFY(UTIL_CAT(shell_root_cmd_, syntax)))))                             \
        __attribute__((used)) = {.entry = &UTIL_CAT(_shell_, syntax)}
#endif

/**
 * @brief Macro for creating a subcommand set. It must be used outside of any
 * function body.
 *
 * Example usage:
 * SHELL_STATIC_SUBCMD_SET_CREATE(
 *	foo,
 *	SHELL_CMD(abc, ...),
 *	SHELL_CMD(def, ...),
 *	SHELL_SUBCMD_SET_END
 * )
 *
 * @param[in] name	Name of the subcommand set.
 * @param[in] ...	List of commands created with @ref SHELL_CMD_ARG or
 *			or @ref SHELL_CMD
 */
#define SHELL_STATIC_SUBCMD_SET_CREATE(name, ...)                                                                      \
    static const struct shell_static_entry shell_##name[] = {__VA_ARGS__};                                             \
    static const union shell_cmd_entry     name           = {.entry = shell_##name}

/**
 * @brief Initializes a shell command.
 * @param[in] _subcmd	Pointer to a subcommands array.
 * @param[in] _handler	Pointer to a function handler.
 */
#define SHELL_CMD(_syntax, _subcmd, _help, _handler) SHELL_CMD_ARG(_syntax, _subcmd, _help, _handler, 0, 0)

#if defined(__cplusplus)

/**
 * @brief The lambda function which parse a certain type of argument, by invoking a certain
 * tshell_parse_args_uint8/int8/{type} function.
 * @param[in] _type	         Target type for parsing (uint8_t, int8_t, etc).
 * @param[in] _min	         Min value which defined for the argument.
 * @param[in] _max	         Max value which defined for the argument.
 * @param[in] _handler_type	 The final function which will be invoked after successful parsing.
 */
#define SHELL_CMD_PARSED_TYPE_FN(_type, _min, _max, _handler_type)                                                     \
    +[](const struct shell *, size_t argc, char **argv) -> int                                                         \
    {                                                                                                                  \
        static_assert(_min <= _max);                                                                                   \
        _type##_t _v;                                                                                                  \
        if (tshell_parse_args_##_type(argc, argv, _min, _max, &_v) < 0)                                                \
            return -1;                                                                                                 \
        auto _l = _handler_type;                                                                                       \
        _l(_v);                                                                                                        \
        return 0;                                                                                                      \
    }

#define SHELL_CMD_NO_ARGS_FN(_handler_no_args)                                                                         \
    +[](const struct shell *, size_t argc, char **argv) -> int                                                         \
    {                                                                                                                  \
        (void) argc;                                                                                                   \
        (void) argv;                                                                                                   \
        auto _l = _handler_no_args;                                                                                    \
        _l();                                                                                                          \
        return 0;                                                                                                      \
    }

#define SHELL_CMD_PARSED_UINT8(_syntax, _help, _min, _max, _handler_uint8)                                             \
    SHELL_CMD(_syntax, NULL, _help, SHELL_CMD_PARSED_TYPE_FN(uint8, _min, _max, _handler_uint8))

#define SHELL_CMD_PARSED_INT8(_syntax, _help, _min, _max, _handler_int8)                                               \
    SHELL_CMD(_syntax, NULL, _help, SHELL_CMD_PARSED_TYPE_FN(int8, _min, _max, _handler_int8))

#define SHELL_CMD_PARSED_UINT32(_syntax, _help, _min, _max, _handler_uint32)                                           \
    SHELL_CMD(_syntax, NULL, _help, SHELL_CMD_PARSED_TYPE_FN(uint32, _min, _max, _handler_uint32))

#define SHELL_CMD_PARSED_INT32(_syntax, _help, _min, _max, _handler_int32)                                             \
    SHELL_CMD(_syntax, NULL, _help, SHELL_CMD_PARSED_TYPE_FN(int32, _min, _max, _handler_int32))

#define SHELL_CMD_NO_ARGS(_syntax, _help, _handler_no_arg)                                                             \
    SHELL_CMD(_syntax, NULL, _help, SHELL_CMD_NO_ARGS_FN(_handler_no_arg))

#endif /* __cplusplus */

/**
 * @brief Initializes a shell command with arguments.
 * @note If a command will be called with wrong number of arguments shell will
 * print an error message and command handler will not be called.
 * @param[in] subcmd	 Pointer to a subcommands array.
 * @param[in] handler	 Pointer to a function handler.
 */
#define SHELL_CMD_ARG(syntax, subcmd, help, handler, mand, opt)                                                        \
    SHELL_EXPR_CMD_ARG(1, syntax, subcmd, help, handler, mand, opt)

/**
 * @brief Initializes a conditional shell command with arguments if expression
 *	  gives non-zero result at compile time.
 *
 * @see SHELL_CMD_ARG. Based on the expression, creates a valid entry or an
 * empty command which is ignored by the shell. It should be used instead of
 * @ref SHELL_COND_CMD_ARG if condition is not a single configuration flag,
 * e.g.:
 * SHELL_EXPR_CMD_ARG(IS_ENABLED(CONFIG_FOO) &&
 *		      IS_ENABLED(CONFIG_FOO_SETTING_1), ...)
 *
 * @param[in] _expr	 Expression.
 * @param[in] _syntax	 Command syntax (for example: history).
 * @param[in] _subcmd	 Pointer to a subcommands array.
 * @param[in] _help	 Pointer to a command help string.
 * @param[in] _handler	 Pointer to a function handler.
 * @param[in] _mand	 Number of mandatory arguments including command name.
 * @param[in] _opt	 Number of optional arguments.
 */
#define SHELL_EXPR_CMD_ARG(_expr, _syntax, _subcmd, _help, _handler, _mand, _opt)                                      \
    {                                                                                                                  \
        .syntax  = (_expr) ? (const char *) T_SHELL_STRINGIFY(_syntax) : "",                                           \
        .help    = (_expr) ? (const char *) _help : NULL,                                                              \
        .subcmd  = (const union shell_cmd_entry *) ((_expr) ? _subcmd : NULL),                                         \
        .handler = (shell_cmd_handler) ((_expr) ? _handler : NULL), .args = {                                          \
            .mandatory = _mand,                                                                                        \
            .optional  = _opt                                                                                          \
        }                                                                                                              \
    }

/**
 * @brief Define ending subcommands set.
 *
 */
#define SHELL_SUBCMD_SET_END                                                                                           \
    {                                                                                                                  \
        NULL                                                                                                           \
    }

/*!
 * @brief Gets the shell command pointer
 *
 * This macro is used to get the shell command pointer. The macro should not be used before the macro
 * SHELL_COMMAND_DEFINE is used.
 *
 * @param command The command string of the command. The double quotes do not need. Such as exit for "exit",
 *                help for "Help", read for "read".
 */
#define SHELL_COMMAND(command) &g_shellCommand##command

#if defined(__cplusplus)
extern "C"
{
#endif /* _cplusplus */

    /*!
     * @brief Initializes the shell module
     *
     * This function must be called before calling all other Shell functions.
     * Call operation the Shell commands with user-defined settings.
     * The example below shows how to set up the Shell and
     * how to call the SHELL_Init function by passing in these parameters.
     * This is an example.
     * @code
     *   static SHELL_HANDLE_DEFINE(s_shellHandle);
     *   SHELL_Init((shell_handle_t)s_shellHandle, (serial_handle_t)s_serialHandle, "Test@SHELL>");
     * @endcode
     * @param shellHandle Pointer to point to a memory space of size #SHELL_HANDLE_SIZE allocated by the caller.
     * The handle should be 4 byte aligned, because unaligned access doesn't be supported on some devices.
     * You can define the handle in the following two ways:
     * #SHELL_HANDLE_DEFINE(shellHandle);
     * or
     * uint32_t shellHandle[((SHELL_HANDLE_SIZE + sizeof(uint32_t) - 1U) / sizeof(uint32_t))];
     * @param serialHandle The serial manager module handle pointer.
     * @param prompt  The string prompt pointer of Shell. Only the global variable can be passed.
     * @retval kStatus_SHELL_Success The shell initialization succeed.
     * @retval kStatus_SHELL_Error An error occurred when the shell is initialized.
     * @retval kStatus_SHELL_OpenWriteHandleFailed Open the write handle failed.
     * @retval kStatus_SHELL_OpenReadHandleFailed Open the read handle failed.
     */
    int tshell_init(const struct tshell_config *conf, char *prompt);

    /*!
     * @brief Sends data to the shell output stream.
     *
     * This function is used to send data to the shell output stream.
     *
     * @param shellHandle The shell module handle pointer.
     * @param buffer Start address of the data to write.
     * @param length Length of the data to write.
     * @retval kStatus_SHELL_Success Successfully send data.
     * @retval kStatus_SHELL_Error An error occurred.
     */
    int tshell_write(shell_handle_t shellHandle, char *buffer, uint32_t length);

    /**
     * @brief Change shell prompt.
     * @note Call this function to change shell prompt.
     * @param shellHandle The shell module handle pointer.
     * @param prompt The string which will be used for command prompt
     */
    void tshell_change_prompt(shell_handle_t shellHandle, char *prompt);

    /**
     * @brief Print shell prompt.
     * @note Call this function to print shell prompt.
     * @param shellHandle The shell module handle pointer.
     */
    void tshell_print_prompt(shell_handle_t shellHandle);

    /**
     * @brief Process a single char.
     */
    void tshell_process_char(uint8_t ch);

    /**
     * @brief Process a buffer.
     */
    void tshell_process_buffer(const uint8_t *buffer, size_t len);

#if defined(__cplusplus)
}
#endif
