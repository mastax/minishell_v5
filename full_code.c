//builtins

        //export=-=-=-=-=-=

// Function to handle appending a new value to an existing environment variable
static int handle_existing_var_append(t_env *env, char *name, char *value, int i)
{
    char *old_value;
    char *new_value;

    // Find the current value of the variable after the '=' sign
    old_value = ft_strchr(env->env_vars[i], '=');
    if (old_value)
    {
        old_value++;  // Move past the '=' to get the actual value
        // Concatenate the old value with the new value
        new_value = ft_concat(old_value, value);
        free(env->env_vars[i]);  // Free the old variable
        // Recreate the environment variable with the updated value
        env->env_vars[i] = ft_concat(name, "=");
        env->env_vars[i] = ft_concat(env->env_vars[i], new_value);
        free(new_value);  // Free the temporary concatenated value
    }
    else
    {
        free(env->env_vars[i]);  // Free the old variable
        // If there was no old value, just set it to the new value
        env->env_vars[i] = ft_concat(name, "=");
        env->env_vars[i] = ft_concat(env->env_vars[i], value);
    }
    return 1;  // Indicate that the variable was handled successfully
}

// Function to handle updating an existing environment variable
static int handle_existing_var(t_env *env, char *name, char *value, int is_append)
{
    int i = 0;

    // Loop through environment variables to find the existing variable
    while (env->env_vars[i] && ft_strncmp(env->env_vars[i], name, ft_strlen(name)))
        i++;
    if (!env->env_vars[i])
        return 0;  // If the variable is not found, return 0
    if (!value)
        return 1;  // If there's no value to set, do nothing and return 1
// Append to the existing variable if needed
    if (is_append)
        return handle_existing_var_append(env, name, value, i);
    free(env->env_vars[i]);  // Free the old variable
    // Set the environment variable to the new value
    env->env_vars[i] = ft_concat(name, "=");
    env->env_vars[i] = ft_concat(env->env_vars[i], value);
    return 1;  // Indicate that the variable was handled successfully
}

// Function to find and update an environment variable if it exists
static int find_and_update_var(t_env *env, char *name, char *value, int is_append)
{
    int i = 0;
    size_t name_len = ft_strlen(name);

    // Loop through all environment variables
    while (i < env->count)
    {
        // Check if the current variable matches the name
        if (ft_strncmp(env->env_vars[i], name, name_len) == 0 && 
            (env->env_vars[i][name_len] == '=' || env->env_vars[i][name_len] == '\0'))
        {
            return handle_existing_var(env, name, value, is_append);  // Update the existing variable
        }
        i++;
    }
    return 0;  // Return 0 if the variable is not found
}

// Main function for handling the export command
int ft_export(t_env *env, char *s)
{
    char *name;
    char *value;
    int is_append;

    is_append = 0;  // Initialize append flag to 0
    if (check_format(s) == 1)
        return 2;  // Return error if the format is invalid
    parse_export_string(s, &name, &value, &is_append);  // Parse the export string into name and value
    if (!find_and_update_var(env, name, value, is_append))
    {
        if (!append_new_var(env, s))  // If the variable doesn't exist, append it as a new variable
        {
            free(name);  // Free the name if appending fails
            return 0;  // Return 0 on failure
        }
    }
    free(name);  // Free the name after processing
    return 1;  // Return 1 on success
}

// Function for processing multiple export commands
int ft_exports(t_env *env, char **args, int *exit_status)
{
    int i;
    int command_return;

    *exit_status = 0;  // Initialize exit status to 0
    if (!args[1] || (args[1][0] == '\0' && !args[2]))//there is  no argunments provided or the first char of the arg is null terminator that mean is empty
    {
        print_sorted_env(env);  // If no arguments, print the sorted environment variables
        return 1;  // Return 1 on success
    }
    i = 1;
    if (args[1][0] == '\0')
        i = 2;  // Skip empty argument if present
    while (args[i])
    {
        command_return = ft_export(env, args[i]);  // Process each export command
        if (command_return == 0)
            return 0;  // Return 0 if export fails
        else if (command_return == 2)
        {
            *exit_status = 1;  // Set exit status to 1 on format error
        }
        i++;
    }
    return 1;  // Return 1 on success
}

