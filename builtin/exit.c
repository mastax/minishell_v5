/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-bah <elel-bah@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 16:55:17 by elel-bah          #+#    #+#             */
/*   Updated: 2024/09/10 15:41:43 by elel-bah         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../mini_shell.h"
#include <errno.h>
#include <limits.h>


static int	ft_convert_digit(char c, int base)
{
	if (is_number(c))
		return (c - '0');
	else if (base == 16)
	{
		if (c >= 'a' && c <= 'f')
			return (c - 'a' + 10);
		else if (c >= 'A' && c <= 'F')
			return (c - 'A' + 10);
	}
	return (0);
}

static int	handle_sign_and_prefix(const char **nptr)
{
	int	sign;

	sign = 1;
	while (ft_is_space(**nptr))
		(*nptr)++;
	if (**nptr == '-' || **nptr == '+')
	{
		if (**nptr == '-')
			sign = -1;
		(*nptr)++;
	}
	return (sign);
}

static long long	ft_strtoll(const char *nptr, char **endptr, int base)
{
	int			sign;
	long long	result;
	long long	prev_result;

	sign = handle_sign_and_prefix(&nptr);
	result = 0;
	while (is_number(*nptr) || (base == 16 && ft_isxdigit(*nptr)))
	{
		prev_result = result;
		result = result * base + ft_convert_digit(*nptr, base);
		if (result / base != prev_result)
		{
			errno = ERANGE;
			break ;
		}
		nptr++;
	}
	if (endptr)
		*endptr = (char *)nptr;
	return (sign * result);
}

// static long long	ft_strtoll(const char *nptr, char **endptr, int base)
// {
//     int			sign;
//     long long	result;
//     long long	prev_result;
	
//     while (ft_is_space(*nptr))
//         nptr++;
//     sign = 1;
//     if (*nptr == '-' || *nptr == '+')
//     {
//         if (*nptr == '-')
//             sign = -1;
//         nptr++;
//     }
//     result = 0;
//     while (is_number(*nptr) || (base == 16 && ft_isxdigit(*nptr)))
//     {
//         prev_result = result;
//         result = result * base + ft_convert_digit(*nptr, base);
//         if (result / base != prev_result)
//         {
//             errno = ERANGE;
//             break;
//         }
//         nptr++;
//     }
//     if (endptr)
//         *endptr = (char *)nptr;
//     return (sign * result);
// }

// static long long	ft_strtoll(const char *nptr, char **endptr, int base)
// {
// 	int			sign;
// 	long long	result;
	
// 	while (ft_is_space(*nptr))
// 		nptr++;
// 	sign = 1;
// 	if (*nptr == '-' || *nptr == '+')
// 	{
// 		if (*nptr == '-')
// 			sign = -1;
// 		nptr++;
// 	}
// 	result = 0;
// 	while (is_number(*nptr) || (base == 16 && ft_isxdigit(*nptr)))
// 	{
// 		result = result * base + ft_convert_digit(*nptr, base);
// 		nptr++;
// 	}
// 	if (endptr)
// 		*endptr = (char *)nptr;
// 	return (sign * result);
// }


int ft_exit(char **av, int *exit_status)
{
	long long	exit_code;
	char		*endptr;

	printf("exit\n");
	if (!av[1])
		exit(*exit_status);
	errno = 0;  // Reset errno before the conversion
	exit_code = ft_strtoll(av[1], &endptr, 10);
	
	if (*endptr != '\0' || av[1][0] == '\0' || errno == ERANGE)
	{
		ft_putstr_fd("exit: ", 2);
		ft_putstr_fd(av[1], 2);
		ft_putstr_fd(": numeric argument required\n", 2);
		exit(255);
	}
	if (av[2])
	{
		ft_putstr_fd("exit: too many arguments\n", 2);
		get_exit_status(1);
		return (1);
	}
	exit((int)(exit_code & 255));
}

// int ft_exit(char **av, int *exit_status)
// {
//     long long exit_code;
//     char *endptr;

//     printf("exit\n");
//     if (!av[1])
//         exit(*exit_status);
//     exit_code = ft_strtoll(av[1], &endptr, 10);
//     if (*endptr != '\0' || av[1][0] == '\0')
//     {
//         ft_putstr_fd("exit: ", 2);
//         ft_putstr_fd(av[1], 2);
//         ft_putstr_fd(": numeric argument required\n", 2);
//         exit(255);
//     }
//     if (av[2])
//     {
//         ft_putstr_fd("exit: too many arguments\n", 2);
//         get_exit_status(1);
//         return (1);
//     }
//     exit((int)(exit_code & 255));
// }
