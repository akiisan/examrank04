#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int 	execute(int i, char **av, int fd,char **env)
{
	av[i] = NULL;
	close(fd);
	execve(av[0],av,env);
	write(2, "Error exec :",13);
	write(2,av[0],sizeof(av[0]) / sizeof(char));
	write(2, "\n",1);
	return (1);
}

int		main(int ac, char **av, char **env)
{
	int tmp_fd = 0;
	int i = 0;
	pid_t pid = 0;
	int fd[2];

	tmp_fd = dup(STDIN_FILENO);
	while (av[i] && av[i+ 1])
	{
		av = &av[i + 1];
		i = 0;
		while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			i++;
		if (av && strcmp(av[0], "cd") == 0)
		{
			if (i != 2)
				write(2,"Error cd\n",10);
			else if (chdir(av[1]) != 0)
			{
				write(2, "Error cd to :",14);
				write(2,av[0],sizeof(av[0]) / sizeof(char));
				write(2, "\n",1);
			}
		}
		else if (av != &av[i] && (av[i] == NULL || strcmp(av[i], ";") == 0))
		{
			pid = fork();
			if (pid == 0)
			{
				dup2(tmp_fd, STDIN_FILENO);
				if (execute(i, av, tmp_fd, env))
					return (1);
			}
			else
			{
				close(tmp_fd);
				waitpid(-1, NULL, WUNTRACED);
				tmp_fd = dup(STDIN_FILENO);
			}
		}
		else if (av != &av[i] && strcmp(av[i], "|") == 0)
		{
			pipe(fd);
			pid = fork();
			if (pid == 0)
			{
				dup2(tmp_fd, STDIN_FILENO);
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				if (execute(i, av, tmp_fd, env))
					return (1);
			}
			else
			{
				close(fd[1]);
				close(tmp_fd);
				waitpid(-1, NULL, WUNTRACED);
				tmp_fd = dup(fd[0]);
			}
		}
	}
	close(tmp_fd);
	return (0);
}