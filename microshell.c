#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> //for Linux

//Структура элеменов ARGV
typedef struct s_cmd
{
	char **argv_main;
	char **argv_exec;
	int type;
}	t_cmd;

// ******************* Общие функции ******************* //

//Длина строки
int	ft_strlen(char *str)
{
	int i = 0;

	while (str[i])
		i++;
	return (i);
}

//Копирование строки в новую переменную
char	*ft_strdup(char *str)
{
	char	*ret;
	int		i;

	ret = malloc(sizeof(char) * (ft_strlen(str) + 1));
	if (ret == NULL)
		return (NULL);
	i = 0;
	while (str[i] != '\0')
	{
		ret[i] = str[i];
		i++;
	}
	ret[i] = '\0';
	return (ret);
}

//Вывод текста ошибки
void	ft_show_error(char *str)
{
	if (str != NULL)
		write(2, str, ft_strlen(str));
}

// ********* Вспомогательные функции программы ********* //

//Вывод фатальной ошибки с выходом
void	ms_fatal_error()
{
	ft_show_error("fatal: error\n");
	exit(1);
}

//Команда CD для смены каталога
void	ms_execute_cd(char **argv_exec)
{
	int	argc = 0;

	while (argv_exec[argc])
		argc++;
	if (argc != 2)
		ft_show_error("error: cd: bad arguments\n");
	else if (chdir(argv_exec[1]) != 0)
	{
		ft_show_error("error: cd: cannot change directory to ");
		ft_show_error(argv_exec[1]);
		ft_show_error("\n");
	}
}

//Движение по массиву ARGV и создание кусочка ARGV для исполнения
int	ft_parse_cmd(t_cmd *cmd, int i)
{
	int	j = 0;
	int	argv_main_i = i;

	while (cmd->argv_main[i] && strcmp(cmd->argv_main[i], ";") != 0 && strcmp(cmd->argv_main[i], "|") != 0)
		i++;
	cmd->argv_exec = (char **)malloc(sizeof(char *) * (i - argv_main_i + 1));
	if (cmd->argv_exec == NULL)
		return (0);
	while (j + argv_main_i < i)
	{
		cmd->argv_exec[j] = ft_strdup(cmd->argv_main[argv_main_i + j]);
		if (cmd->argv_exec[j] == NULL)
		{
			while (j > 0)
			{
				free(cmd->argv_exec[j - 1]);
				j--;
			}
			free(cmd->argv_exec);
			return (0);
		}
		j++;
	}
	cmd->argv_exec[j] = NULL;
	if (cmd->argv_main[i] && strcmp(cmd->argv_main[i], ";") == 0)
		cmd->type = 1;
	if (cmd->argv_main[i] && strcmp(cmd->argv_main[i], "|") == 0)
		cmd->type = 2;
	return (i);
}

// ************* Основная функция программы ************ //

int main(int argc, char **argv, char **env)
{
	int		i = 1;
	int		free_i;
	int		pipes[2];
	t_cmd	cmd;
	pid_t	pid;

	cmd.argv_main = argv;
	while (i < argc)
	{
		cmd.type = 0;
		if ((i = ft_parse_cmd(&cmd, i)) == 0)
			ms_fatal_error();
		if (cmd.type == 2 && pipe(pipes) != 0)
			ms_fatal_error();
		if (cmd.argv_exec[0] && strcmp(cmd.argv_exec[0], "cd") == 0)
			ms_execute_cd(cmd.argv_exec);
		if (cmd.argv_exec[0] && strcmp(cmd.argv_exec[0], "cd") != 0)
		{
			pid = fork();
			if (pid < 0)
				ms_fatal_error();
			if (pid == 0)
			{
				if (cmd.type == 2)
					dup2(pipes[1], 1);
				if(execve(cmd.argv_exec[0], cmd.argv_exec, env) < 0)
				{
					ft_show_error("error: cannot execute ");
					ft_show_error(cmd.argv_exec[0]);
					ft_show_error("\n");
					exit(1);
				}
				exit(0);
			}
			else
				waitpid(pid, NULL, 0);
		}
		if (cmd.type == 2)
		{
			dup2(pipes[0], 0);
			close(pipes[1]);
			close(pipes[0]);
		}
		free_i = 0;
		while (cmd.argv_exec[free_i] != NULL)
		{
			free(cmd.argv_exec[free_i]);
			free_i++;
		}
		free(cmd.argv_exec);
		i++;
	}
	return (0);
}
