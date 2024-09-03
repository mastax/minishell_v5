/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_signals.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 17:03:53 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/02 18:19:28 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

void sig_int(int code)
{
    (void)code;
    if (g_sig.pid == 0)
    {
        write(STDERR_FILENO, "\n", 1);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
    else
    {
        write(STDERR_FILENO, "\n", 1);
    }
    g_sig.sigint = 1;
    g_sig.exit_status = 1;
    // g_sig.exit_status = 130;
}

void sig_quit(int code)
{
    char *nbr;
    nbr = ft_itoa(code);
    if (g_sig.pid != 0)
    {
        ft_putstr_fd("Quit: \n", STDERR_FILENO);
        ft_putendl_fd(nbr, STDERR_FILENO);
        g_sig.exit_status = 131;
        g_sig.sigquit = 1;
    }
    // else
    //     ft_putstr_fd("\b\b  \b\b", STDERR_FILENO);
    free(nbr);
}

void sig_init(void)
{
    g_sig.sigint = 0;
    g_sig.sigquit = 0;
    g_sig.pid = 0;
    g_sig.exit_status = 0;
}