// Function to copy environment variables into a new array
static char **copy_env_vars(t_env *env)
{
    char **sorted_env;
    int i;

    sorted_env = malloc(sizeof(char *) * (env->count + 1));  // Allocate memory for the new array
    if (!sorted_env)
        return NULL;  // Return NULL on allocation failure
    i = 0;
    while (i < env->count)
    {
        sorted_env[i] = ft_strdup(env->env_vars[i]);  // Copy each environment variable
        i++;
    }
    sorted_env[i] = NULL;  // Null-terminate the array
    return sorted_env;  // Return the copied array
}

// Function to sort environment variables alphabetically
static void sort_env_vars(char **sorted_env, int count)
{
    int i;
    int j;
    char *temp;

    i = 0;
    while (i < count - 1)
    {
        j = 0;
        while (j < count - i - 1)
        {
            // Compare adjacent variables and swap them if they are out of order
            if (ft_strcmp(sorted_env[j], sorted_env[j + 1]) > 0)
            {
                temp = sorted_env[j];
                sorted_env[j] = sorted_env[j + 1];
                sorted_env[j + 1] = temp;
            }
            j++;
        }
        i++;
    }
}

// Function to print environment variables in the required format
static void print_env_vars(char **sorted_env, int count)
{
    int i;
    char *name;
    char *value;

    i = 0;
    while (i < count)
    {
        ft_putstr_fd("declare -x ", 1);  // Print the prefix "declare -x"
        name = sorted_env[i];
        value = ft_strchr(sorted_env[i], '=');  // Find the '=' in the variable
        if (value)
        {
            *value = '\0';  // Temporarily terminate the name string
            ft_putstr_fd(name, 1);  // Print the variable name
            ft_putstr_fd("=\"", 1);  // Print the equals sign and opening quote
            ft_putstr_fd(value + 1, 1);  // Print the variable value
            ft_putendl_fd("\"", 1);  // Print the closing quote and newline
            *value = '=';  // Restore the '=' character
        }
        else
            ft_putendl_fd(sorted_env[i], 1);  // Print the variable if there's no value
        i++;
    }
}

// Function to free the memory allocated for the sorted environment variables
static void free_sorted_env(char **sorted_env, int count)
{
    int i;

    i = 0;
    while (i < count)
    {
        free(sorted_env[i]);  // Free each string in the array
        i++;
    }
    free(sorted_env);  // Free the array itself
}

// Function to print the sorted environment variables
void print_sorted_env(t_env *env)
{
    char **sorted_env;

    sorted_env = copy_env_vars(env);  // Copy the environment variables
    if (!sorted_env)
        return;  // Return if copying fails
    sort_env_vars(sorted_env, env->count);  // Sort the copied variables
    print_env_vars(sorted_env, env->count);  // Print the sorted variables
    free_sorted_env(sorted_env, env->count);  // Free the memory
}

// Function to check the format of the export argument
int check_format(char *arg)
{
    int cur;
    int has_equal;

    // Check if the first character is a valid identifier character
    if (!ft_isalpha(arg[0]) && arg[0] != '_')
        return (print_error(1, arg), 1);  // Return error if invalid
    cur = 0;
    while (arg[cur] != '\0')
    {
        if (arg[cur] == '=')
            break;  // Stop if '=' is found
        if (arg[cur] == '-')
            return (print_error(1, arg), 1);  // Return error if '-' is found
        cur++;
    }
    has_equal = (arg[cur] == '=') ? 1 : 0;  // Check if '=' was found
    while (arg[cur] != '\0')
    {
        if (!ft_isalnum(arg[cur]) && arg[cur] != '_')
            return (print_error(2, arg), 1);  // Return error if invalid character is found
        cur++;
    }
    return 0;  // Return 0 if the format is valid
}

// Function to print error messages for invalid export arguments
void print_error(int error_type, char *arg)
{
    if (error_type == 1)
        ft_putstr_fd("minishell: export: not a valid identifier: ", 2);
    else if (error_type == 2)
        ft_putstr_fd("minishell: export: not a valid value: ", 2);
    ft_putendl_fd(arg, 2);  // Print the invalid argument
}

// Function to append a new environment variable
int append_new_var(t_env *env, char *str)
{
    char **new_env_vars;

    new_env_vars = malloc(sizeof(char *) * (env->count + 2));  // Allocate space for the new array
    if (!new_env_vars)
        return 0;  // Return 0 on allocation failure
    int i = 0;
    while (i < env->count)
    {
        new_env_vars[i] = env->env_vars[i];  // Copy existing environment variables
        i++;
    }
    new_env_vars[i] = ft_strdup(str);  // Add the new variable
    new_env_vars[++i] = NULL;  // Null-terminate the array
    free(env->env_vars);  // Free the old array
    env->env_vars = new_env_vars;  // Update the environment with the new array
    env->count++;  // Increase the environment variable count
    return 1;  // Return 1 on success
}

