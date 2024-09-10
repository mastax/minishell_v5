/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 16:56:05 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/10 10:52:27 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"

void	free_command(t_arg *cmd)
{
	int i;
	t_arg *tmp;
	
	if (!cmd)
		return ;
	while (cmd)
	{
		i = 0;
		while (cmd->arg && cmd->arg[i])
		{
			free(cmd->arg[i]);
			i++;
		}
		free(cmd->arg);
		i = 0;
		while (cmd->red && cmd->red[i])
		{
			free(cmd->red[i]);
			i++;
		}
		free(cmd->red);
		tmp = cmd->next;
		free(cmd);
		cmd = tmp;
	}
}

void	free_tokens(t_token *tokens)
{
	t_token	*tmp;

	while (tokens)
	{
		tmp = tokens;
		tokens = tokens->next;
		free(tmp->content);
		free(tmp);
	}
}

void	restore_io(t_io *io)
{
	dup2(io->original_stdin, STDIN_FILENO);
	dup2(io->original_stdout, STDOUT_FILENO);
	close(io->original_stdin);
	close(io->original_stdout);
}

void	save_original_io(t_io *io)
{
	io->original_stdin = dup(STDIN_FILENO);
	io->original_stdout = dup(STDOUT_FILENO);
}


int	main(int ac, char **av, char **envp)
{
	// t_io	io;
	t_env	*env;
	int		status;
	t_fd_tracker fd_tracker = {0};  // Initialize all elements to 0

	(void)ac;
	(void)av;
	rl_catch_signals = 0;
	env = create_env(envp);
	if (!env)
	{
		fprintf(stderr, "Failed to create environment\n");
		return (1);
	}
	status = main_shell_loop(env, &fd_tracker);
	free_env(env);
	close_all_fds(&fd_tracker);
	return (status);
}
