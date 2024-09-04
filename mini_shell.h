#ifndef MINI_SHELL_H
# define MINI_SHELL_H

#define MAX_FD 1024  // Adjust this based on your system's limits
typedef struct s_fd_tracker {
    int fd_table[MAX_FD];
    int qout;
} t_fd_tracker;

# include "./include/struct.h"
# include "./include/libft.h"
# include "./include/exe_struct_hlp.h"


# include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <sys/stat.h>


#define MAX_PIPES 100
#define MAX_COMMANDS 101
#define MAX_ARGS 100
#define DELIMITERS " \t\r\n\a"
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define BUFFER_SIZE 1024

typedef struct s_setup_context {
    t_arg *cmd;
    t_env *env;
    t_io *io;
    int (*pipe_fds)[2];
    int *command_count;
    int *pipe_count;
} t_setup_context;

typedef struct s_command_context
{
    t_arg *cmd;
    t_env *env;
    int *exit_status;
    t_fd_tracker *fd_tracker;
} t_command_context;

typedef struct s_execution_data {
    t_io io;
    int command_count;
    int pipe_count;
    int pipe_fds[MAX_PIPES][2];
} t_execution_data;




t_sig g_sig;

// get_path
char	*ft_get_path(char *cmd, int *exit_status);

// Function prototypes
t_env *create_env_v_i(int count);
t_env *create_env(char **envp);
void free_env(t_env *env);
char *get_env_value(t_env *env, const char *key);
int set_env_value(t_env *env, const char *key, const char *value);
int unset_env_value(t_env *env, const char *key);

// int execute_builtin(t_arg *cmd, t_env *env, int *exit_status);///////////
int execute_builtin_p(t_arg *cmd, t_env *env, int *exit_status);
int execute_builtin_ch(t_arg *cmd, t_env *env, int *exit_status);
int execute_external_command(char **argv, char **envp);

int execute_command(t_command_context *context);
// int execute_command(t_arg *cmd, t_env *env, int *exit_status);

// int main_shell_loop(t_env *env);
int ft_env(t_env *env);

int ft_change_dir(char **av, t_env *env, int *exit_status);
int change_to_home(t_env *env);
int change_to_previous(t_env *env);
                    /*PIP - HELPER FUNCTIONS*/

char **split_pipeline(char *line);
void redirect_io(int input_fd, int output_fd);
int run_pipeline_command(char *command, t_env *env, int input_fd, int output_fd);
int execute_builtin_command(char **argv, t_env *env, int input_fd, int output_fd);

int execute_external_command_fork(char **argv, t_env *env, int input_fd, int output_fd);
// void setup_child_process(t_arg *cmd, int cmd_index, int pipe_count, int pipe_fds[MAX_PIPES][2]);

void handle_command(char *command, t_pipeline_state *state);
int wait_and_cleanup(int *pids, int num_commands, int prev_input, int temp_stdout);
int handle_pipeline(char **commands, t_env *env);

void    free_argv(char **av);
                    /*UNSET - HELPER FUNCTIONS*/


                    /*EXPORT - HELPER FUNCTIONS*/

int     check_format(char *arg);
int     print_error(int error_code, char *content);
void    print_sorted_env(t_env *env);
int     parse_export_string(char *s, char **name, char **value, int *is_append);

                    /*exit BUiLTIN*/

int     ft_is_arg_nbr(char *ar);
                    /*THE BUILTIN*/

int		ft_echo(char **av);
int ft_exit(char **av, int *exit_status);
int     pwd(int fd);
int ft_exports(t_env *env, char **args, int *exit_status);
int ft_unsets(t_env *env, char **args, int *exit_status);

int append_new_var(t_env *env, char *s);
                    /*THE_EXECUTOR*/

int main_shell_loop(t_env *env, t_fd_tracker *fd_tracker);
// int     main_shell_loop(t_env *env);
int     is_builtin(const char *cmd);
char    **split_line(char *line);
char *find_command(char *cmd, char **envp);
// char *find_command(char *cmd, char **envp, int *exit_status);
int     execute_external_command(char **argv, char **envp);
// void execute_external_command(t_arg *cmd, t_env *env);
void    restore_io(t_io *io);
void    save_original_io(t_io *io);
// int setup_pipes(int pipe_count, int pipe_fds[][2]);

int setup_pipes(int pipe_count, int pipe_fds[][2], t_fd_tracker *fd_tracker);/////

// void setup_child_process(child_setup_params *params);