// Function to parse the export string into name and value
int parse_export_string(char *s, char **name, char **value, int *is_append)
{
    *name = ft_strdup(s);  // Duplicate the string 's' and assign it to 'name'
    *value = ft_strchr(*name, '=');  // Find the first occurrence of '=' in 'name'
    
    if (*value)  // If '=' is found
    {
        **value = '\0';  // Replace '=' with '\0' to separate 'name' from 'value'
        (*value)++;  // Move 'value' to point to the start of the actual value after '='
        
        if (*(*value - 2) == '+')  // Check if the character before '=' is '+', indicating append mode
        {
            *is_append = 1;  // Set the append flag to true
            *(*value - 2) = '\0';  // Remove the '+' by replacing it with '\0'
        }
    }
    else
        *value = NULL;  // If '=' is not found, set 'value' to NULL
    
    return (0);  // Return 0 indicating successful parsing
}

//=-=-=-=-=-=-UNSET:

int unset_env_value(t_env *env, const char *key)
{
    int i; // Variable to iterate through the environment variables
    int j; // Variable to handle the shifting of environment variables
    size_t key_len; // Length of the key to search for

    i = 0; // Initialize the loop counter
    j = 0; // Initialize the shift counter
    key_len = ft_strlen(key); // Calculate the length of the key

    // Loop through each environment variable
    while (i < env->count)
    {
        // Check if the current variable starts with the key and is followed by '='
        if (ft_strncmp(env->env_vars[i], key, key_len) == 0 && env->env_vars[i][key_len] == '=')
        {
            free(env->env_vars[i]); // Free the memory for the matching environment variable

            j = i; // Set j to the current position
            // Shift all subsequent environment variables up by one position
            while (j < env->count - 1)
            {
                env->env_vars[j] = env->env_vars[j + 1]; // Move the next variable up
                j++; // Move to the next position
            }

            env->count--; // Decrease the environment variable count
            env->env_vars[env->count] = NULL; // Set the last position to NULL

            return 1; // Return 1 to indicate success
        }
        i++; // Move to the next environment variable
    }

    return 0; // Return 0 if the key was not found
}
int ft_unsets(t_env *env, char **args, int *exit_status)
{
    int i; // Variable to iterate through the arguments
    int ret; // Variable to track the result of the unset operation

    i = 0; // Initialize the loop counter
    ret = 0; // Initialize the result tracker

    // First loop: check the format of each argument
    while (args[++i]) // Start from args[1] and loop through all arguments
    {
        if (check_format(args[i]) == 1) // Check if the argument has an invalid format
            ret = 1; // If invalid, set ret to 1
    }

    i = 0; // Reset the loop counter

    // Second loop: attempt to unset each environment variable
    while (args[++i]) // Again, start from args[1] and loop through all arguments
    {
        if (unset_env_value(env, args[i]) == 1) // Try to unset the environment variable
            ret = 0; // If successful, reset ret to 0 (indicating no errors)
    }

    *exit_status = ret; // Set the exit status to the value of ret
    return 1; // Return 1 to indicate the command was executed
}


//=-=-=-=-=-=-

//=-=-=-=-==--=-CD

int ft_change_dir(char **av, t_env *env, int *exit_status)
{
    char current_dir[PATH_MAX];  // Buffer to store the current directory path
    char new_dir[PATH_MAX];      // Buffer to store the new directory path after the change

    // Try to get the current working directory and store it in current_dir
    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        // If getcwd fails, print an error message to standard error
        ft_putstr_fd("cd: error retrieving current directory: getcwd:", 2);
        ft_putstr_fd(" access parent directories: No such file or directory\n", 2);
    }

    // If no directory is provided in av[1], change to the home directory
    if (!av[1])
        *exit_status = change_to_home(env);
    // If av[1] is "-", change to the previous directory
    else if (ft_strcmp(av[1], "-") == 0)
        *exit_status = change_to_previous(env);
    // Otherwise, try to change to the directory specified in av[1]
    else
        *exit_status = chdir(av[1]);

    // If the directory change failed (chdir returned -1)
    if (*exit_status == -1)
    {
        // Print an error message using perror and set exit_status to 1
        perror("cd");
        return (*exit_status = 1);
    }

    // Attempt to get the new current directory after the change
    if (getcwd(new_dir, sizeof(new_dir)) == NULL)
        return (1);  // Return 1 if retrieving the new directory fails

    // Update the environment variable OLDPWD with the old directory path
    set_env_value(env, "OLDPWD", current_dir);
    // Update the environment variable PWD with the new directory path
    set_env_value(env, "PWD", new_dir);

    return (0);  // Return 0 to indicate success
}


