#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "stackNode.h"
#include "stack.h"
#include "stack.c"

#define BUF_LIM 1024
#define MAX 100

void shLoop(void);
char* shRead(void);
char** shParse(int* argc, char* line);
int shExec(int argc,char** args);
int threadSpinner(char* cmd);
void readHist();

/*
In main it will see if there are two arguments if there is 
only one then it will call the shLoop() and be an intereactive shell.
If there are 2 arguments it will open the file specified by the 
command line and then if it is a real file it will say batch mode,
list the file given, and keep reading in the commands and printing
them and executing them. If it does not exist it will give a message
saying the file does not exist
*/

int main(int argc, char* argv[])
{
	
	char **shArgs;
        int shArgc = 0;
	int shStatus = 0;
	FILE *fp;
	fp = fopen("history","w"); // Create/Clear out the file from previous history (if any)
	fclose(fp);
	
	if(argc > 2) // handle argument errors
	{
		printf("Too Many Arguments\nUse: shell [file]\n");
	}
	else if(argc == 2) // If a file is passed, read in the input
	{
        	char buff[255];
		fp = fopen(argv[1], "r"); //open batch file for reading
	

		if(fp)
		{
        		printf("*******batch mode*******\n");
			printf("--> file: %s\n", argv[1]);
			while(1)
			{
				shArgc=0;
				if(fgets(buff, 150, fp) == NULL) break; // read in each line
				{	
					printf("batch line> %s\n", buff);
					shArgs = shParse(&shArgc, buff); //send the input for parsing
                			shStatus = shExec(shArgc, shArgs); //send parsed input to shExec
					
				}
			}//end while
	        	fclose(fp);
		}//end if
		else
			printf("File does not exist\n");
	}//end if
	else	
		shLoop();
		
	return 0;
	
}


/*
Function: shLoop()
Description: Main loop for the shell. we call a do-while loop
which prints out our prompt and then executes
shRead(), shParse(), and shExec() until our
shStatus function is negative. If the status is negative, we exit.
*/
void shLoop(void)
{
	int shStatus = 0;
	char **shArgs;
	int shArgc = 0;
	char *shInput;
	
	do
        {	shArgc = 0; // set/reset arg count to 0
		shInput = ""; // sanitize input variable
                printf("#>> "); 
		shInput = shRead(); //read in from shell command line
		
		//If we read in "quit" or CTRL-D, quit
		if((strcmp(shInput, "quit") == 0) || (shInput[0]==EOF))
		{
			shStatus = -1;
			break;
			
		}
		else if(shInput[0] == '\0') // If we read a \n, continue
		{
			continue;
		}
		shArgs = shParse(&shArgc,shInput); // send intput for prsing
		shStatus = shExec(shArgc,shArgs); // send parsed input to shExec for execution
		
		
		
        }while(shStatus >= 0); //continue while status is positive
}

/*
Function: shRead()
Description: This function reads in characters from stdin to a buffer.
our buffer size is determiend by a global variable BUF_LIM. We initialize
a c string, buffer, to be the size of BUF_LIM. using getchar() we read
characters into buffer while keeping track of our place using an iterator
called bufPos. As we add chars, we increment bufPos. Input is contolled
by an if statement checking if an EOF char or newline is read.
If a newline or EOF is read, we terminate the c-string with a null and
return the buffer. Additionally if we exceed our buffer length, we 
realloc enough space.
*/

char* shRead(void)
{
	FILE *fp; // File pointer for history
	int bufSize = BUF_LIM; // size of buffer
	int bufPos=0; // position in the buffer
	int i; 
	char *buffer = malloc(sizeof(char) * bufSize); // buffer
	char c;

	//Handle allocation errors
	if(!buffer)
	{
		fprintf(stderr, "Could Not Allocate to buffer");
		return 0;
	}
	
	while(1) 
	{
		c = getchar(); //read char by char
		
		//Handle CTRL-D and "newline" input
		if( c == EOF || c == '\n' )
		{
			if(c == EOF) // if CTRL-D entered, 
			{	
				// This does not work as expected. 
				// If you CTRL-D in the middle of input
				// you have to CTRL-D again before the
				// program will quit. if CTRL-D is the only
				// thing entered, works fine
				
				buffer[0] = EOF; 
				return buffer;
			}
			else
			{
				buffer[bufPos] = '\0'; // end string
				
				//Print command line to file
				fp = fopen("history", "a"); 
        			fprintf(fp, "%s\n", buffer);
        			fclose(fp);

				return buffer;
			}
		}
		else
			buffer[bufPos]=c; // set buffer at bufPos to be char enetered
		
		bufPos++; // increment bufPos
	
		// if we write more than the buffer can hold, reallocate
		if(bufPos >= bufSize)
		{
			bufSize += BUF_LIM;
			buffer = realloc(buffer, bufSize);
			
			// handle memory allocation error
			if(!buffer)
			{
                		fprintf(stderr, "Could Not re-allocate to buffer");
                		return 0;
        		}
		}
	} 
	
}

