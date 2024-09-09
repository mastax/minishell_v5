//sel-hasn

#include "../mini_shell.h"

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