char *get_env_value(t_env *env, const char *key) // helper function for getting environment variables
{
    int key_len;  // Variable to store the length of the key
    int i;        // Loop index

    key_len = ft_strlen(key);  // Get the length of the key
    i = 0;

    // Loop through all environment variables
    while (i < env->count)
    {
        // Check if the current environment variable matches the key
        if (ft_strncmp(env->env_vars[i], key, key_len) == 0 && env->env_vars[i][key_len] == '=')
            // Return the value part of the environment variable (after '=')
            return env->env_vars[i] + key_len + 1;
        i++;
    }

    // If the key was not found, return NULL
    return NULL;
}

int change_to_home(t_env *env)
{
    char *home;  // Pointer to hold the value of the HOME environment variable

    home = get_env_value(env, "HOME");  // Get the value of the HOME environment variable

    // If HOME is not set, print an error message and return -1
    if (!home)
    {
        ft_putstr_fd("cd: HOME not set\n", STDERR_FILENO);
        return (-1);
    }

    // Change to the directory specified by HOME and return the result of chdir
    return (chdir(home));
}

int change_to_previous(t_env *env)
{
    char *oldpwd;  // Pointer to hold the value of the OLDPWD environment variable

    oldpwd = get_env_value(env, "OLDPWD");  // Get the value of the OLDPWD environment variable

    // If OLDPWD is not set, print an error message and return -1
    if (!oldpwd)
    {
        ft_putstr_fd("cd: OLDPWD not set\n", STDERR_FILENO);
        return (-1);
    }

    // Print the value of OLDPWD to standard output
    ft_putendl_fd(oldpwd, STDOUT_FILENO);
    // Change to the directory specified by OLDPWD and return the result of chdir
    return chdir(oldpwd);
}

static char *create_new_var(const char *key, const char *value)
{
    size_t key_len;   // Variable to store the length of the key
    size_t value_len; // Variable to store the length of the value
    char *new_var;    // Pointer to hold the newly created key=value string

    key_len = ft_strlen(key);    // Get the length of the key
    value_len = ft_strlen(value); // Get the length of the value

    // Allocate memory for the new environment variable string
    // It needs space for key, '=', value, and the null terminator
    new_var = malloc(key_len + value_len + 2);
    if (!new_var)
        return (NULL);  // Return NULL if memory allocation fails

    // Copy the key into new_var
    ft_strcpy(new_var, key);
    // Add the '=' character after the key
    new_var[key_len] = '=';
    // Copy the value into new_var after the '='
    ft_strcpy(new_var + key_len + 1, value);

    return (new_var);  // Return the newly created key=value string
}

static int update_existing_env(t_env *env, const char *key, char *new_var)
{
    int i;           // Loop index
    size_t key_len;  // Variable to store the length of the key

    key_len = ft_strlen(key);  // Get the length of the key
    i = 0;

    // Loop through all environment variables
    while (i < env->count)
    {
        // Check if the current environment variable matches the key
        // and if the key is followed by '=' in the current variable
        if (ft_strncmp(env->env_vars[i], key, key_len) == 0 && 
            env->env_vars[i][key_len] == '=')
        {
            // Free the old value
            free(env->env_vars[i]);
            // Update the environment variable with the new key=value string
            env->env_vars[i] = new_var;
            return 1;  // Return 1 to indicate the update was successful
        }
        i++;
    }

    return 0;  // Return 0 if the key was not found in the environment
}

