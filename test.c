//heredoc

#define BUFFER_SIZE 1024

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

static int read_line_interrupted = 0;

static void read_line_signal_handler(int sig)
{
    (void)sig;
    read_line_interrupted = 1;
}

char *read_line(void)
{
    char *line = NULL;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int total_size = 0;

    signal(SIGINT, read_line_signal_handler);

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0)
    {
        if (read_line_interrupted)
        {
            free(line);
            read_line_interrupted = 0;
            return NULL;
        }

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

int	ft_expand_herdoc_var(char **var, t_env *env, t_type prv_type, int i)
{
	int		j;
	char	*var_name;

	while (var[0][i] != '\0')
	{
		if (var[0][i] == '\'')
			i = ft_skipe_qoute(*var, i);
		else if (var[0][i] == '$' && (is_valid_var(var[0][i + 1]) == 1))
		{
			j = ft_name_len(*var, i + 1);
			var_name = ft_substr(*var, i + 1, j);
			if (!var_name)
				return (-1);
			if (prv_type == WORD)
				if (ft_var_update(i, var, get_var_from_env(var_name, j, env),
						&var[0][i + j + 1]) == -1)
					return (-1);
		}
		else if ((var[0][i] == '$' && (is_valid_var(var[0][i + 1]) != 1))
			|| (var[0][i] != '$' && var[0][i] != '\0'))
			i++;
	}
	return (0);
}

int expand_variable(char **line, t_env *env)
{
    char *expanded_line;

    expanded_line = ft_strdup(*line);
    if (expanded_line == NULL)
    {
        perror("ft_strdup");
        free(*line);
        return -1;
    }
    if (ft_expand_herdoc_var(&expanded_line, env, WORD, 0) == -1)
    {
        free(expanded_line);
        free(*line);
        return -1;
    }

    free(*line);
    *line = expanded_line;
    return 0;
}

int write_to_pipe(int pipefd[2], char *line)
{
    write(pipefd[1], line, ft_strlen(line));
    write(pipefd[1], "\n", 1);
    return 0;
}
int count_heredocs(char **red)
{
    int count;
    int i;

    count = 0;
    i = 0;
    if (!red || !(*red))
        return (0);
    while (red[i] != NULL)
    {
        if (ft_strcmp(red[i], "<<") == 0)
            count++;
        i++;
    }
    return count;
}


void child_process(int pipefd[2], const char *delimiter, t_env *env, t_fd_tracker *fd_tracker)
{
    char *line;

    close(pipefd[0]); // Close read end of the pipe
    untrack_fd(fd_tracker, pipefd[0]);
    while ((line = read_line()) != NULL)
    {
        if (g_sig.sigint)///
        {
            close(pipefd[1]);
            untrack_fd(fd_tracker, pipefd[1]);
            exit(1);
        }///
        if (ft_strcmp(line, delimiter) == 0)
            break;
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
    close(pipefd[1]); // Close write end of the pipe
    untrack_fd(fd_tracker, pipefd[1]);
    waitpid(pid, NULL, 0); // Wait for the child process to finish
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

static char *get_heredoc_delimiter(char **red, int index)
{
    int count = 0;
    int i = 0;
    while (red && red[i])
    {
        if (ft_strcmp(red[i], "<<") == 0)
        {
            if (count == index && red[i + 1])
                return red[i + 1];
            count++;
        }
        i++;
    }
    return NULL;
}

int process_delimiter(char **red, int i, char **delimiter, char **processed_delimiter, t_fd_tracker *tracker)
{
    *delimiter = get_heredoc_delimiter(red, i);
    if (!*delimiter)
        return 0;

    tracker->qout = check_if_qoutes(*delimiter);
    if (tracker->qout == 1)
        *processed_delimiter = ft_remove_quotes(strdup(*delimiter));
    else
        *processed_delimiter = ft_strdup(*delimiter);
    return *processed_delimiter != NULL;
}
int create_heredoc(const char *delimiter, t_env *env, t_fd_tracker *tracker)
{
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return -1;
    }
    track_fd(tracker, pipefd[0]);
    track_fd(tracker, pipefd[1]);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        untrack_fd(tracker, pipefd[0]);
        untrack_fd(tracker, pipefd[1]);
        return -1;
    }
    if (pid == 0) { // Child process
        child_process(pipefd, delimiter, env, tracker);
        exit(0);
    } else { // Parent process
        parent_process(pipefd, pid, tracker);
        untrack_fd(tracker, pipefd[1]); // Untrack the write end that we closed
        return pipefd[0];
    }
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
    int     *heredoc_fds;
    char    *delimiter;
    char    *processed_delimiter;
    int     i;

    heredoc_fds = malloc(sizeof(int) * count);
    if (!heredoc_fds)
        return NULL;
    i = 0;
    while (i < count)
    {
        if (!process_delimiter(red, i, &delimiter, &processed_delimiter, tracker))
            return(cleanup_and_return(heredoc_fds, i, tracker), NULL);
        heredoc_fds[i] = create_heredoc(processed_delimiter, env, tracker);
        free(processed_delimiter);
        if (heredoc_fds[i] == -1)
            return (cleanup_and_return(heredoc_fds, i, tracker), NULL);
        i++;
    }
    return heredoc_fds;
}
//execution

void	free_command(t_arg *cmd)
{
	t_arg *tmp;
	
	if (!cmd)
		return ;

	// Free arguments
	while (cmd)
	{
		tmp = cmd->next;//if(cmd->arg != NULL)
		for (int i = 0; cmd->arg && cmd->arg[i]; i++)
			free(cmd->arg[i]);//if (cmd->srg[i] != NULL)free()
		free(cmd->arg);
		// Free redirections
		for (int i = 0; cmd->red && cmd->red[i]; i++)
			free(cmd->red[i]);
		free(cmd->red);
		// Free the command structure itself
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

char *get_env_value(t_env *env, const char *key) //helper cd
{
	int key_len;
	int i;

	key_len = ft_strlen(key);
	i = 0;
	while (i < env->count)
	{
		if (ft_strncmp(env->env_vars[i], key, key_len) == 0 && env->env_vars[i][key_len] == '=')
			return env->env_vars[i] + key_len + 1;
		i++;
	}
	return NULL;
}

int change_to_home(t_env *env)
{
	char *home;

	home = get_env_value(env, "HOME");
	if (!home)
	{
		ft_putstr_fd("cd: HOME not set\n", STDERR_FILENO);
		return (1);
	}
	return (chdir(home));
}

int change_to_previous(t_env *env)
{
	char *oldpwd;

	oldpwd = get_env_value(env, "OLDPWD");
	if (!oldpwd)
	{
		ft_putstr_fd("cd: OLDPWD not set\n", STDERR_FILENO);
		return (1);
	}
	ft_putendl_fd(oldpwd, STDOUT_FILENO);
	return (chdir(oldpwd));
}

t_env *allocate_env(int count)
{
	t_env *env;

	env = malloc(sizeof(t_env));
	if (!env)
		return (NULL);
	env->env_vars = malloc(sizeof(char *) * (count + 1));
	if (!env->env_vars)
	{
		free(env);
		return (NULL);
	}
	env->count = count;
	return (env);
}

t_env *create_env(char **envp)
{
	t_env *env;
	int count;
	int i;

	count = 0;
	while (envp[count])
		count++;
	if (count == 0)
		return (create_env_v_i(5));
	env = allocate_env(count);
	if (!env)
		return (NULL);
	i = 0;
	while (i < count && (env->env_vars[i] = ft_strdup(envp[i])))
		i++;
	if (i < count)
	{
		while (--i >= 0)
			free(env->env_vars[i]);
		free(env->env_vars);
		free(env);
		return (NULL);
	}
	env->env_vars[count] = NULL;
	return (env);
}

t_env *populate_env_vars(t_env *env, const char **vars)
{
	int	i;
	int	j;
	char current_dir[PATH_MAX];

	i = 0;
	j = 0;
	if (getcwd(current_dir, sizeof(current_dir)) == NULL)
		return (NULL);
	while (i < env->count)
	{
		if (i == 1)
			env->env_vars[i] = ft_strjoin(ft_strdup(vars[i]), current_dir);
		else
			env->env_vars[i] = ft_strdup(vars[i]);
		if (!env->env_vars[i])
		{
			while (j < i)
				free(env->env_vars[j++]);
			return (free(env->env_vars), free(env), NULL);
		}
		i++;
	}
	env->env_vars[env->count] = NULL;
	return (env);
}

t_env *create_env_v_i(int count)
{
	t_env *env;
	const char *init_vars[6];

    init_vars[0] = "PATH=/usr/gnu/bin:/usr/local/bin:/bin:/usr/bin:.";
    init_vars[1] = "PWD=";
    init_vars[2] = "OLDPWD";
    init_vars[3] = "SHLVL=1";
    init_vars[4] = "_=/usr/bin/env";
    init_vars[5] = NULL;
	env = malloc(sizeof(t_env));
	if (!env)
		return (NULL);
	env->env_vars = malloc(sizeof(char *) * (count + 1));
	if (!env->env_vars)
		return (free(env), NULL);
	env->count = count;
	return (populate_env_vars(env, init_vars));
}
void cleanup_parent_process(int *heredoc_fds, int heredoc_count)
{
    if (heredoc_fds) {
        for (int i = 0; i < heredoc_count; i++)
            close(heredoc_fds[i]);
        free(heredoc_fds);
    }
}

int count_commands(t_arg *cmd)
{
    int count = 0;
    while (cmd)
    {
        count++;
        cmd = cmd->next;
    }
    return count;
}

int wait_for_children(pid_t *pids, int command_count, int *exit_status)
{
    int status;
    int i = 0;
    while (i < command_count)
    {
        if (waitpid(pids[i], &status, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status))
            *exit_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            *exit_status = 128 + WTERMSIG(status);
        if (i == command_count - 1)
            g_sig.exit_status = *exit_status;
        i++;
    }
    return *exit_status;
}

void setup_child_process(child_setup_params *params)
{
    int i;

    if (params->cmd_index > 0)
        dup2(params->pipe_fds[params->cmd_index - 1][0], STDIN_FILENO);
    if (params->cmd_index < params->pipe_count)
        dup2(params->pipe_fds[params->cmd_index][1], STDOUT_FILENO);
    i = 0;
    while (i < params->pipe_count)
    {
        close(params->pipe_fds[i][0]);
        close(params->pipe_fds[i][1]);
        i++;
    }
    if (params->heredoc_fds && params->heredoc_count > 0)
    {
        dup2(params->heredoc_fds[params->heredoc_count - 1], STDIN_FILENO);
        i = 0;
        while (i < params->heredoc_count)
        {
            close(params->heredoc_fds[i]);
            i++;
        }
        // Don't free heredoc_fds here, it will be freed in the parent process
    }
}

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

int setup_and_handle_heredocs(t_setup_context *ctx)
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

typedef struct s_command_context1 {
    t_arg *cmd;
    t_env *env;
    int *exit_status;
    t_io *io;
    int (*pipe_fds)[2];
    int command_count;
    int pipe_count;
} t_command_context1;


//=-=-=-=-=-

static void configure_child_io(t_command_context1 *ctx, t_arg *current_cmd, int cmd_index)
{
    if (cmd_index > 0)
        dup2(ctx->pipe_fds[cmd_index - 1][0], STDIN_FILENO);
    if (cmd_index < ctx->pipe_count)
        dup2(ctx->pipe_fds[cmd_index][1], STDOUT_FILENO);
    for (int i = 0; i < ctx->pipe_count; i++)
    {
        close(ctx->pipe_fds[i][0]);
        close(ctx->pipe_fds[i][1]);
    }
    if (current_cmd->heredoc_fds)
    {
        for (int i = 0; i < count_heredocs(current_cmd->red); i++)
        {
            dup2(current_cmd->heredoc_fds[i], STDIN_FILENO);
            close(current_cmd->heredoc_fds[i]);
        }
    }
    if (apply_redirections(current_cmd->red) == -1)
        exit(1);
}

static void execute_child_process(t_command_context1 *ctx, t_arg *current_cmd)
{
    if (current_cmd->arg == NULL || current_cmd->arg[0] == NULL)
        exit(0);
    if (is_builtin(current_cmd->arg[0]))
    {
        int builtin_result = execute_builtin_ch(current_cmd, ctx->env, ctx->exit_status);
        exit(builtin_result);
    }
    if (current_cmd->arg[0][0] == '\0')
    {
        ft_putstr_fd(": command not found\n", 2);
        exit(127);
    }
    char *cmd_path = ft_get_path(current_cmd->arg[0], ctx->exit_status);
    if (cmd_path == NULL)
        cmd_path = find_command(current_cmd->arg[0], ctx->env->env_vars);
    if (!cmd_path) {
        ft_putstr_fd(current_cmd->arg[0], 2);
        ft_putstr_fd(": command not found\n", 2);
        exit(127);
    }
    execve(cmd_path, current_cmd->arg, ctx->env->env_vars);
    perror("execve");
    exit(1);
}

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
//=-=-=-=-

static void handle_parent_builtin(t_command_context1 *ctx, t_arg *current_cmd)
{
    if (apply_redirections(current_cmd->red) == -1)
    {
        restore_io(ctx->io);
        return;
    }
    execute_builtin_p(current_cmd, ctx->env, ctx->exit_status);
    restore_io(ctx->io);
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

        g_sig.pid = pids[cmd_index];
        current_cmd = current_cmd->next;
        cmd_index++;
    }
    return child_count;
}

static void cleanup_pipes(t_command_context1 *ctx)
{
    for (int i = 0; i < ctx->pipe_count; i++) {
        close(ctx->pipe_fds[i][0]);
        close(ctx->pipe_fds[i][1]);
    }
}

static void handle_parent_builtin(t_command_context1 *ctx, t_arg *current_cmd);
static int process_commands(t_command_context1 *ctx, pid_t *pids);
static void cleanup_pipes(t_command_context1 *ctx);

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

void cleanup_command_heredocs(t_arg *cmd, t_fd_tracker *fd_tracker) {
    int i;
    int heredoc_count;

    i = 0;
    heredoc_count = count_heredocs(cmd->red);
    while (i < heredoc_count) {
        if (cmd->heredoc_fds[i] > 2) {
            close(cmd->heredoc_fds[i]);
            untrack_fd(fd_tracker, cmd->heredoc_fds[i]);
        }
        i++;
    }
}

void cleanup_heredoc_fds(t_arg *cmd, t_fd_tracker *fd_tracker)
{
    t_arg *current_cmd;

    current_cmd = cmd;
    while (current_cmd)
    {
        if (current_cmd->heredoc_fds) {
            cleanup_command_heredocs(current_cmd, fd_tracker);
        }
        current_cmd = current_cmd->next;
    }
}

//=-=-=-=-=-=-=-

static void init_setup_context(t_setup_context *setup_ctx, t_command_context *context, 
                               t_execution_data *exec_data)
{
    setup_ctx->cmd = context->cmd;
    setup_ctx->env = context->env;
    setup_ctx->io = &exec_data->io;
    setup_ctx->pipe_fds = exec_data->pipe_fds;
    setup_ctx->command_count = &exec_data->command_count;
    setup_ctx->pipe_count = &exec_data->pipe_count;
}

static void init_command_context1(t_command_context1 *ctx, t_command_context *context, 
                                  t_execution_data *exec_data)
{
    ctx->cmd = context->cmd;
    ctx->env = context->env;
    ctx->exit_status = context->exit_status;
    ctx->io = &exec_data->io;
    ctx->pipe_fds = exec_data->pipe_fds;
    ctx->command_count = exec_data->command_count;
    ctx->pipe_count = exec_data->pipe_count;
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

static char *get_path_from_env(char **envp)
{
    int i;

    i = 0;
    while (envp[i])
    {
        if (ft_strncmp(envp[i], "PATH=", 5) == 0)
            return (envp[i] + 5);
        i++;
    }
    return (NULL);
}

static char *create_full_path(char *dir, char *cmd)
{
    char *full_path;

    full_path = malloc(ft_strlen(dir) + ft_strlen(cmd) + 2);
    if (!full_path)
        return (NULL);
    ft_strlcpy(full_path, dir, ft_strlen(dir) + 1);
    ft_strlcat(full_path, "/", ft_strlen(dir) + 2);
    ft_strlcat(full_path, cmd, ft_strlen(dir) + ft_strlen(cmd) + 2);
    return (full_path);
}

static char *search_command_in_path(char *path_copy, char *cmd)
{
    char *dir;
    char *full_path;

    dir = ft_strtok(path_copy, ":");
    while (dir)
    {
        full_path = create_full_path(dir, cmd);
        if (!full_path)
            return (NULL);
        if (access(full_path, X_OK) == 0)
            return (full_path);
        free(full_path);
        dir = ft_strtok(NULL, ":");
    }
    return (NULL);
}

char *find_command(char *cmd, char **envp)
{
    char *path;
    char *path_copy;
    char *full_path;

    path = get_path_from_env(envp);
    if (!path)
        return (NULL);
    path_copy = ft_strdup(path);
    if (!path_copy)
        return (NULL);
    full_path = search_command_in_path(path_copy, cmd);
    free(path_copy);
    return (full_path);
}

char	*ft_get_path(char *cmd, int *exit_status)
{
	char		*path;
	struct stat	statfile;

	if (stat(cmd, &statfile) == 0 && S_ISDIR(statfile.st_mode)
		&& ft_strchr(cmd, '/') != NULL)
	{
		*exit_status = 126;
		ft_putstr_fd(cmd, 2);
		ft_putstr_fd(": is a directory\n", 2);
		exit (*exit_status);
	}
	path = cmd;
	if (ft_strchr(cmd, '/') != NULL && access(cmd, X_OK) == -1)
	{
		*exit_status = 127;
		if (access(cmd, F_OK) == 0)
			*exit_status = 126;
		perror(cmd);
		exit (*exit_status);
	}
	if (ft_strchr(cmd, '/') != NULL && access(cmd, X_OK) == 0)
		return (path);
	return (NULL);
}
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
            return (exit_status);
        }
        close_all_fds(fd_tracker);
    }
    close_all_fds(fd_tracker);
    return (exit_status);
}
