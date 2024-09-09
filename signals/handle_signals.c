/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_signals.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 17:03:53 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/09 11:23:30 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

void sig_int(int code)
{
    (void)code;
    if (get_in_heredoc(-500))
    {
        write(STDERR_FILENO, "\n", 1);
        get_sigint(1);
        get_exit_status(1);
    }
    else if (get_pid(-500) == 0)
    {
        write(STDERR_FILENO, "\n", 1);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
        get_sigint(1);
        get_exit_status(1);
    }
    else
        write(STDERR_FILENO, "\n", 1);
}

void sig_quit(int code)
{
    char *nbr;
    nbr = ft_itoa(code);
    if (get_pid(-500) != 0)
    {
        ft_putstr_fd("Quit: \n", STDERR_FILENO);
        ft_putendl_fd(nbr, STDERR_FILENO);
       	get_exit_status(131);
        get_sigquit(1);
    }
    free(nbr);
}

void sig_init(void)
{
    get_sigint(0);
    get_sigquit(0);
    get_pid(0);
    get_exit_status(0);
}