static int add_new_env(t_env *env, char *new_var)
{
    int i;           // Loop index
    char **new_env;  // Pointer to the new environment variable array

    // Allocate memory for the new environment array
    // It needs space for the current count + 1 new variable + 1 for the NULL terminator
    new_env = malloc(sizeof(char*) * (env->count + 2));
    if (!new_env)
    {
        // If memory allocation fails, free the new_var and return 0
        free(new_var);
        return (0);
    }

    // Copy all existing environment variables to the new environment array
    i = 0;
    while (i < env->count)
    {
        new_env[i] = env->env_vars[i];
        i++;
    }

    // Add the new variable to the end of the new environment array
    new_env[env->count] = new_var;
    // Set the last element to NULL to indicate the end of the array
    new_env[env->count + 1] = NULL;

    // Free the old environment array
    free(env->env_vars);
    // Point env->env_vars to the new environment array
    env->env_vars = new_env;
    // Increment the count of environment variables
    env->count++;

    return 1;  // Return 1 to indicate success
}

int set_env_value(t_env *env, const char *key, const char *value)
{
    char *new_var;  // Pointer to hold the new key=value string

    // Create a new key=value string
    new_var = create_new_var(key, value);
    if (!new_var)
        return 0;  // Return 0 if memory allocation fails

    // Try to update an existing environment variable
    if (update_existing_env(env, key, new_var))
        return 1;  // Return 1 if the update was successful

    // If the variable does not exist, add it as a new environment variable
    return (add_new_env(env, new_var));
}
//=-=-=-=-=-=-=-=-=-=--=-==-
//REDIRECTION
//=-=-=-=-=-=-=-=-=-=-=--=-=

static int handle_output_redirection(const char *filename, int flags)
{
    // Open the file with the provided flags (e.g., O_WRONLY, O_CREAT, etc.)
    int fd = open(filename, flags, 0644);
    
    // If the file can't be opened, print an error and return -1
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    // Redirect the standard output (STDOUT_FILENO) to the opened file descriptor
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        // If dup2 fails, close the file and return -1
        close(fd);
        return -1;
    }

    // Close the file as it is no longer needed
    close(fd);
    
    // Return 0 to indicate success
    return 0;
}

static int handle_input_redirection(const char *filename)
{
    // Open the file in read-only mode
    int fd = open(filename, O_RDONLY);
    
    // If the file can't be opened, print an error and return -1
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    // Redirect the standard input (STDIN_FILENO) to the opened file descriptor
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("dup2");
        // If dup2 fails, close the file and return -1
        close(fd);
        return -1;
    }

    // Close the file as it is no longer needed
    close(fd);
    
    // Return 0 to indicate success
    return 0;
}

static int process_redirection(char **red, int *i)
{
    const char *error_msg;

    // Check if the next element after the redirection symbol is a valid filename
    if (!red[*i + 1])
    {
        // Print an error message if the filename is missing
        error_msg = "Syntax error: missing filename after redirection\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return -1;
    }

    // If the redirection symbol is ">", handle it as output redirection (overwrite)
    if (ft_strcmp(red[*i], ">") == 0)
        return handle_output_redirection(red[*i + 1], O_WRONLY | O_CREAT | O_TRUNC);
    
    // If the redirection symbol is ">>", handle it as output redirection (append)
    else if (ft_strcmp(red[*i], ">>") == 0)
        return handle_output_redirection(red[*i + 1], O_WRONLY | O_CREAT | O_APPEND);
    
    // If the redirection symbol is "<", handle it as input redirection
    else if (ft_strcmp(red[*i], "<") == 0)
        return handle_input_redirection(red[*i + 1]);
    
    // Return -1 if the redirection symbol is not recognized
    return -1;
}

int apply_redirections(char **red)
{
    int i = 0;

    // If there are no redirections, return 0 (nothing to do)
    if (!red || !red[0])
        return 0;

    // Loop through all elements in the red array
    while (red[i])
    {
        // Check if the current element is a redirection symbol (">", ">>", "<")
        if (ft_strcmp(red[i], ">") == 0 || ft_strcmp(red[i], ">>") == 0 || 
            ft_strcmp(red[i], "<") == 0)
        {
            // Process the redirection and check for errors
            if (process_redirection(red, &i) == -1)
                return -1;
            // Skip the redirection symbol and filename
            i += 2;
        }
        else
            // Move to the next element if it's not a redirection symbol
            i++;
    }
    
    // Return 0 to indicate all redirections were successfully applied
    return 0;
}

//=-=-=-=-=-=-=-=-=-=--=-==-
//HEREDOC
//=-=-=-=-=-=-=-=-=-=-=--=-=