// int fork_and_execute(t_arg *cmd, t_env *env, int *exit_status, int cmd_index, int pipe_count, int pipe_fds[][2], int *heredoc_fds, pid_t *pid);
int fork_and_execute(fork_execute_params *params);
// int execute_builtin_command(t_arg *current_cmd, t_env *env, int *exit_status, t_io *io);
// void setup_child_process(int cmd_index, int pipe_count, int pipe_fds[][2], int *heredoc_fds, int heredoc_count);
// void setup_child_process(int cmd_index, int pipe_count, int pipe_fds[MAX_PIPES][2], t_arg *current_cmd);
void setup_child_process(child_setup_params *params);

// void setup_child_process(t_arg *cmd, int cmd_index, int pipe_count, int pipe_fds[MAX_PIPES][2]);
void cleanup_parent_process(int *heredoc_fds, int heredoc_count);
int count_commands(t_arg *cmd);
int wait_for_children(pid_t *pids, int command_count, int *exit_status);
void    free_tokens(t_token *tokens);
void    free_command(t_arg *cmd);


/*free the list*/
// void free_command_list(t_arg *cmd_list);

/*parsing*/
// //
int	ft_var_update(int i, char **var, char *to_update, char	*secend_part);
int 	is_valid_var(char c);
t_token *ft_list_new(char *token, int z);
char	*ft_compress_spaces(char *s);
int		ft_check_qoutes(char	*line);
int	ft_skipe_qoute(char	*s, int i);
int	ft_name_len(char *var, int i);
int ft_have_sp_tb(char *s);
int	ft_handl_spichel_cond(t_token **token, t_token *now, t_token *next_token,
 t_type *prv_type);
int	get_token(t_token **token, char	*s, int z);
void free_tokens(t_token *tokens);
int		ft_check_error(t_token *token);
void	ft_putstr_fd(char *str, int fd);
char	*ft_remove_quotes(char *s);
// int		expanden(t_token **token, char **env);
int	expanding(t_token **token, t_env *env, int exit_status, t_type prv_type);
int	expanding_helper(char *s, int i);
// int	get_token(t_token **token, char	*s);
int		ft_skipe_spaces(char *s, int index);
char    *ft_remove_char(char *s, unsigned int index);
void	ft_lstadd_back(t_token **lst, t_token *new);
int	check_induble(char *s, int i);
int	is_spc_opr(char *s, int i);


// t_arg *parsing(char *line, char **env);
int	parsing(char *line, t_token	**token, t_env *env, int exit_status);
t_arg	*ft_arg_new(int fd);
void	ft_argadd_back(t_arg **lst, t_arg *new);
t_arg	*ft_arglast(t_arg	*lst);

// int is_pipe_token(t_arg *arg);
// t_arg *get_next_arg(t_arg *arg);
// char *get_arg_content(t_arg *arg);
// void free_arg_content(t_arg *arg);
// t_arg *convert_tokens_to_args(t_token *tokens);
int ft_convert_token_to_arg(t_token *token, t_arg *args, int stat);

/*Redirections functions*/

int apply_redirections(char **red);

/*HERDOC*/

int count_heredocs(char **red);

// int *handle_heredocs(char **red, int count, t_env *env);
int *handle_heredocs(char **red, int count, t_env *env, t_fd_tracker *tracker);

char *read_line(void);
int expand_variable(char **line, t_env *env);
int write_to_pipe(int pipefd[2], char *line);

int ft_expand_variable(t_token *t, t_env *env, t_type prv_type, int i);//expending
char	*get_var_from_env(char *var, int var_len, t_env *env);
int	ft_var_update(int i, char **var, char *to_update, char	*secend_part);
int	ft_expand_herdoc_var(char **var, t_env *env, t_type prv_type, int i);

/*exit_status*/


// int	ft_expand_exit_status(char **var, int exit_status);
int	ft_expand_exit_status(char **var, int exit_status, int i);



/* SIGNALS*/
void sig_int(int code);
void sig_init(void);
void sig_quit(int code);

void track_fd(t_fd_tracker *tracker, int fd);
void untrack_fd(t_fd_tracker *tracker, int fd);
void close_all_fds(t_fd_tracker *tracker);

/*heredoc helpers*/

void child_process(int pipefd[2], const char *delimiter, t_env *env, t_fd_tracker *fd_tracker);
int parent_process(int pipefd[2], pid_t pid, t_fd_tracker *fd_tracker);
int check_if_qoutes(char *s);

#endif