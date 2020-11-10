/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isaadi <isaadi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/04 12:53:04 by isaadi            #+#    #+#             */
/*   Updated: 2020/11/10 16:47:07 by isaadi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>
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

void	check_command()
{

}

void	check_redir()
{

}

void	check_syntax()
{
	t_bm	p;
	t_bm	tmp;
	int		i;
	char	c;

	i = -1;
	p = next_word(g_line->rd);
	while (p.msk)
	{
		if (STR_IS_REDIR(p))
		{
			tmp = p;
			tmp.buf += tmp.cnt;
			tmp.msk += tmp.cnt;
			if (!tmp.msk || STR_IS_REDIR(tmp) || BASHSYN(tmp.msk[0]))
			{
				g_bash_errno = ESYNTAX;
				return ;
			}
		}
		else if (BASHSYN(p.msk[0]))
		{
			c = '|' + ';' - p.msk[0];
			tmp = p;
			tmp = previous_word(tmp, g_line->rd);
			if (!tmp.msk || tmp.msk[0] == c)
			{
				g_bash_errno = ESYNTAX;
				return ;
			}
		}
		p.buf += p.cnt;
		p.msk += p.cnt;
		p = next_word(p);
	}
}

int		count_quote(char c)
{
	int		ret;
	char	*p;

	ret = 0;
	p = ft_strchr(g_line->rd.msk, c);
	while (p)
	{
		ret++;
		p = ft_strchr(++p, c);
	}
	return (ret);
}

void	check_multiline()
{
	char	*p;

	p = ft_strrchr(g_line->rd.msk, '|');
	if (p)
		while (*(++p))
			if (*p != WHTSPC)
				break ;
	if (count_quote('\'') % 2 || count_quote('"') % 2 ||
	(g_line->rd.buf[g_line->rd_len - 1] == '\\' &&
	g_line->rd.msk[g_line->rd_len - 1] == '\\') || (p && !*p))
		g_bash_errno = EMULTILINE;
}

void	initial_error_check()
{
	check_multiline();
	if (g_bash_errno)
		return ;
	check_syntax();
	if (g_bash_errno)
		return ;
	check_redir();
	if (g_bash_errno)
		return ;
}

int		assign(void *p, unsigned long long v, int size)
{
	if (size == 1)
		*((char*)(p)) = (char)v;
	else if (size == 2)
		*((short*)(p)) = (short)v;
	else if (size == 4)
		*((int*)(p)) = (int)v;
	else if (size == 8)
		*((long*)(p)) = (long)v;
	return (0);
}

void	copy_valid_chars(char *dst, char *src, char *msk, size_t tr_ln)
{
	size_t	i;

	i = 0;
	while (src[i] && i < tr_ln)
	{
		while (src[i] && msk[i] != LITERAL && msk[i] != SEMILIT &&
		msk[i] != NONLIT && msk[i] != '<' && msk[i] != '>' && msk[i] != '2')
		{
			src++;
			msk++;
		}
		if (!src[i])
			break ;
		dst[i] = src[i];
		i++;
	}
}

size_t	true_len(t_bm p)
{
	size_t	ret;

	if ((!(NCMP(p.buf, p.msk, p.cnt)) && (IS_REDIR(p.msk[0]))) ||
	(p.cnt == 2 && !NCMP(p.msk, "22", 2) && !NCMP(p.buf, ">>", 2)))
		return (p.cnt);
	ret = 0;
	while (p.buf[0] && p.msk[0] != WHTSPC && p.msk[0] != '>' && p.msk[0] != '<')
	{
		if (p.msk[0] == SEMILIT || p.msk[0] == LITERAL || p.msk[0] == NONLIT)
			ret++;
		p.buf++;
		p.msk++;
	}
	return (ret);
}

void	continue_split_redirects(t_bm *rd, t_bm **wr, size_t wc)
{
	t_bm	p;
	size_t	tr_ln;
	size_t	i;

	i = 0;
	p = next_word(*rd);
	while (i < wc)
	{
		wr[0][i].buf = NULL;
		wr[0][i].msk = NULL;
		tr_ln = true_len(p);
		if (!(MALLOC(wr[0][i].buf, tr_ln + 1)))
			cleanup(EXIT);
		wr[0][i].buf[tr_ln] = '\0';
		if (!(MALLOC(wr[0][i].msk, tr_ln + 1)))
			cleanup(EXIT);
		wr[0][i].msk[tr_ln] = '\0';
		copy_valid_chars(wr[0][i].buf ,p.buf, p.msk, tr_ln);
		copy_valid_chars(wr[0][i].msk ,p.msk, p.msk, tr_ln);
		p.msk += p.cnt;
		p.buf += p.cnt;
		p = next_word(p);
		i++;
	}
}

t_bm	previous_word(t_bm rd, t_bm ref)
{
	while (ref.msk < (rd.msk - 1) && rd.msk[-1] == WHTSPC)
	{
		rd.msk--;
		rd.buf--;
	}
	if (rd.msk[-1] == WHTSPC)
	{
		rd.msk = NULL;
		return (rd);
	}
	if (BASHSYN(rd.msk[-1]))
	{
		rd.cnt = 1;
		return (rd);
	}
	else if (rd.msk[-1] == '2')
	{
		rd.cnt = 2;
		return (rd);
	}
	rd.cnt = 0 * (int)rd.msk-- * 0 * (int)rd.buf--;
	while (rd.msk < ref.msk && rd.msk[0] != WHTSPC &&
	!BASHSYN(rd.msk[0]))
		rd.cnt += 1 + 0 * ((int)rd.buf-- + (int)rd.msk--);
	return (rd);
}

t_bm	next_word(t_bm rd)
{
	while (rd.msk[0] == WHTSPC && rd.buf[0])
	{
		rd.msk++;
		rd.buf++;
	}
	if (BASHSYN(rd.msk[0]))
	{
		rd.cnt = 1;
		return (rd);
	}
	else if (rd.msk[0] == '2')
	{
		rd.cnt = 2;
		return (rd);
	}
	else if (!rd.buf[0])
	{
		rd.msk = NULL;
		return (rd);
	}
	rd.cnt = 0;
	while (rd.msk[rd.cnt] != WHTSPC &&
	!BASHSYN(rd.msk[rd.cnt]) && rd.buf[rd.cnt])
		rd.cnt++;
	return (rd);
}

size_t	count_words_redir(t_bm *rd)
{
	size_t	ret;
	t_bm	p;

	ret = 0;
	p = next_word(*rd);
	while (p.msk)
	{
		ret++;
		p.msk += p.cnt;
		p.buf += p.cnt;
		p = next_word(p);
	}
	return (ret);
}

void	split_redirect(t_bm *rd, t_bm **wr)
{
	size_t	wc;

	wc = count_words_redir(rd);
	if (!(MALLOC(*wr, wc + 1)))
		cleanup(EXIT);
	wr[0][wc].buf = NULL;
	wr[0][wc].msk = NULL;
	continue_split_redirects(rd, wr, wc);
}

void	split_redirects()
{
	size_t	i;
	size_t	j;

	if (!(MALLOC(g_line->redir, g_line->env.cnt + 1)))
		cleanup(EXIT);
	g_line->redir[g_line->env.cnt] = NULL;
	i = -1;
	while (++i < g_line->env.cnt)
	{
		g_line->redir[i] = NULL;
		if (!(MALLOC(g_line->redir[i], g_line->scol[i].cnt + 1)))
			cleanup(EXIT);
		g_line->redir[i][g_line->scol[i].cnt] = NULL;
		j = -1;
		while (++j < g_line->scol[i].cnt)
		{
			g_line->redir[i][j] = NULL;
			split_redirect(&g_line->pipe[i][j], &g_line->redir[i][j]);
		}
	}
}

void	split_pipe()
{
	size_t	i;

	i = -1;
	while (++i < g_line->env.cnt)
	{
		g_line->pipe[i] = NULL;
		split_wmask(&g_line->scol[i], &g_line->pipe[i], '|');
	}
}

void	continue_split_wmask(t_bm *rd, t_bm **wr, size_t cnt, char c)
{
	size_t	i;
	char	*p;
	char	*bk;

	p = rd->msk;
	bk = rd->msk;
	i = -1;
	while (++i < cnt)
	{
		wr[0][i].msk = NULL;
		wr[0][i].buf = NULL;
		if (!(p = ft_strchr(p, c)))
			p = ft_strchr(rd->msk, '\0');
		if (!(MALLOC(wr[0][i].buf, p - bk + 1)))
			cleanup(EXIT);
		wr[0][i].buf[p - bk] = '\0';
		if (!(MALLOC(wr[0][i].msk, p - bk + 1)))
			cleanup(EXIT);
		wr[0][i].msk[p - bk] = '\0';
		ft_strncpy(wr[0][i].buf, rd->buf + (bk - rd->msk), p - bk);
		ft_strncpy(wr[0][i].msk, rd->msk + (bk - rd->msk), p - bk);
		bk = ++p;
	}
}

void	split_wmask(t_bm *rd, t_bm **wr, char c)
{
	size_t	i;
	size_t	cnt;

	cnt = 1;
	i = 0;
	while (rd->buf[i])
	{
		if (rd->msk[i] == c)
			cnt++;
		i++;
	}
	if (!(MALLOC(*wr, cnt + 1)))
		cleanup(EXIT);
	wr[0][cnt].msk = NULL;
	wr[0][cnt].buf = NULL;
	continue_split_wmask(rd, wr, cnt, c);
	rd->cnt = cnt;
}

void	reset_stdin(int *bk)
{
	if (close(0) == -1)
		handle_error();
	if (dup2(*bk, 0) == -1)
		handle_error();
	if (close(*bk) == -1)
		handle_error();
	backup_stdin(bk);
}

void	backup_stdin(int *bk)
{
	if ((*bk = dup(STDIN_FILENO)) == -1)
		handle_error();
}

void	continue_rplc_env_var()
{
	size_t	i;
	size_t	j;
	t_evar	env;

	i = -1;
	j = 0;
	while (g_line->rd.buf[++i])
	{
		if (g_line->rd.msk[i] == '$')
		{
			env = find_env_in_line(&i);
			ft_strncpy(&g_line->env.buf[j], env.value, env.value_len);
			ft_memset(&g_line->env.msk[j], LITERAL, env.value_len);
			j += env.value_len;
		}
		else
		{
			g_line->env.buf[j] = g_line->rd.buf[i];
			g_line->env.msk[j] = g_line->rd.msk[i];
			j++;
		}
	}
}

t_evar	find_env(char *s)
{
	size_t	j;

	j = 0;
	while (g_line->env_var[j].name)
	{
		if (!ft_strcmp(g_line->env_var[j].name, s))
			break ;
		j++;
	}
	return (g_line->env_var[j]);
}

t_evar	find_env_in_line(size_t *ref)
{
	t_evar	ret;
	size_t	i;
	size_t	j;
	char	env_space[ENV_NAME_LEN_MAX];

	(*ref)++;
	i = *ref;
	j = 0;
	while (g_line->rd.buf[i] && g_line->rd.msk[i] == ENVVAR &&
	i - *ref < ENV_NAME_LEN_MAX)
		env_space[j++] = g_line->rd.buf[i++];
	env_space[j] = '\0';
	ret = find_env(env_space);
	if (!ret.name)
	{
		ret.name_len = ft_strlen(env_space);
		ret.value_len = 0;
	}
	*ref = --i;
	return (ret);
}

void	rplc_env_var()
{
	size_t	tlen;
	t_evar	env;
	size_t	i;
	size_t	j;

	i = -1;
	tlen = 0;
	while (g_line->rd.buf[++i])
	{
		if (g_line->rd.msk[i] == '$')
			tlen += find_env_in_line(&i).value_len;
		else
			tlen++;
	}
	if (!(MALLOC(g_line->env.buf, tlen + 1)))
		cleanup(EXIT);
	if (!(MALLOC(g_line->env.msk, tlen + 2)))
		cleanup(EXIT);
	ft_memset(g_line->env.buf, 0, tlen + 1);
	ft_memset(g_line->env.msk, 0, tlen + 2);
	continue_rplc_env_var();
}

int		is_ws(char c)
{
	if (c == ' ' || c == '\n' || c == '\t' ||
	c == '\v' || c == '\f' || c == '\r' || c == '\0')
		return (1);
	return (0);
}

int		env_var_comp(char c)
{
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
	(c >= 'a' && c <= 'z') || c == '_')
		return (1);
	return (0);
}

void	env_var(size_t *ref)
{
	size_t	i;

	i = *ref;
	g_line->rd.msk[i] = '$';
	if (g_line->rd.buf[++i] && env_var_comp(g_line->rd.buf[i]) &&
	!(g_line->rd.buf[i] >= '0' && g_line->rd.buf[i] <= '9'))
	{
		*ref = i;
		g_line->rd.msk[i] = ENVVAR;
		while (g_line->rd.buf[++i] && env_var_comp(g_line->rd.buf[i]) &&
		i - *ref < ENV_NAME_LEN_MAX)
			g_line->rd.msk[i] = ENVVAR;
	}
	i--;
	*ref = i;
}

void	bash_syntax(size_t *ref)
{
	size_t	i;

	i = *ref;
	g_line->rd.msk[i] = g_line->rd.buf[i];
	if (g_line->rd.buf[i] == '>' && g_line->rd.buf[i + 1] == '>')
	{
		g_line->rd.msk[i] = '2';
		i++;
		g_line->rd.msk[i] = '2';
	}
	*ref = i;
}

void	squote(size_t *ref)
{
	size_t	i;

	i = *ref;
	g_line->rd.msk[i] = '\'';
	while (g_line->rd.buf[++i] && g_line->rd.buf[i] != '\'')
		g_line->rd.msk[i] = LITERAL;
	if (g_line->rd.buf[i])
		g_line->rd.msk[i] = '\'';
	else
	{
		g_bash_errno = EMULTILINE;
		i--;
	}
	*ref = i;
}

void	dquote(size_t *ref)
{
	size_t	i;

	i = *ref;
	g_line->rd.msk[i] = '"';
	while (g_line->rd.buf[++i] && g_line->rd.buf[i] != '"')
	{
		if (g_line->rd.buf[i] == '\\')
			backslash(&i, SEMILIT);
		else if (g_line->rd.buf[i] == '$')
			env_var(&i);
		else
			g_line->rd.msk[i] = SEMILIT;
	}
	if (g_line->rd.buf[i])
		g_line->rd.msk[i] = '"';
	else
	{
		g_bash_errno = EMULTILINE;
		i--;
	}
	*ref = i;
}

void	backslash(size_t *ref, char lit_type)
{
	g_line->rd.msk[I] = '\\';
	if (lit_type == NONLIT)
	{
		if (g_line->rd.buf[I + 1])
			g_line->rd.msk[PPI] = LITERAL;
		else
			g_bash_errno = EMULTILINE;
	}
	else if (lit_type == SEMILIT)
	{
		if (g_line->rd.buf[I + 1])
		{
			if (g_line->rd.buf[PPI] == '"' || g_line->rd.buf[I] == '\\' ||
			g_line->rd.buf[I] == '$')
				g_line->rd.msk[I] = LITERAL;
			else
				A(g_line->rd.msk[I - 1], SEMILIT) &&
				A(g_line->rd.msk[I], SEMILIT);
		}
		else
			g_bash_errno = EMULTILINE;
	}
}

int		set_mask()
{
	size_t	i;

	i = -1;
	while (g_line->rd.buf[++i])
	{
		if (g_line->rd.buf[i] == '\\')
			backslash(&i, NONLIT);
		else if (g_line->rd.buf[i] == '"')
			dquote(&i);
		else if (g_line->rd.buf[i] == '\'')
			squote(&i);
		else if (g_line->rd.buf[i] == '|' || g_line->rd.buf[i] == '>' ||
		g_line->rd.buf[i] == '<' || g_line->rd.buf[i] == ';')
			bash_syntax(&i);
		else if (g_line->rd.buf[i] == '$')
			env_var(&i);
		else if (is_ws(g_line->rd.buf[i]))
			g_line->rd.msk[i] = WHTSPC;
		else
			g_line->rd.msk[i] = NONLIT;
	}
	return (0);
}

// void	syntax_error(char *error)
// {
// 	PRINTS("minishell: syntax error near unexpected token `");
// 	PRINT(error);
// 	PRINTS("'\n");
// }

// void	bash_error(int er, char *error)
// {
// 	if (er == ESYNTAX)
// 		syntax_error(error);
// 	else if (er == ECOMMAND)
// 		command_not_found(error);
// 	else if (er == EERRNO)
// 		errno_has_awaken(error);
// 	else if (er == EMULTILINE)
// 		multiline(error);
// }

void	free_redir()
{
	size_t	i;
	size_t	j;
	size_t	k;

	i = -1;
	while (g_line->redir[++i])
	{
		j = -1;
		while (g_line->redir[i][++j])
		{
			k = -1;
			while (g_line->redir[i][j][++k].buf)
				free_buf_and_mask(g_line->redir[i][j][k]);
			free(g_line->redir[i][j]);
		}
		free(g_line->redir[i]);
	}
	free(g_line->redir);
}

void	free_g_bw()
{
	int	i;

	i = -1;
	while (++i < MAX_OPEN_FD)
		free(g_bw.buf[0][i]);
	free(g_bw.buf[0]);
	free(g_bw.buf_i[0]);
}

void	free_pipe()
{
	int	i;
	int	j;

	i = -1;
	while (g_line->pipe[++i])
	{
		j = -1;
		while (g_line->pipe[i][++j].buf)
		{
			free(g_line->pipe[i][j].buf);
			free(g_line->pipe[i][j].msk);
		}
		free(g_line->pipe[i]);
	}
	free(g_line->pipe);
}

void	free_scol()
{
	int	i;

	i = -1;
	while (g_line->scol[++i].buf && g_line->scol[i].msk)
	{
		free(g_line->scol[i].buf);
		free(g_line->scol[i].msk);
	}
	free(g_line->scol);
}

void	free_envar()
{
	int	i;
	
	i = -1;
	while (g_line->env_var[++i].name_len != -1 &&
	g_line->env_var[i].value_len != -1)
	{
		free(g_line->env_var[i].name);
		free(g_line->env_var[i].value);
	}
	free(g_line->env_var[i].value);
}

int		cleanup(int ex)
{
	free(g_line->rd.buf);
	free(g_line->rd.msk);
	free(g_line->env.buf);////
	free(g_line->env.msk);////
	if (g_line->env_var)
		free_envar();
	if (g_line->scol)///////
		free_scol();
	if (g_line->pipe)///////
		free_pipe();
	free(g_line->env_var);
	if (g_bw.buf[0])
		free_g_bw();
	if (g_line->redir)
		free_redir();
	if (ex)
		handle_error();
	return (0);
}

void	free_and_set_to_null(void *adr)
{
	void	**cast;

	cast = (void**)adr;
	free(*cast);
	*cast = NULL;
}

void	fastn()
{
	free_and_set_to_null(&g_line->redir);
	free_and_set_to_null(&g_line->scol);
	free_and_set_to_null(&g_line->pipe);
	free_and_set_to_null(&g_line->env.buf);
	free_and_set_to_null(&g_line->env.msk);
}

void	free_buf_and_mask(t_bm s)
{
	free(s.buf);
	free(s.msk);
}

void	free_tmp()
{
	int	i;
	int	j;
	int	k;

	i = -1;
	while (g_line->scol[++i].buf)
	{
		free_buf_and_mask(g_line->scol[i]);
		j = -1;
		while (g_line->pipe[i][++j].buf)
		{
			free_buf_and_mask(g_line->pipe[i][j]);
			k = -1;
			while (g_line->redir[i][j][++k].buf)
				free_buf_and_mask(g_line->redir[i][j][k]);
			free(g_line->redir[i][j]);
		}
		free(g_line->pipe[i]);
		free(g_line->redir[i]);
	}
	fastn();
}

////////////////////////////////////////////////////////////////////////////////

int		format_string()
{
	// printf("str = |%s|\n", line.rd.buf);
	// printf("msk = |%s|\n", line.mask);
	// printf("env = |%s|\n", line.buf);
	// printf("msk = |%s|\n", line.msk);
	// for (int i = 0; i < g_line->env.cnt; i++)
	// {
	// 	for (int j = 0; j < g_line->scol[i].cnt; j++)
	// 	{
	// 		for (int k = 0; g_line->redir[i][j][k].buf; k++)
	// 		{
	// 			printf("buf[%d][%d][%d] = |%s|\n", i, j, k, g_line->redir[i][j][k].buf);
	// 			printf("msk[%d][%d][%d] = |%s|\n", i, j, k, g_line->redir[i][j][k].msk);
	// 		}
	// 	}
	// }
	char	*tmp;
	int		ret;

	g_bash_errno = 0;
	if ((tmp = ft_strchr(g_line->rd.buf, '\n')))
		*tmp = '\0' * g_line->rd_ret--;
	g_line->rd_len = ft_strlen(g_line->rd.buf);
	set_mask();
	if (g_bash_errno)
		return (g_bash_errno);
	rplc_env_var();
	split_wmask(&g_line->env, &g_line->scol, ';');
	if (!(MALLOC(g_line->pipe, g_line->env.cnt + 1)))
		cleanup(EXIT);
	g_line->pipe[g_line->env.cnt] = NULL;
	split_pipe();
	split_redirects();
	for (int i = 0; g_line->scol[i].buf; i++)
	{
		printf("scol[%d].buf = |%s|\n", i, g_line->scol[i].buf);
		printf("scol[%d].msk = |%s|\n", i, g_line->scol[i].msk);
		for (int j = 0; g_line->pipe[i][j].buf; j++)
		{
			printf("pipe[%d][%d].buf = |%s|\n", i, j, g_line->pipe[i][j].buf);
			printf("pipe[%d][%d].msk = |%s|\n", i, j, g_line->pipe[i][j].msk);
			for (int k = 0; g_line->redir[i][j][k].buf; k++)
			{
				printf("buf[%d][%d][%d] = |%s|\n", i, j, k, g_line->redir[i][j][k].buf);
				printf("msk[%d][%d][%d] = |%s|\n", i, j, k, g_line->redir[i][j][k].msk);
			}
		}
	}
	initial_error_check();
	// else if (check_command())
	// 	ret = ECOMMAND;
	free_tmp();
	return (g_bash_errno);
}

void	reset_prompt(void)
{
	PRINTS("\b\b  \n");
	init_read(*g_line);
}

void	exit_the_shell(void)
{
	PRINTS("exit\n");
	cleanup(RETURN);
	exit(0);
}

void	ctrl_d(int *bk)
{
	reset_stdin(bk);
	PRINTS("  \n");
}

void	handle_signal(int sig)
{
	reset_prompt();
}

void	handle_error(void)
{
	printf("error?\n");
	exit(1);
}

void	print_color(int color)
{
	if (color == 0)
		BPRINTS(ESC_RED_B);
	else if (color == 1)
		BPRINTS(ESC_ORANGE_B);
	else if (color == 2)
		BPRINTS(ESC_YELLOW_B);
	else if (color == 3)
		BPRINTS(ESC_LIME_B);
	else if (color == 4)
		BPRINTS(ESC_GREEN_B);
	else if (color == 5)
		BPRINTS(ESC_TURQUOISE_B);
	else if (color == 6)
		BPRINTS(ESC_CYAN_B);
	else if (color == 7)
		BPRINTS(ESC_BLUE_B);
	else if (color == 8)
		BPRINTS(ESC_PINK_B);
	else if (color == 9)
		BPRINTS(ESC_PURPLE_B);
	else if (color == 10)
		BPRINTS(ESC_MAGENTA_B);
}

void	skittles(char *s)
{
	static int	color;
	int			i;

	i = -1;
	while (s[++i])
	{
		print_color(color);
		BPRINTC(s[i]);
		color++;
		color %= 11;
	}
}

void	init_read()
{
	t_evar	home;
	t_evar	pwd;
	t_evar	user;

	home = find_env("HOME");
	pwd = find_env("PWD");
	user = find_env("USER");
	ft_memset(g_line->rd.buf, '\0', ARG_MAX + 2);
	ft_memset(g_line->rd.msk, '\0', ARG_MAX + 3);
	skittles(user.value);
	skittles("@minishell");
	BPRINTS(ESC_RESET ":");
	BPRINTS(ESC_BLUE_B);
	if (ft_strstr(pwd.value, home.value) == pwd.value)
	{
		BPRINTS("~");
		BPRINT(pwd.value + home.value_len);
	}
	else
		BPRINT(pwd.value);
	BPRINTS(ESC_RESET "$ ");
	bflush(STDOUT_FILENO);
}

void	continue_init_env()
{
	int		i;

	i = -1;
	while (g_line->envp[++i])
	{
		g_line->env_var[i].name_len = ft_strchr(g_line->envp[i], '=') - g_line->envp[i];
		if (!(MALLOC(g_line->env_var[i].name, g_line->env_var[i].name_len + 1)))
			cleanup(EXIT);
		g_line->env_var[i].name[g_line->env_var[i].name_len] = '\0';
		ft_strncpy(g_line->env_var[i].name,
		g_line->envp[i], g_line->env_var[i].name_len);
		if (!(g_line->env_var[i].value =
		ft_strdup(ft_strchr(g_line->envp[i], '=') + 1)))
			cleanup(EXIT);
		g_line->env_var[i].value_len = ft_strlen(g_line->env_var[i].value);
	}
}

void	init_env()
{
	int		i;

	i = 0;
	while (g_line->envp[i])
		i++;
	if (!(MALLOC(g_line->env_var, i + 1)))
		cleanup(EXIT);
	i = -1;
	while (g_line->envp[++i])
	{
		g_line->env_var[i].name = NULL;
		g_line->env_var[i].value = NULL;
	}
	g_line->env_var[i].name = NULL;
	g_line->env_var[i].name_len = -1;
	g_line->env_var[i].value_len = -1;
	if (!(MALLOC(g_line->env_var[i].value, 1)))
		cleanup(EXIT);
	g_line->env_var[i].value[0] = '\0';
	continue_init_env();
}

void	init_buf()
{
	if (!(MALLOC(g_line->rd.buf, ARG_MAX + 2)))
		handle_error();
	if (!(MALLOC(g_line->rd.msk, ARG_MAX + 3)))
	{
		free(g_line->rd.buf);
		handle_error();
	}
}

void	init_line()
{
	g_line->rd.buf = NULL;
	g_line->rd.msk = NULL;
	g_line->env_var = NULL;
	g_line->env.buf = NULL;
	g_line->env.msk = NULL;
	g_line->scol = NULL;
	g_line->pipe = NULL;
	g_line->redir = NULL;
}

void	init_globals(t_line *ref)
{
	g_line = ref;
	g_bw.buf = NULL;
	g_bw.buf_i = NULL;
}

void	init(t_line *ref)
{
	init_globals(ref);
	init_line();
	init_buf();
	init_env();
}

int		main(int ac, char **av, char **envp)
{
	t_line	line;
	int		stdin_bak;

	signal(SIGINT, handle_signal);
	signal(SIGQUIT, handle_signal);
	line.envp = envp;
	init(&line);
	backup_stdin(&stdin_bak);
	while (1)
	{
		init_read();
		line.rd_ret = read(STDIN_FILENO, line.rd.buf, ARG_MAX);
		if (line.rd_ret < 0)
			handle_error();
		else if (!ft_strchr(line.rd.buf, '\n') && line.rd_ret > 0)
			ctrl_d(&stdin_bak);
		else if (line.rd_ret == 0)
			exit_the_shell();
		else
		{
			if (!format_string())
			{
				if (!CMP(line.rd.buf, "exit"))
					return (cleanup(RETURN));
			}
			else
			{
				printf("error\n");
			}
		}
	}
}
