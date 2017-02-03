#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include "fun.h"

/* CREATED BY KAMIL KURYŚ / MACIEJ OWERCZUK /
   BIAŁYSTOK 2016 */

int main(int argc, char *argv[])
{
	status = 1;
	head = NULL;
	tail = NULL;
	historyCount = 0;

	signal(SIGINT, SIG_IGN);
	signal(SIGINT, Handle_signal);
	signal(SIGQUIT, Show_history);

	char buff[PATH_MAX+1], *home, *line;
	int i = 0, savestdin=dup(0);
	home = GetHomeDirectory("log.txt");
	Command *command;

	/* status:
		0 - exit
		1 - work
		2 - read script */

	if (argc > 1)
	{
		int fd = open(argv[1], O_RDONLY);
		dup2(fd, 0);
		line = ReadLine();
		status = 2;
	}

	while(status)
	{
		if (status != 2)		
	 		Prompt();

		line = ReadLine();
		AddToHistory(line, home);
		if (strncmp(line, "", 1) != 0)
		{
			command = ParseLine(line);;
			if (!BuiltInFunctions(command))
			{
				Execute(command);
			}
		}
	}	
	dup2(savestdin, 0);
	printf("\n"); 
	return 0;
}
