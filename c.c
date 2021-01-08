#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
#define TIME(x) b = clock(); x printf("The section : \"%.20s\"[...] took %lfs = %lfms\n", #x, (double)(clock() - b) / CLOCKS_PER_SEC, ((double)(clock() - b) / CLOCKS_PER_SEC) * 1000)
#define LEN 0xffffffff
clock_t b;

int main()
{
	pid_t pid;
	char *s = malloc(LEN);
	for (register size_t i = 0; i < LEN - 1; i++)
		s[i] = i % 94 + 32;
   	s[LEN - 1] = 0;	
	TIME(pid = fork(););
	if (pid == 0)
	{
		s[LEN / 2] = 'a';
		//child
	}
	else
	{
		wait(NULL);
		//parent
	}
}
