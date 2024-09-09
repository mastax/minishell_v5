/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_signals_utils.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 10:52:46 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/09 10:54:41 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

int get_sigint(int n)
{
	static int  v;

    v = 0;
	if (n != -500)
		v = n;
	return (v);
}
int get_sigquit(int n)
{
	static int v;

    v = 0;
	if (n != -500)
		v = n;
	return (v);
}
int get_pid(int n)
{
	static int v;

    v = 0;
	if (n != -500)
		v = n;
	return (v);
}
int get_exit_status(int n)
{
	static int v;

    v = 0;
	if (n != -500)
		v = n;
	return (v);
}
int get_in_heredoc(int n)
{
	static int  v;

    v = 0;
	if (n != -500)
		v = n;
	return (v);
}