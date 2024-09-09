/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fd_leaks_tracker.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 15:38:41 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/09 20:43:11 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

void track_fd(t_fd_tracker *tracker, int fd)
{
    if (fd >= 0 && fd < MAX_FD)
        tracker->fd_table[fd] = 1;
}

void untrack_fd(t_fd_tracker *tracker, int fd)
{
    if (fd >= 0 && fd < MAX_FD)
        tracker->fd_table[fd] = 0;
}

void close_all_fds(t_fd_tracker *tracker)
{
    int fd;

    fd = 3;
    while (fd < MAX_FD)
    {
        if (tracker->fd_table[fd])
        {
            close(fd);
            untrack_fd(tracker, fd);
        }
        fd++;
    }
}

void cleanup_command_heredocs(t_arg *cmd, t_fd_tracker *fd_tracker)
{
    int i;
    int heredoc_count;

    i = 0;
    heredoc_count = count_heredocs(cmd->red);
    while (i < heredoc_count) {
        if (cmd->heredoc_fds[i] > 2)
        {
            close(cmd->heredoc_fds[i]);
            untrack_fd(fd_tracker, cmd->heredoc_fds[i]);
        }
        i++;
    }
}

