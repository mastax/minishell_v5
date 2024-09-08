/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_utils1.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 21:16:39 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/08 16:08:21 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

static char *append_buffer(char *line, char *buffer, int bytes_read, int *total_size)
{
    char *new_line;
    int new_size = *total_size + bytes_read + 1;

    new_line = malloc(new_size);
    if (!new_line)
    {
        free(line);
        return NULL;
    }
    if (line)
    {
        ft_memcpy(new_line, line, *total_size);
        free(line);
    }
    ft_memcpy(new_line + *total_size, buffer, bytes_read);
    *total_size = new_size - 1;
    new_line[*total_size] = '\0';
    return new_line;
}

char *read_line(void)
{
    char *line;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int total_size;

    line = NULL;
    total_size = 0;
    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0)
    {
        line = append_buffer(line, buffer, bytes_read, &total_size);
        if (!line)
            return NULL;
        if (line[total_size - 1] == '\n')
        {
            line[total_size - 1] = '\0';
            return line;
        }
    }
    if (bytes_read == -1 || (bytes_read == 0 && total_size == 0))
    {
        free(line);
        return NULL;
    }
    return line;
}

void child_process(int pipefd[2], const char *delimiter, t_env *env, t_fd_tracker *fd_tracker)
{
    char *line;

    close(pipefd[0]);
    untrack_fd(fd_tracker, pipefd[0]);
    while ((line = read_line()) != NULL)
    {
        if (ft_strcmp(line, delimiter) == 0)
            break ;
        if (fd_tracker->qout == 0)
            if (expand_variable(&line, env) == -1)
                exit(1);
        write_to_pipe(pipefd, line);
        free(line);
    }
    close(pipefd[1]);
    untrack_fd(fd_tracker, pipefd[1]);
}

void parent_process(int pipefd[2], pid_t pid, t_fd_tracker *fd_tracker)
{
    (void)pid;
    int status;
    close(pipefd[1]); // Close write end of the pipe
    untrack_fd(fd_tracker, pipefd[1]);
    waitpid(pid, &status, 0);
}
int check_if_qoutes(char *s)
{
    int i;

    i = 0;
    while (s[i])
    {
        if (s[i] == '"' || s[i] == '\'')
            return (1);
        i++;
    }
    return (0);
}