// Get the delimiter for the nth heredoc in the redirection array
static char *get_heredoc_delimiter(char **red, int index)
{
    int count = 0; // Keeps track of how many heredocs (<<) we've found
    int i = 0;     // Index to iterate through the red array

    // Loop through the redirection array until the end
    while (red && red[i])
    {
        // If we find the heredoc operator (<<)
        if (ft_strcmp(red[i], "<<") == 0)
        {
            // If it's the nth heredoc we're looking for and the next element is not NULL
            if (count == index && red[i + 1])
                return (ft_strdup(red[i + 1])); // Return a duplicate of the delimiter
            count++; // Increment the count of heredocs found
        }
        i++; // Move to the next element in the array
    }
    return NULL; // Return NULL if no delimiter is found
}

// Process the delimiter for a heredoc, removing special characters and quotes if necessary
int process_delimiter(char **red, int i, char **delimiter, char **processed_delimiter, t_fd_tracker *tracker)
{
    int j;

    j = 0;
    // Get the ith heredoc delimiter from the redirection array
    *delimiter = get_heredoc_delimiter(red, i);
    if (!*delimiter) // If no delimiter is found, return 0 (failure)
        return 0;

    // Loop through the characters in the delimiter
    while (delimiter[0][j])
    {
        // If we find a dollar sign followed by either a double quote or single quote
        if (delimiter[0][j] == '$' && (delimiter[0][j + 1] == '"'
			|| delimiter[0][j + 1] == '\''))
		{
            // Remove the dollar sign from the delimiter
            delimiter[0] = ft_remove_char(delimiter[0], j);
            if (!delimiter[0]) // If removal failed, return 0 (failure)
                return (0);
        }
        j++; // Move to the next character in the delimiter
    }

    // Check if the delimiter has quotes around it (single or double)
    tracker->qout = check_if_qoutes(*delimiter);

    // If quotes are found, remove them
    if (tracker->qout == 1)
        *processed_delimiter = ft_remove_quotes(*delimiter);
    else
        *processed_delimiter = *delimiter; // If no quotes, the delimiter remains unchanged

    // Return 1 if the processed delimiter is not NULL (success)
    return *processed_delimiter != NULL;
}

// Setup a pipe for process communication
int setup_pipe(int pipefd[2], t_fd_tracker *tracker)
{
    // Create a pipe with two file descriptors (pipefd[0] for reading, pipefd[1] for writing)
    if (pipe(pipefd) == -1)
    {
        perror("pipe"); // Print an error if pipe creation fails
        return -1; // Return -1 to indicate failure
    }

    // Track the file descriptors (for proper closing and memory management)
    track_fd(tracker, pipefd[0]);
    track_fd(tracker, pipefd[1]);
    return 0; // Return 0 on success
}

// Handle an error during fork, cleaning up pipe file descriptors
int handle_fork_error(int pipefd[2], t_fd_tracker *tracker)
{
    perror("fork"); // Print an error message for fork failure
    close(pipefd[0]); // Close the reading end of the pipe
    close(pipefd[1]); // Close the writing end of the pipe
    untrack_fd(tracker, pipefd[0]); // Untrack the reading end of the pipe
    untrack_fd(tracker, pipefd[1]); // Untrack the writing end of the pipe
    get_in_heredoc(0);  // Reset the heredoc flag (indicating heredoc process has ended)
    return -1; // Return -1 to indicate failure
}

// Create the heredoc by forking a child process to handle input
int create_heredoc(const char *delimiter, t_env *env, t_fd_tracker *tracker)
{
    int pipefd[2]; // File descriptors for the pipe
    pid_t pid;     // Process ID for fork

    // Setup a pipe for communication between processes
    if (setup_pipe(pipefd, tracker) == -1)
        return -1; // If pipe creation fails, return -1

    get_in_heredoc(1); // Set heredoc flag to indicate we're in a heredoc

    pid = fork(); // Create a new process (child process) with fork
    if (pid == -1) // If fork fails
        return handle_fork_error(pipefd, tracker); // Handle fork error

    if (pid == 0) // Child process
    {
        signal(SIGINT, SIG_DFL); // Set signal handler for Ctrl-C to default behavior
        child_process(pipefd, delimiter, env, tracker); // Handle heredoc input in the child process
        exit(0); // Exit the child process when done
    }

    // Parent process waits for the child to finish
    parent_process(pipefd, pid, tracker);

    get_in_heredoc(0);  // Reset heredoc flag (indicating heredoc process has ended)
    untrack_fd(tracker, pipefd[1]); // Close and untrack the writing end of the pipe
    return pipefd[0]; // Return the reading end of the pipe (where heredoc data can be read)
}

