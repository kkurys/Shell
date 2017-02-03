#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include "fun.h"

void Execute(Command *command)
{
	int savestdin=dup(0), savestdout=dup(1);
	int fdin, fdout, i;
	pid_t pid;
	for (i = 0; i<command->numOfSimpleCommands; i++)
	{
		dup2(fdin, 0);
		close(fdin);
		if (i == command->numOfSimpleCommands - 1)
		{
			if (command->outFile)
			{
				fdout = open(command->outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
			}
			else 
			{
				fdout = dup(savestdout);
			}
		}
		else 
		{
			int fdpipe[2];
			pipe(fdpipe);
			fdout=fdpipe[1];
			fdin=fdpipe[0];
		}
		dup2(fdout, 1);
		close(fdout);
		pid = fork();
		if (pid == 0)
		{
			char **args = command->simpleCommands[i]->args;
			execvp(args[0], args);
			perror("execvp");
			exit(-1);
		}
		else
		{
			if (!command->background)
			{
				wait(NULL);
			}
		}
	}
	dup2(savestdin, 0);
	dup2(savestdout, 1);
	close(savestdin);
	close(savestdout);
}

void InsertArg(SimpleCommand *sc, char *arg)
{
	if (sc->numOfArgs >= sc->availableArgs)
	{	
		sc->availableArgs += sc->availableArgs;
		char **tmp = realloc(sc->args, (sc->availableArgs)*sizeof(char*));
		if (!tmp)
		{
			perror("Args memory reallocation failed!");
			exit(-1);
		}
		else 
		{
			sc->args = tmp;
		}
	}
	sc->args[sc->numOfArgs] = arg;
	sc->numOfArgs++;
}

void InsertSimpleCommand(Command *command, SimpleCommand *sc)
{
	if (command->numOfSimpleCommands >= command->availableSimpleCommands)
	{
		command->availableSimpleCommands += command->availableSimpleCommands;
		SimpleCommand **tmp = realloc(command->simpleCommands, command->availableSimpleCommands * sizeof(SimpleCommand*));
		if (!tmp)
		{
			perror("SimpleCommand mem reallocation failed!");
			exit(-1);
		}
		else 
		{
			command->simpleCommands = tmp;
		}
	}
	command->numOfSimpleCommands ++;
	command->simpleCommands[command->numOfSimpleCommands - 1] = sc;
}

SimpleCommand *InitSimpleCommand()
{
	SimpleCommand *sc = malloc(sizeof(SimpleCommand));

	sc->availableArgs = 1;
	sc->numOfArgs = 0;
	sc->args = malloc(sc->availableArgs * sizeof(char*));

	return sc;
}

Command *InitCommand()
{
	Command *command = malloc(sizeof(Command));

	command->availableSimpleCommands = 1;
	command->numOfSimpleCommands = 0;
	command->background = 0;
	command->simpleCommands = malloc(sizeof(SimpleCommand*));

	return command;
}

Command *ParseLine(char *line)
{	
	Command *command = InitCommand();
	SimpleCommand *sc = InitSimpleCommand();
	InsertSimpleCommand(command, sc);
	int pos = 0, buffPos = 0, outFileExpected = 0;
	char *token;
	
	token = strtok(line, " \t\r\n\a");
	while (token != NULL)
	{
		if (strncmp(token, "|", 1) == 0)
		{
			InsertArg(sc, NULL);
			sc = InitSimpleCommand();
			InsertSimpleCommand(command, sc);
		}
		else if (strncmp(token, ">>", 2) == 0)
		{
			InsertArg(sc, NULL);
			outFileExpected = 1;
		}
		else if (strncmp(token, "&", 2) == 0)
		{
			command->background = 1;
		}
		else if (outFileExpected)
		{
			command->outFile = token;
		}
		else 
		{
			InsertArg(sc, token);
		}
		token = strtok(NULL, " \t\r\n\a");		
	}
	InsertArg(sc, NULL);
	return command;
}

void Prompt()
{
	char *buff = malloc(128);

	cwd = getcwd(buff, PATH_MAX +1);
	printf("%s > " , cwd); 
	free(buff);
}

void PrintCommands(Command *command)
{
	int i = 0, j = 0;

	for (i; i<command->numOfSimpleCommands; i++)
	{
		printf("%d: ", i);
		for (j = 0; j<command->simpleCommands[i]->numOfArgs; j++)
		{
			printf("%s ", command->simpleCommands[i]->args[j]);			
		}
		printf("\n");
	}

}	

char *ReadLine()
{
	int currentPosition = 0, buffSize = 128;
	char *buffer = malloc(buffSize), c;

	if (buffer == NULL)
	{
		printf("Buffer: memory allocation failed!\n");
		exit(-1);
	}
	while (1)
	{
		c = getchar();
		if (c == EOF || c == '\n')
		{
			if (c == EOF)
			{
				status = 0;
			}
			buffer[currentPosition] = '\0';
			return buffer;
		}
		else 
		{
			buffer[currentPosition] = c;
		}
		currentPosition++;
		if (currentPosition >= buffSize)
		{
			buffSize += buffSize;
			char *tmp;

			if(tmp = realloc(buffer, buffSize))
			{
				buffer = tmp;
			}
			else 
			{
				printf("Buffer: memory  reallocation failed!\n");
				exit(-1);
			}
		}
	}
}
 
void WriteHistoryToFile(char* filePath)
{
	int fd = open (filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);  
	HistoryList *currNode = head;

	if (fd == -1)
	{
	        printf("Write: cannot open file in %s\n", filePath);
	        return;
	}

	while(currNode != NULL)
    	{
        	char* tmpLine = currNode->cmdLine;
        	size_t length = strlen (tmpLine);
        	write (fd, tmpLine, length);
        	write (fd, "\n", 1);
        	currNode = currNode->next;
    	}
    	close (fd);
}
 
void AddToHistory(char* line, char* filePath)
{
	HistoryList *newNode = malloc(sizeof(HistoryList));
	char* newLine = malloc(strlen(line));

	strcpy(newLine, line);
	newNode->cmdLine = newLine;
	newNode->next = NULL;
 
	if (head == NULL)
	{
		head = newNode;
		tail = newNode;
		historyCount++;
	}
	else if (historyCount < 20)
	{
		tail->next = newNode;
		tail = tail->next;
		historyCount++;
	}
	else
	{
		HistoryList *hold = head;
	        tail->next = newNode;
	        tail = tail->next;
	        head = head->next;
        	free(hold);
    }
    WriteHistoryToFile(filePath);
}
 
char *GetHomeDirectory(char* fileName)
{
	char *homeDir = getenv("HOME");
	char *logDir = malloc(strlen(homeDir) + strlen(fileName) + 2);
   
	strcpy(logDir, getenv("HOME"));
	strcat(logDir, "/");
	strcat(logDir, fileName);
    
	return logDir;
}
void Handle_signal(int signo)
{
	printf("\n%s > ", cwd);
	fflush(stdout);
}
void Show_history(int signo)
{
	HistoryList *tmp = head;
	int no = 1;

	printf("\nHistory: \n");
	while (tmp != NULL)
	{
		printf("%d. %s\n", no, tmp->cmdLine);
		no++;
		tmp = tmp->next;
	}
	Handle_signal(signo);
}


int BuiltInFunctions(Command *command)
{
	int i = 0;

	for (i; i < command->numOfSimpleCommands; i++)
	{
		if (strncmp(command->simpleCommands[i]->args[0], "cd", 2) == 0)
		{
			if (command->simpleCommands[i]->args[1] == NULL)
			{
				fprintf(stderr, "lack of cd argument\n");
			}
			else 
			{
				if (chdir(command->simpleCommands[i]->args[1]) != 0)
				{
					perror("chdir error");
				}
			}
			return 1;
		}
		else if (strncmp(command->simpleCommands[i]->args[0], "help", 4) == 0)
		{
			printf("Built-in functions: \n");
			printf("cd help exit\n");
			return 1;
		}
		else if (strncmp(command->simpleCommands[i]->args[0], "exit", 4) == 0)
		{
			exit(0);
		}	
	}
	return 0;
}

