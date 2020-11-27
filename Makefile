all: minishell

minishell:
	gcc -fsanitize=address main.c sesco.c libft/ft_strncpy.c libft/ft_strlen.c libft/ft_memset.c libft/ft_strchr.c libft/ft_strcmp.c libft/ft_strdup.c libft/ft_strstr.c bwrite.c libft/ft_strncmp.c libft/ft_memcmp.c libft/ft_strrchr.c libft/ft_strnstr.c libft/ft_split.c libft/ft_strjoin.c libft/ft_strlcpy.c -g -o minishell && export ASAN_OPTIONS=detect_leak=1

clean:
	rm -rf *.o

fclean:
	rm -rf minishell

re: fclean minishell
