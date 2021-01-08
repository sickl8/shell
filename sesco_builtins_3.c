/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sesco_builtins_3.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isaadi <isaadi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/09 17:08:48 by aamzouar          #+#    #+#             */
/*   Updated: 2021/01/06 17:58:50 by isaadi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "libft/libft.h"
#include "proto.h"
#include "typedef.h"
#include "def.h"
#include "global.h"
#include "errors.h"

#include <stdio.h>

int		check_var_name(char *name, int end)
{
	int		i;

	i = 0;
	if (!ft_isalpha(name[i]) && name[i] != '_')
		return (0);
	i++;
	while (i < end)
	{
		if (!ft_isalnum(name[i]) && name[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int		*check_errors_of_args(char **args, int len, int i, int j)
{
	int		*valid_args;
	char	*name;

	if (!(valid_args = ft_calloc(len, sizeof(int))))
		cleanup(EXIT);
	while (i < len + 1)
	{
		j = 0;
		while (args[i][j] != '=' && args[i][j] != '\0')
			j++;
		if (!check_var_name(args[i], j))
		{
			g_bash_errno = E_BUILTIN;
			g_builtin_errno = EB_UNSET_EXPORT_NVI;
			g_bash_commandid = BC_EXPORT;
			STRCPY(g_bash_error, args[i] ? args[i] : "");
			g_program_return = 1;
			bash_error();
		}
		// if it doesn't have equal on it consider it as invalid
		if (!check_var_name(args[i], j) || args[i][j] == '\0')
			valid_args[i - 1] = 1;
		i++;
	}
	return (valid_args);
}

t_evar	ft_realloc(char *name, char *value)
{
	t_evar	tmp;
	int		name_len;
	int		value_len;

	if (name)
	{
		name_len = ft_strlen(name);
		value_len = ft_strlen(value);
		if (!(MALLOC(&(tmp.name), name_len + 1)))
			cleanup(EXIT);
		ft_strcpy(tmp.name, name);
		if (!(MALLOC(&(tmp.value), value_len + 1)))
			cleanup(EXIT);
		ft_strcpy(tmp.value, value);
		tmp.name_len = name_len;
		tmp.value_len = value_len;
	}
	else
	{
		tmp.name = NULL;
		(tmp.value = ft_strdup("")) ? 0 : cleanup(EXIT);
		tmp.name_len = -1;
		tmp.value_len = -1;
	}
	return (tmp);
}

int		bc_cd(t_cmd *data)
{
	g_parent = 1;
	if (data->args[1] && CMP(data->args[1], "-"))
	{
		if (change_dir(data->args[1], getcwd(NULL, 0)) < 0)
		{
			g_program_return = 1;
			// g_bash_errno = E_ERRNO;
			// ft_strncpy(g_bash_error, data->args[1], -1);
			// g_bash_commandid = BC_CD;
			// bash_error();
			return (1);
		}
	}
	else
		return (go_to(data));
	return (0);
}
