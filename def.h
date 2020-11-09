/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   def.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isaadi <isaadi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/04 13:06:51 by isaadi            #+#    #+#             */
/*   Updated: 2020/11/09 20:33:00 by isaadi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEF_H
# define DEF_H

# define MALLOC(x, y) x = malloc(sizeof(*(x)) * (y))
# define ENV_NAME_LEN_MAX 32760
# define WHTSPC 'W'
# define NONLIT 'N'
# define ENVVAR 'V'
# define SEMILIT 'S'
# define LITERAL 'L'
# define RDRWORD 'R'
# define BASHSYN(x) (x == '|' || x == '>' || x == '<' || x == ';')
# define ARG_MAX 262144
# define CMP(x, y) (ft_strcmp(x, y))
# define NCMP(x, y, n) (ft_strncmp(x, y, n))
# define PRINTS(x) write(STDOUT_FILENO, x, sizeof(x))
# define BPRINTS(x) bwrite(STDOUT_FILENO, x, sizeof(x) - 1)
# define PRINT(x) write(STDOUT_FILENO, x, ft_strlen(x))
# define BPRINT(x) bwrite(STDOUT_FILENO, x, ft_strlen(x))
# define PRINTC(x) write(STDOUT_FILENO, &x, 1)
# define BPRINTC(x) bwrite(STDOUT_FILENO, &x, 1)
# define H printf("\nline: %d\n", __LINE__)
# define BWR(fd, string) bwrite(fd, string, ft_strlen(string))
# define IS_REDIR(x) (x == '<' || x == '>')
# define EXIT 1
# define RETURN 0
# define A(x, y) assign(&x, y, sizeof(x))
# define I *ref
# define IPP (*ref)++
# define PPI ++(*ref)

# include "bufferedio.h"
# include "colors.h"
# include "cursor.h"

#endif
