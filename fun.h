/* types */
typedef void (*sighandler_t)(int);

typedef struct SimpleCommand_Struct
{
	int availableArgs, numOfArgs;
	char **args;
}SimpleCommand;

typedef struct Command_Struct
{
	int availableSimpleCommands, numOfSimpleCommands;
	SimpleCommand **simpleCommands;
	char *outFile;
	int background;

}Command;

typedef struct lista
{
    char* cmdLine;
    struct lista *next;
} HistoryList;

/* void functions */
void Handle_signal(int signo);
void Execute(Command *command);
void InsertArg(SimpleCommand *sc, char *arg);
void InsertSimpleCommand(Command *command, SimpleCommand *sc);
void Prompt();
void PrintCommands(Command *command);
void WriteHistoryToFile(char* filePath);
void AddToHistory(char* line, char* filePath);
void Show_history(int signo);
void FreeMemory(Command *command);

/* string functions */
char *GetHomeDirectory(char* fileName);
char *ReadLine();

/* other functions */
int BuiltInFunctions(Command *command);
SimpleCommand *InitSimpleCommand();
Command *ParseLine(char *line);
Command *InitCommand();

/* global variables */
char *cwd;
int status, historyCount;
HistoryList *head, *tail;