/*
Function: shParse()
Description: This function tokenizes the input from shRead() and stores
it in an array of c-strings. The structure for this function is very
similar to shRead().We allocate memory to the array of c-strings based on
the value of tokBuffSize. Each token gets stroed and we store the number
of arguments in the variable argCount. Similarly to shRead(), if we run
out of space in our c-string array, we allocate more using realloc.
When we are done we retun the array of c-strings
*/

char** shParse(int* argc,char* line)
{
	int tokBufSize = 64;
	int tokBufPos = 0;
	int argCount = *argc; 

	char** tokens = malloc(tokBufSize * sizeof(char*));
	char* token;
	
	// handle memory error
	if(!tokens)
	{
		fprintf(stderr, "Could Not allocate to buffer");
                return 0;
	}
	
	token = strtok(line, " \t\n\v\f\r"); // Tokenize input by whitespace
	
	
	while(token != NULL)
	{
		
		tokens[tokBufPos] = token; // fill up the token array with tokenized input
		tokBufPos++; // keep track of our place in the tokens array
		
		if(tokBufPos >= 64)
		{
			tokBufSize += 64;
			tokens = realloc(tokens, tokBufSize * sizeof(char*));

			if(!tokens)
        		{
                		fprintf(stderr, "Could Not re-allocate to buffer");
                		return 0;
        		}
		}
		token = strtok(NULL, " \t\n\v\f\r"); //get next token
		if(token != " ");
			argCount++; // cound how many arguments we tokenized
	}
	
	*argc=argCount; //set argument count
	
	return tokens;

}

/*
Function: shExec()
Description:Similar to regular command line
arguments, I've implemented our arguments and argc. argc is the number
of arguments. and the actual arguments are in the variable args.
In order to make it easier to work with, I've implemented a stack structure.
The stack will hold each parameter in order. So to get the next parameter,
just pop(stack). If you encounter a semi-colon, a child procress 
gets created to execute the command.
Special handling of "quit" "cd" and semi-colons is done as well. 
cd will cause chdir() to be executed. quit will cause the program to quit
after all other commands are executed, and lone semi-colons are disregarded
while a semi-colon at the end of a string caues a child process to be created.
*/
int shExec(int argc, char** args)
{
	int s, i, quit;
	int bufSize = BUF_LIM;
	char* building, *tmp;
	building = malloc(bufSize * sizeof(char*)); // command being built
	tmp = malloc(bufSize * sizeof(char*)); // will hold each tokenized str
	myStack *stack = newStack(); //initialize new stack
	
	if(args[0] == NULL)
        {
                return 1;
        }
	
	for(i = argc-1; i >= 0; i--)
	{	
		push(stack, args[i]); // Push tokenized strings onto stack
				      // in reverse so that commands happen in order 
	}
	
	for(i = 0; i < argc; i++)
	{
		tmp=pop(stack); // Get next argument from stack
		
		if(strcmp(tmp, "cd") == 0) // handle cd command
		{
			tmp=pop(stack); //get directory
			chdir(tmp); // change dir to directory
			continue;
		}
		if((strcmp(tmp, "history") == 0) || (strcmp(tmp, "history;") == 0)) // handle "history" command
		{
			readHist();
			continue;
		}
		if((strcmp(tmp, "quit") == 0) || (strcmp(tmp, "quit;") == 0)) // handle "quit" command
		{
			quit = -1;
			continue;
		}
		if(tmp[0] == ';') // Handle single semi-colon
		{	
			if((strcmp(building, "")) != 0)
			{	
				s = threadSpinner(building);
				building = malloc(bufSize * sizeof(char*));
			}
			continue;
		}
		else if(tmp[strlen(tmp)] == ';') // handle commands separated by semi-colon
		{
			tmp[strlen(tmp)] = '\0'; // end first command
			strcat(building, tmp); // build command
			s = threadSpinner(building); // start new child process
			continue; // continue processing commands
		}
		else // Keep building string if simple input
		{
			strcat(tmp, " ");
			strcat(building, tmp);
		}
	}
	s = threadSpinner(building); // start new child process
	
	// If we triggered "quit" wait to quit til all processes finish
	if(quit == -1)
	{
		return quit; // return -1, which quits
	}
	else
		return s; // return status from system call
}

/*
Function: threadSpinner()
Description:
Takes a command as input, and forks a child to execute it concurrently
with other children being made from the shExec() command
*/
int threadSpinner(char* cmd)
{
	int pid;
		
	pid = fork(); // start new child
	
	if(pid == -1) // handle fork error
	{
		printf("forking error");
		exit(-1);
	}
	if(pid == 0) // child executes command
	{
		system(cmd);
		exit(0);

	}
	else // parent waits to return
	{
		wait(NULL);
	}

	return 0;

}

/*
Function: readHist()
Description:
Reads in the history from the history file and prints it to the screen.
*/
void readHist()
{
	FILE *fp; 
	char *line = NULL;
	char buf[1024];
	size_t length = 0;
	ssize_t num_read;
	fp = fopen("history", "r");
	while((num_read=getline(&line, &length, fp)) != -1)
	{
		strcpy(buf, line);
		printf("%s\n",buf);

	}
	fclose(fp);

}
