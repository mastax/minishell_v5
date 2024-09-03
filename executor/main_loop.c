/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 16:56:01 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/03 10:54:41 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

int setup_pipes(int pipe_count, int pipe_fds[][2], t_fd_tracker *fd_tracker)
{
    (void)fd_tracker;
    for (int i = 0; i < pipe_count; i++) {
        if (pipe(pipe_fds[i]) == -1) {
            perror("pipe");
            return 1;
        }
        track_fd(fd_tracker, pipe_fds[i][0]);
        track_fd(fd_tracker, pipe_fds[i][1]);
    }
    return 0;
}

// int setup_pipes(int pipe_count, int pipe_fds[][2])
// {
//     for (int i = 0; i < pipe_count; i++) {
//         if (pipe(pipe_fds[i]) == -1) {
//             perror("pipe");
//             return 1;
//         }
//     }
//     return 0;
// }

static int handle_input(char *input, t_env *env)
{
    (void)env;
    if (!input)
    {
        ft_putstr_fd("\nExiting shell.\n", 1);
        return (1);
    }
    if (*input)
        add_history(input);
    return (0);
}

static int process_command(t_token *tokens, t_env *env, int *exit_status)
{
    t_arg *cmd;
    int result;
    t_fd_tracker fd_tracker = {0};

    if (!tokens)
        return (0);
    cmd = ft_arg_new(2);
    if (!cmd)
        return (0);
    if (ft_convert_token_to_arg(tokens, cmd, 0) == 0)
    {
        t_command_context context;
        context.cmd = cmd;
        context.env = env;
        context.exit_status = exit_status;
        context.fd_tracker = &fd_tracker;

        result = execute_command(&context);
        // result = execute_command(cmd, env, exit_status, &fd_tracker);
        free_command(cmd);
        return result;  // Return the result from execute_command
    }
    free_command(cmd);
    return (0);
}

// static int process_command(t_token *tokens, t_env *env, int *exit_status)
// {
//     t_arg *cmd;
//     int result;
//     t_fd_tracker fd_tracker = {0};

//     if (!tokens)
//         return (0);
//     cmd = ft_arg_new(2);
//     if (!cmd)
//         return (0);
//     if (ft_convert_token_to_arg(tokens, cmd, 0) == 0)
//     {
//         result = execute_command(cmd, env, exit_status);
//         free_command(cmd);
//         return result;  // Return the result from execute_command
//     }
//     free_command(cmd);
//     return (0);
// }

#define GREEN "\001\033[1;32m\002"
#define RESET "\001\033[0m\002"
//=-=-=-=-=-=-=-

int handle_parse_result(int parse_result, t_token **tokens, int *exit_status)
{
    if (parse_result == -1)
    {
        free_tokens(*tokens);
        g_sig.exit_status = 1;
    }
    else if (parse_result == 258)
    {
        free_tokens(*tokens);
        g_sig.exit_status = 258;
    }
    *exit_status = g_sig.exit_status;
    return (parse_result);
}

void initialize_loop_iteration(t_token **tokens)
{
    *tokens = NULL;
    g_sig.pid = 0;
    g_sig.sigint = 0;
    signal(SIGINT, sig_int);
    signal(SIGQUIT, sig_quit);
}

int process_input(char *input, t_env *env, t_token **tokens, int *exit_status)
{
    int parse_result;

    if (handle_input(input, env))
        return (-1);
    parse_result = parsing(input, tokens, env, *exit_status);
    if (parse_result == 0)
    {
        if (process_command(*tokens, env, &g_sig.exit_status) == -1)
        {
            free_tokens(*tokens);
            return (-1);
        }
        free_tokens(*tokens);
        *tokens = NULL;
    }
    return (handle_parse_result(parse_result, tokens, exit_status));
}

int main_shell_loop(t_env *env, t_fd_tracker *fd_tracker)
{
    char *input;
    t_token *tokens;
    int exit_status = 0;

    sig_init();
    while (1)
    {
        initialize_loop_iteration(&tokens);
        input = readline(GREEN"minishell> "RESET);
        if (input == NULL)
        {
            write(STDERR_FILENO, "exit\n", 6);
            break;
        }
        if (g_sig.sigint)
        {
            exit_status = g_sig.exit_status;
            g_sig.sigint = 0;
        }
        if (process_input(input, env, &tokens, &exit_status) == -1)
        {
            close_all_fds(fd_tracker);
            // return (exit_status);
        }
        close_all_fds(fd_tracker);
    }
    close_all_fds(fd_tracker);
    return (exit_status);
}

//=-=-=-=-=-=-=-
// int main_shell_loop(t_env *env, t_fd_tracker *fd_tracker)
// {
//     char *input;
//     t_token *tokens;
//     int exit_status;
//     int parse_result;

//     exit_status = 0;
//     sig_init();
//     while (1)
//     {
//         tokens = NULL;
//         g_sig.pid = 0;  // Reset pid before each command
//         g_sig.sigint = 0;  // Reset sigint flag
//         signal(SIGINT, sig_int);
//         signal(SIGQUIT, sig_quit);
//         input = readline(GREEN"minishell> "RESET);
//         if (input == NULL)
//         {
//             write(STDERR_FILENO, "exit\n", 6);
//             break;
//         }
//         if (g_sig.sigint)//for ctrl-c than echo $?
//         {
//             exit_status = g_sig.exit_status;
//             g_sig.sigint = 0;  // Reset the flag
//         }
//         if (handle_input(input, env))
//             return (exit_status);
//         parse_result = parsing(input, &tokens, env, exit_status);
//         if (parse_result == 0)
//         {
//             if (process_command(tokens, env, &g_sig.exit_status) == -1)
//             {
//                 free_tokens(tokens);
//                 close_all_fds(fd_tracker);
//                 return (exit_status);
//             }
//             free_tokens(tokens);
//             tokens = NULL;
//         }
//         else if (parse_result == -1)
//         {
//             free_tokens(tokens);
//             g_sig.exit_status = 1;
//         }
//         else if (parse_result == 258)
//         {
//             free_tokens(tokens);
//             g_sig.exit_status = 258;
//         }
//         exit_status = g_sig.exit_status;
//         close_all_fds(fd_tracker);
//     }
//     close_all_fds(fd_tracker);
//     return (exit_status);
// }