// Cleanup file descriptors and free memory if an error occurs
void cleanup_and_return(int *heredoc_fds, int count, t_fd_tracker *tracker)
{
    int j;

    j = 0;
    // Loop through each file descriptor and close it
    while (j < count)
    {
        close(heredoc_fds[j]); // Close the heredoc file descriptor
        untrack_fd(tracker, heredoc_fds[j]); // Untrack it from the file descriptor tracker
        j++;
    }
    free(heredoc_fds); // Free the memory allocated for heredoc file descriptors
}

// Handle multiple heredocs, creating a pipe for each one
int *handle_heredocs(char **red, int count, t_env *env, t_fd_tracker *tracker)
{
    int     *heredoc_fds; // Array of file descriptors for the heredocs
    char    *delimiter;   // The delimiter for the current heredoc
    char    *processed_delimiter; // Processed delimiter with special characters removed
    int     i;

    // Allocate memory to store the file descriptors for each heredoc
    heredoc_fds = malloc(sizeof(int) * count);
    if (!heredoc_fds) // If memory allocation fails, return NULL
        return NULL;

    i = 0;
    // Loop through each heredoc to process and create it
    while (i < count)
    {
        // Process the delimiter for the ith heredoc
        if (!process_delimiter(red, i, &delimiter, &processed_delimiter, tracker))
            return(cleanup_and_return(heredoc_fds, i, tracker), NULL); // Cleanup and return NULL if processing fails

        // Create the heredoc with the processed delimiter
        heredoc_fds[i] = create_heredoc(processed_delimiter, env, tracker);

        free(processed_delimiter); // Free the memory for the processed delimiter

        // If heredoc creation fails, cleanup and return NULL
        if (heredoc_fds[i] == -1)
            return (cleanup_and_return(heredoc_fds, i, tracker), NULL);

        i++; // Move to the next heredoc
    }
    return heredoc_fds; // Return the array of heredoc file descriptors
}

//==-=-=-=-=-=-=-=-

//heredoc helpers

//=-=-=-=-=-=-=-=-=-

// Appends a buffer to an existing line, resizing the line dynamically as needed.
static char *append_buffer(char *line, char *buffer, int bytes_read, int *total_size)
{
    char *new_line;
    int new_size = *total_size + bytes_read + 1;  // Calculate new size including null terminator

    new_line = malloc(new_size);  // Allocate memory for the new string
    if (!new_line)  // Check if malloc failed
    {
        free(line);  // Free the existing line to avoid memory leaks
        return NULL;  // Return NULL if memory allocation failed
    }
    if (line)  // If there was an existing line
    {
        ft_memcpy(new_line, line, *total_size);  // Copy old line data to new buffer
        free(line);  // Free the old line
    }
    ft_memcpy(new_line + *total_size, buffer, bytes_read);  // Append the buffer data to the new line
    *total_size = new_size - 1;  // Update the total size, excluding the null terminator
    new_line[*total_size] = '\0';  // Ensure the new line is null-terminated
    return new_line;  // Return the updated line
}

// Reads input from standard input (keyboard) and returns a single line.
char *read_line(void)
{
    char *line = NULL;  // Initialize an empty line
    char buffer[BUFFER_SIZE];  // Buffer to store chunks of input
    int bytes_read;
    int total_size = 0;  // Track total size of the line being built

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0)  // Read from stdin in chunks
    {
        line = append_buffer(line, buffer, bytes_read, &total_size);  // Append each chunk to the line
        if (!line)  // Check if appending failed
            return NULL;
        if (line[total_size - 1] == '\n')  // Stop if a newline is encountered
        {
            line[total_size - 1] = '\0';  // Replace newline with null terminator
            return line;  // Return the complete line
        }
    }

    // Handle errors or empty input
    if (bytes_read == -1 || (bytes_read == 0 && total_size == 0))
    {
        free(line);  // Free any allocated memory
        return NULL;  // Return NULL if there was an error or no input
    }
    return line;  // Return the final line
}

