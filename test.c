//signals

void sig_int(int code)
{
    (void)code;
    if (g_sig.in_heredoc)
    {
        write(STDERR_FILENO, "\n", 1);
        g_sig.sigint = 1;
        g_sig.exit_status = 1;
    }
    else if (g_sig.pid == 0)
    {
        write(STDERR_FILENO, "\n", 1);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
        g_sig.sigint = 1;
        g_sig.exit_status = 1;
    }
    else
    {
        write(STDERR_FILENO, "\n", 1);
    }
}

// void sig_int(int code)
// {
//     (void)code;
//     if (g_sig.pid == 0)
//     {
//         write(STDERR_FILENO, "\n", 1);
//         rl_on_new_line();
//         rl_replace_line("", 0);
//         rl_redisplay();
//     }
//     else
//     {
//         write(STDERR_FILENO, "\n", 1);
//     }
//     g_sig.sigint = 1;
//     g_sig.exit_status = 1;
//     // g_sig.exit_status = 130;
// }

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

//heredoc funstions

static char *get_heredoc_delimiter(char **red, int index)
{
    int count = 0;
    int i = 0;
    while (red && red[i])
    {
        if (ft_strcmp(red[i], "<<") == 0)
        {
            if (count == index && red[i + 1])
                return ft_strdup(red[i + 1]);
            count++;
        }
        i++;
    }
    return NULL;
}

int process_delimiter(char **red, int i, char **delimiter, char **processed_delimiter, t_fd_tracker *tracker)
{
    int j;

    j = 0;
    *delimiter = get_heredoc_delimiter(red, i);
    if (!*delimiter)
        return 0;
    while (delimiter[0][j])
    {
        if (delimiter[0][j] == '$' && (delimiter[0][j + 1] == '"'
			|| delimiter[0][j + 1] == '\''))
		{
            delimiter[0] = ft_remove_char(delimiter[0], j);
            if (!delimiter[0])
                return (0);
        }
        j++;
    }
    tracker->qout = check_if_qoutes(*delimiter);
    if (tracker->qout == 1)
        *processed_delimiter = ft_remove_quotes(*delimiter);
    else
        *processed_delimiter = *delimiter;
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
    g_sig.in_heredoc = 1;/////
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        untrack_fd(tracker, pipefd[0]);
        untrack_fd(tracker, pipefd[1]);
        g_sig.in_heredoc = 0;  // Reset the flag
        return -1;
    }
    if (pid == 0) { // Child process
        signal(SIGINT, SIG_DFL);
        child_process(pipefd, delimiter, env, tracker);
        exit(0);
    } else { // Parent process
        parent_process(pipefd, pid, tracker);
        g_sig.in_heredoc = 0;  // Reset the flag
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
void child_process(int pipefd[2], const char *delimiter, t_env *env, t_fd_tracker *fd_tracker)
{
    char *line;

    close(pipefd[0]); // Close read end of the pipe
    untrack_fd(fd_tracker, pipefd[0]);
    
    while ((line = read_line()) != NULL)
    {        if (ft_strcmp(line, delimiter) == 0)
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

int parent_process(int pipefd[2], pid_t pid, t_fd_tracker *fd_tracker) {
    (void)pid;
    int status;
    close(pipefd[1]); // Close write end of the pipe
    untrack_fd(fd_tracker, pipefd[1]);
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
        return(1);
    return(0);
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

char *read_line(void)
{
    char *line = NULL;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int total_size = 0;

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        line = append_buffer(line, buffer, bytes_read, &total_size);
        if (!line)
            return NULL;
        if (line[total_size - 1] == '\n') {
            line[total_size - 1] = '\0';
            return line;
        }
    }

    if (bytes_read == -1 || (bytes_read == 0 && total_size == 0)) {
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

//execution::

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
        if (g_sig.sigint)
        {
            g_sig.sigint = 0;
            exit_status = g_sig.exit_status;
        }
        input = readline(GREEN"minishell> "RESET);
        if (input == NULL)
        {
            write(STDERR_FILENO, "exit\n", 6);
            break;
        }
        if (process_input(input, env, &tokens, &exit_status) == -1)
        {
            close_all_fds(fd_tracker);
        }
        close_all_fds(fd_tracker);
    }
    close_all_fds(fd_tracker);
    return (exit_status);
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

//=-=-=-=-

// typedef struct s_setup_context {
//     t_arg *cmd;
//     t_env *env;
//     t_io *io;
//     int (*pipe_fds)[2];
//     int *command_count;
//     int *pipe_count;
// } t_setup_context;


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
        if (cmd->heredoc_fds[i] > 2)
        {
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
//structs

typedef struct x_arg
{
	char  **arg;
	char  **red;
	int     *heredoc_fds;
	struct x_arg *next;
} t_arg;

typedef enum s_type
{
	WORD,
	PIPE,
	RED_IN,
	RED_OUT,
	HER_DOC,
	APPEND,
}	t_type;

typedef struct s_tokenz
{
	char			*content;
	int				type;
	bool			qout_rm;
	bool			is_empty;
	int				j;
	struct s_tokenz	*next;
}	t_token;

// Add this to your header file (mini_shell.h)
typedef struct s_env
{
	char    **env_vars;
	int     count;
} t_env;

typedef struct s_io
{
	int original_stdin;
	int original_stdout;
} t_io;

typedef struct s_redir
{
	int fd;
	int original_fd;
	int is_output;
	int type;
} t_redir;

typedef struct s_pipeline_state //for pip
{
	int prev_input;
	int pipe_fds[2];
	int *pids;
	int num_commands;
	t_env *env;
}	t_pipeline_state;

typedef struct s_sig
{
	int		sigint;
	int		sigquit;
	int		exit_status;
	pid_t	pid;
	int in_heredoc;///
}	t_sig;

t_sig	g_sig;