/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_heredoc.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/08 15:34:13 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/08 16:08:41 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"



static int  setup_pipe(int pipefd[2], t_fd_tracker *tracker)
{
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return -1;
    }
    track_fd(tracker, pipefd[0]);
    track_fd(tracker, pipefd[1]);
    return (0);
}

static int  handle_fork_error(int pipefd[2], t_fd_tracker *tracker)
{
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    untrack_fd(tracker, pipefd[0]);
    untrack_fd(tracker, pipefd[1]);
    get_in_heredoc(0);  // Reset the flag
    return (-1);
}

int create_heredoc(const char *delimiter, t_env *env, t_fd_tracker *tracker)
{
    int pipefd[2];
    pid_t pid;

    if (setup_pipe(pipefd, tracker) == -1)
        return (-1);
    get_in_heredoc(1);
    pid = fork();
    if (pid == -1)
        return handle_fork_error(pipefd, tracker);
    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        child_process(pipefd, delimiter, env, tracker);
        exit(0);
    }
    parent_process(pipefd, pid, tracker);
    get_in_heredoc(0);  // Reset the flag
    untrack_fd(tracker, pipefd[1]);
    return (pipefd[0]);
}