// Expands a variable inside heredoc content, replacing $VARIABLE with its value from the environment.
int	ft_expand_herdoc_var(char **var, t_env *env, t_type prv_type, int i)
{
	int		j;
	char	*var_name;

	while (var[0][i] != '\0')  // Iterate through the string until the end
	{
		if (var[0][i] == '\'')  // Skip single-quoted sections (variables aren't expanded inside them)
			i = ft_skipe_qoute(*var, i);
		else if (var[0][i] == '$' && (is_valid_var(var[0][i + 1]) == 1))  // Check if '$' is followed by a valid variable
		{
			j = ft_name_len(*var, i + 1);  // Get the length of the variable name
			var_name = ft_substr(*var, i + 1, j);  // Extract the variable name from the string
			if (!var_name)  // Check if extraction failed
				return (-1);
			if (prv_type == WORD)  // If this is a regular word (not quoted)
				if (ft_var_update(i, var, get_var_from_env(var_name, j, env),
						&var[0][i + j + 1]) == -1)  // Replace the variable with its value
					return (-1);  // Return error if variable expansion fails
		}
		else if ((var[0][i] == '$' && (is_valid_var(var[0][i + 1]) != 1))
			|| (var[0][i] != '$' && var[0][i] != '\0'))  // Continue if '$' is not followed by a valid variable
			i++;
	}
	return (0);  // Return success
}

// Expands all variables in a heredoc line.
int expand_variable(char **line, t_env *env)
{
    char *expanded_line;

    expanded_line = ft_strdup(*line);  // Create a copy of the original line
    if (expanded_line == NULL)  // Check if strdup failed
    {
        perror("ft_strdup");  // Print error
        free(*line);  // Free the original line
        return -1;  // Return error
    }
    if (ft_expand_herdoc_var(&expanded_line, env, WORD, 0) == -1)  // Expand variables in the line
    {
        free(expanded_line);  // Free the expanded line if expansion failed
        free(*line);  // Free the original line
        return -1;  // Return error
    }

    free(*line);  // Free the original line
    *line = expanded_line;  // Replace original line with expanded one
    return 0;  // Return success
}

// Writes a line to a pipe.
int write_to_pipe(int pipefd[2], char *line)
{
    write(pipefd[1], line, ft_strlen(line));  // Write the line to the pipe's write end
    write(pipefd[1], "\n", 1);  // Write a newline after the line
    return 0;  // Return success
}

// Counts the number of heredoc delimiters ("<<") in the redirection array.
int count_heredocs(char **red)
{
    int count;
    int i;

    count = 0;
    i = 0;
    if (!red || !(*red))  // Check if the redirection array is empty
        return (0);
    while (red[i] != NULL)  // Loop through the redirection array
    {
        if (ft_strcmp(red[i], "<<") == 0)  // Count occurrences of "<<"
            count++;
        i++;
    }
    return count;  // Return the total count of heredocs
}

// Child process for handling heredoc input.
void child_process(int pipefd[2], const char *delimiter, t_env *env, t_fd_tracker *fd_tracker)
{
    char *line;

    close(pipefd[0]);  // Close the read end of the pipe
    untrack_fd(fd_tracker, pipefd[0]);  // Untrack the file descriptor
    while ((line = read_line()) != NULL)  // Read input lines from stdin
    {
        if (ft_strcmp(line, delimiter) == 0)  // Stop if the line matches the heredoc delimiter
            break;
        if (fd_tracker->qout == 0)  // If no quotes are found around the delimiter, expand variables
            if (expand_variable(&line, env) == -1)  // Exit on expansion failure
                exit(1);
        write_to_pipe(pipefd, line);  // Write the line to the pipe
        free(line);  // Free the line after writing
    }
    close(pipefd[1]);  // Close the write end of the pipe after reading all lines
    untrack_fd(fd_tracker, pipefd[1]);  // Untrack the file descriptor
}

// Parent process for handling heredoc input.
void parent_process(int pipefd[2], pid_t pid, t_fd_tracker *fd_tracker)
{
    (void)pid;  // We don’t use the pid here, so it is cast to void to avoid compiler warnings
    int status;
    close(pipefd[1]);  // Close the write end of the pipe
    untrack_fd(fd_tracker, pipefd[1]);  // Untrack the file descriptor
    waitpid(pid, &status, 0);  // Wait for the child process to finish
}

// Checks if a string contains single or double quotes.
int check_if_qoutes(char *s)
{
    int i;

    i = 0;
    while (s[i])  // Iterate through the string
    {
        if (s[i] == '"' || s[i] == '\'')  // Return 1 if a single or double quote is found
            return (1);
        i++;
    }
    return (0);  // Return 0 if no quotes are found
}