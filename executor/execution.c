/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 16:50:49 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/09 18:35:41 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

// int setup_and_handle_heredocs(t_setup_context *ctx)
// {
//     t_fd_tracker fd_tracker = {0};
//     t_arg *current_cmd;
//     int heredoc_count;

//     *(ctx->command_count) = count_commands(ctx->cmd);
//     *(ctx->pipe_count) = *(ctx->command_count) - 1;
//     save_original_io(ctx->io);
//     if (setup_pipes(*(ctx->pipe_count), ctx->pipe_fds, &fd_tracker) != 0)
//     {
//         restore_io(ctx->io);
//         return 1;
//     }
//     // Process all heredocs
//     current_cmd = ctx->cmd;
//     while (current_cmd) {
//         heredoc_count = count_heredocs(current_cmd->red);
//         if (heredoc_count > 0)
//         {
//             current_cmd->heredoc_fds = handle_heredocs(current_cmd->red, heredoc_count, ctx->env, &fd_tracker);
//             if (!current_cmd->heredoc_fds)
//             {
//                 restore_io(ctx->io);
//                 return 1;
//             }
//         }
//         current_cmd = current_cmd->next;
//     }
//     return 0;
// }
static int handle_parent_process(t_command_context1 *ctx, t_arg *current_cmd, pid_t *pids, int cmd_index)
{
    pids[cmd_index] = fork();
    if (pids[cmd_index] == -1)
    {
        perror("fork");
        restore_io(ctx->io);
        return 0;
    }
    else if (pids[cmd_index] == 0)
    {
        configure_child_io(ctx, current_cmd, cmd_index);
        execute_child_process(ctx, current_cmd);
    }
    restore_io(ctx->io);
    return 1;
}
static int process_commands(t_command_context1 *ctx, pid_t *pids)
{
    int child_count = 0;
    int cmd_index = 0;
    t_arg *current_cmd = ctx->cmd;

    while (current_cmd)
    {
        int is_builtins = current_cmd->arg != NULL && is_builtin(current_cmd->arg[0]);
        int is_parent_builtin = is_builtins && (ft_strcmp(current_cmd->arg[0], "cd") == 0 ||
                                                ft_strcmp(current_cmd->arg[0], "exit") == 0 ||
                                                ft_strcmp(current_cmd->arg[0], "export") == 0 ||
                                                ft_strcmp(current_cmd->arg[0], "unset") == 0);

        if (is_parent_builtin && ctx->pipe_count == 0)
            handle_parent_builtin(ctx, current_cmd);
        else
            child_count += handle_parent_process(ctx, current_cmd, pids, cmd_index);

        get_pid(pids[cmd_index]);
        current_cmd = current_cmd->next;
        cmd_index++;
    }
    return child_count;
}
int execute_commands(t_command_context1 *ctx)
{
    pid_t pids[MAX_COMMANDS];
    int child_count = process_commands(ctx, pids);
    
    cleanup_pipes(ctx);
    
    int result = 0;
    if (child_count > 0)
        result = wait_for_children(pids, child_count, ctx->exit_status);
    restore_io(ctx->io);
    return result;
}

int execute_command(t_command_context *context)
{
    t_execution_data exec_data = {0};
    t_setup_context setup_ctx;
    t_command_context1 cmd_ctx;
    int result;

    exec_data.command_count = 0;
    exec_data.pipe_count = 0;

    init_setup_context(&setup_ctx, context, &exec_data);

    if (setup_and_handle_heredocs(&setup_ctx) != 0)
    {
        close_all_fds(context->fd_tracker);
        return (1);
    }
    init_command_context1(&cmd_ctx, context, &exec_data);
    result = execute_commands(&cmd_ctx);
    cleanup_heredoc_fds(context->cmd, context->fd_tracker);
    return (result);
}