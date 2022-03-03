#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

int	execute(int i, char **av, char **env, int fd)
{
	av[i] = NULL;
	close(fd);
	execve(av[0],av,env);
	write(2, "Error exec :",13);
	write(2,av[0],sizeof(av[0]) / sizeof(char));
	write(2, "\n",1);
	return (1);
}

int	main(int ac, char **av, char **env)
{
	int i = 0;
	pid_t pid = 0;
	int tmp_fd = 0;
	int fd[2];
	int cmd = 0;
	tmp_fd = dup(0);
	while (av[i] && av[i + 1])
	{
		av = &av[i + 1];
		i = 0;
		while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			i++;
		if (strcmp(av[0], "cd") == 0)
		{
			if (i != 2)
				write(2, "Bad arg cd \n",13);
			else if (chdir(av[1]) != 0)
			{
				write(2, "Can't access to : \n",20);
			}
		}
		else if (av != &av[i + 1] && (av[i] == NULL || strcmp(av[i], ";") == 0))
		{
			cmd++;
			pid = fork();
			if (pid == 0)
			{
				dup2(tmp_fd, 0);
				if (execute(i, av, env, tmp_fd))
					return (1);
			}
			else
			{
				close(tmp_fd);
				//waitpid(-1, NULL, 2);
				tmp_fd = dup(0);
			}
		}
		else if (av != &av[i + 1] && strcmp(av[i], "|") == 0)
		{
			cmd++;
			pipe(fd);
			pid = fork();
			if (pid == 0)
			{
				dup2(tmp_fd, 0);
				dup2(fd[1], 1);
				close(fd[0]);
				close(fd[1]);
				if (execute(i, av, env, tmp_fd))
					return (1);
			}
			else
			{
				close(tmp_fd);
				close(fd[1]);
				//waitpid(-1, NULL, 2);
				tmp_fd = dup(fd[0]);
				close(fd[0]);
			}
		}
	}
	close(tmp_fd);
	i = 0;
	while (i < cmd)
	{
		waitpid(-1, NULL, 2);
		i++;
	}
	return (0);
}