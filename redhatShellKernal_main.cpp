
// Libraires
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
#include <cstring>
#include <string>


// Macros
#define TRUE 1
#define LINE_MAX 100 // Max input length
#define ARGS_MAX 6   // How many inputs it will take


// Function Prototypes
void getInput(char[], char*[]);
void isOutputFile(char*[], char[], int&, int&);
void isInputFile(char*[], char[], int&, int&);
void checkTypeIO(char[], int&);


// Call to Main
int main(int argc, char* argv[])
{
    //// Variables
    // Command Line Input
    char terminalCommand[LINE_MAX];  

    // Command Line Arguments Depending on if piping or no         
    char* commandLineArgs_NoPipe[ARGS_MAX];
    char* commandLineArgs_Pipe[ARGS_MAX];

    // Misc
    char fileName[LINE_MAX];
    int pipeInfo[2];

    // Welcome Message
    cout << "{                         }" << endl;
    cout << "{ Welcome to RAT OS Shell }" << endl;
    cout << "{                         }" << endl;


    //// Running Indefinitely on the Kernal
	while (TRUE)
	{
        // Variables to Initialize
		int i = 0, j = 0;           // indicies                  
		int IOflag = 0;             // flag       
		int pipeCount = 0;          // counter
		
        // Command Line Input Message
        getInput(terminalCommand, commandLineArgs_NoPipe);

        // Command Input Loop
		while (commandLineArgs_NoPipe[i] != NULL)
		{
            // Command Output File Condition
			if (commandLineArgs_NoPipe[i][0] == '>')      
			{
                isOutputFile(commandLineArgs_NoPipe, fileName, i, IOflag);
				break;
			}

            // Command Input File Condition
			if (commandLineArgs_NoPipe[i][0] == '<')  
			{
                isInputFile(commandLineArgs_NoPipe, fileName, i, IOflag);
				break;
			}

            // Pipe Check Condition
			if (commandLineArgs_NoPipe[i][0] == '|')  
			{
				pipeCount = 1;                               
				commandLineArgs_NoPipe[i] = NULL;
				commandLineArgs_Pipe[0] = strtok(NULL, " ");

                // 2nd Command Input Loop
				while (commandLineArgs_Pipe[j] != NULL) 
				{
                    // Pipe Output File Condition
					if (commandLineArgs_Pipe[j][0] == '>')   
					{
                        isOutputFile(commandLineArgs_NoPipe, fileName, j, IOflag);
						break;
					}

                    // Pipe Input File Condition
					if (commandLineArgs_Pipe[j][0] == '<') 
                    {
                        isInputFile(commandLineArgs_NoPipe, fileName, j, IOflag);
						break;
					}

                    // When no Input/Output File in Pipe
					commandLineArgs_Pipe[++j] = strtok(NULL, " ");
				}

				break;
			}
			
			// When no Input/Output File or Pipe in Command
            commandLineArgs_NoPipe[++i] = strtok(NULL, " ");
		}


        //// Pipe Conditions
        // If No Pipe Used
		if (pipeCount == 0)              
		{
			int pid = fork();

            // Child Process
			if (pid == 0)             
			{
                checkTypeIO(fileName, IOflag);
                execvp(commandLineArgs_NoPipe[0], commandLineArgs_NoPipe);
			}

            // Parent Process 
			wait(0);
		}

        // If Piping Used 
	    if (pipeCount == 1)              
		{
            // Pipe Ain't Working Guard Clause
			if (pipe(pipeInfo) == -1)
			{
				exit(0);
			}

            // Fork #1
			int pid1 = fork();

            // Child #1.1
			if (pid1 == 0)               
			{
				dup2(pipeInfo[1], 1);
				close(pipeInfo[0]);
				close(pipeInfo[1]);
				execvp(commandLineArgs_NoPipe[0], commandLineArgs_NoPipe);

				exit(0);
			}

            // Parent #1
            if (pid1 != 0)
            {
                // 2nd Forking
                int pid2 = fork();

                // Child #1.2
                if (pid2 == 0)        
                {
                    dup2(pipeInfo[0], 0);
                    close(pipeInfo[0]);
                    close(pipeInfo[1]);
                    checkTypeIO(fileName, IOflag);
                    execvp(commandLineArgs_Pipe[0], commandLineArgs_Pipe);
                }
                
                // Parent #1
                close(pipeInfo[0]);
                close(pipeInfo[1]);
                wait(0);
                wait(0);
            }
		}
	}

	return 0;
}


// Function Definitions
void getInput(char str[], char* args[])
{
    cout << "[RAT OS Shell]{ ";
    fgets(str, LINE_MAX, stdin);        
    str[strlen(str) - 1] = '\0';                                          
    args[0] = strtok(str, " ");

    return;
}


void isOutputFile(char* args[], char fileName[], int& index, int& IOflag)
{
    IOflag = 1;
    args[index] = NULL;
    char* temp = strtok(NULL, ">");
    sprintf(fileName, "%s", temp);

    return;
}


void isInputFile(char* args[], char fileName[], int& index, int& IOflag)
{
    IOflag = 2;
    args[index] = NULL;
    char* temp = strtok(NULL, "<");
    sprintf(fileName, "%s", temp);

    return;
}


void checkTypeIO(char fileName[], int& IOflag)
{
    /*  File Property Value:
        O_TRUNC - opens file as though it had been created earlier but not written to
        O_RDWR  - opens file for read/write
        O_CREAT - makes file if there isn't one in the current directory
    */

    // Open File w/ Properties
    int temp = open(fileName, O_CREAT | O_RDWR | O_TRUNC, 0644);

    // Input/Output Handling
    switch (IOflag)
    {
        // Output Condition
        case 1:
            dup2(temp, 1);
            close(temp);
            break;

        // Input Condition
        case 2:
            dup2(temp, 2); 
            close(temp);
            break;

        default:
            break;
    }

    return;
}

