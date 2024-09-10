/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 20:59:26 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/10 12:42:54 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

int setup_and_handle_heredocs(t_setup_context *ctx)//execution
{
    t_fd_tracker fd_tracker = {0};
    t_arg *current_cmd;
    int heredoc_count;

    *(ctx->command_count) = count_commands(ctx->cmd);
    *(ctx->pipe_count) = *(ctx->command_count) - 1;
    save_original_io(ctx->io);
    if (setup_pipes(*(ctx->pipe_count), ctx->pipe_fds, &fd_tracker) != 0)
    {
        restore_io(ctx->io);
        return 1;
    }
    // Process all heredocs
    current_cmd = ctx->cmd;
    while (current_cmd) {
        heredoc_count = count_heredocs(current_cmd->red);
        if (heredoc_count > 0)
        {
            current_cmd->heredoc_fds = handle_heredocs(current_cmd->red, heredoc_count, ctx->env, &fd_tracker);
            if (!current_cmd->heredoc_fds)
            {
                restore_io(ctx->io);
                return 1;
            }
        }
        current_cmd = current_cmd->next;
    }
    return 0;
}

static char *get_heredoc_delimiter(char **red, int index)
{
    int count = 0;
    int i = 0;
    while (red && red[i])
    {
        if (ft_strcmp(red[i], "<<") == 0)
        {
            if (count == index && red[i + 1])
                return (ft_strdup(red[i + 1]));
            count++;
        }
        i++;
    }
    return NULL;
}

typedef struct s_delimiter_data {
    char **red;
    int i;
    char *delimiter;
    char *processed_delimiter;
    t_fd_tracker *tracker;
} t_delimiter_data;

int process_delimiter(t_delimiter_data *data)
{
    int j;

    j = 0;
    data->delimiter = get_heredoc_delimiter(data->red, data->i);
    if (!data->delimiter)
        return 0;
    while (data->delimiter[j])
    {
        if (data->delimiter[j] == '$' && (data->delimiter[j + 1] == '"'
            || data->delimiter[j + 1] == '\''))
        {
            data->delimiter = ft_remove_char(data->delimiter, j);
            if (!data->delimiter)
                return (0);
        }
        j++;
    }
    data->tracker->qout = check_if_qoutes(data->delimiter);
    if (data->tracker->qout == 1)
        data->processed_delimiter = ft_remove_quotes(data->delimiter);
    else
        data->processed_delimiter = data->delimiter;
    return data->processed_delimiter != NULL;
}

void cleanup_and_return(int *heredoc_fds, int count, t_fd_tracker *tracker)
{
    int j;

    j = 0;
    while (j < count)
    {
        close(heredoc_fds[j]);
        untrack_fd(tracker, heredoc_fds[j]);
        j++;
    }
    free(heredoc_fds);
}

int *handle_heredocs(char **red, int count, t_env *env, t_fd_tracker *tracker)
{
    int *heredoc_fds;
    t_delimiter_data data;
    int i;

    heredoc_fds = malloc(sizeof(int) * count);
    if (!heredoc_fds)
        return NULL;
    i = 0;
    while (i < count)
    {
        data = (t_delimiter_data){red, i, NULL, NULL, tracker};
        if (!process_delimiter(&data))
            return(cleanup_and_return(heredoc_fds, i, tracker), NULL);
        heredoc_fds[i] = create_heredoc(data.processed_delimiter, env, tracker);
        free(data.processed_delimiter);
        if (heredoc_fds[i] == -1)
            return (cleanup_and_return(heredoc_fds, i, tracker), NULL);
        i++;
    }
    return heredoc_fds;
}

// int process_delimiter(char **red, int i, char **delimiter, char **processed_delimiter, t_fd_tracker *tracker)
// {
//     int j;

//     j = 0;
//     *delimiter = get_heredoc_delimiter(red, i);
//     if (!*delimiter)
//         return 0;
//     while (delimiter[0][j])
//     {
//         if (delimiter[0][j] == '$' && (delimiter[0][j + 1] == '"'
// 			|| delimiter[0][j + 1] == '\''))
// 		{
//             delimiter[0] = ft_remove_char(delimiter[0], j);
//             if (!delimiter[0])
//                 return (0);
//         }
//         j++;
//     }
//     tracker->qout = check_if_qoutes(*delimiter);
//     if (tracker->qout == 1)
//         *processed_delimiter = ft_remove_quotes(*delimiter);
//     else
//         *processed_delimiter = *delimiter;
//     return *processed_delimiter != NULL;
// }

// int *handle_heredocs(char **red, int count, t_env *env, t_fd_tracker *tracker)
// {
//     int     *heredoc_fds;
//     char    *delimiter;
//     char    *processed_delimiter;
//     int     i;

//     heredoc_fds = malloc(sizeof(int) * count);
//     if (!heredoc_fds)
//         return NULL;
//     i = 0;
//     while (i < count)
//     {
//         if (!process_delimiter(red, i, &delimiter, &processed_delimiter, tracker))
//             return(cleanup_and_return(heredoc_fds, i, tracker), NULL);
//         heredoc_fds[i] = create_heredoc(processed_delimiter, env, tracker);
//         free(processed_delimiter);
//         if (heredoc_fds[i] == -1)
//             return (cleanup_and_return(heredoc_fds, i, tracker), NULL);
//         i++;
//     }
//     return heredoc_fds;
// }
