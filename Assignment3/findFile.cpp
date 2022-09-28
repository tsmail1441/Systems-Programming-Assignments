#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <cstring>


using namespace std;

#define BUFFERSIZE 300
#define ALLDIRECTORIES 10000


int fd[2];

int *comesFromPipe = (int*)mmap(NULL, sizeof(int), 
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
int *childrenList = (int*)mmap(NULL, 10*sizeof(int), 
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
int *breakChild = (int*)mmap(NULL, sizeof(int), 
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
int *numChildren = (int*)mmap(NULL, sizeof(int), 
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

int tokenizeArgs(char* args, char **arg_tokens);
void sigHandler(int i);
int getArgLen(char **arg_tokens);
char* findFile(char* cwd, char* filename, char* retVal, int searchSub, int &foundFlag);

int main()
{
    pipe(fd); // fd[0] and fd[1]
    int parentPID = getpid();
    int t = dup(STDIN_FILENO);

    signal(SIGUSR1, sigHandler);

    *breakChild = 0;
    *numChildren = 0;

    for (int i = 0; i < 10; i++)
    {
        childrenList[i] = 0;
    }
    write(STDOUT_FILENO, "findstuff$ ", sizeof("findstuff$"));

    while(1)
    {
        
        char userInput[BUFFERSIZE] = {0};
        char userInputCpy[BUFFERSIZE] = {0};
        dup2(t, STDIN_FILENO);
        *comesFromPipe = 0;
        
        
        read(STDIN_FILENO, userInput, ALLDIRECTORIES);
        
        if(*comesFromPipe == 0)
        {
           char *argTokens[10] = {0};
           int numArgs = 0;
           int searchSub = 1;

           //tokenize (break up by space) the user input: {find} {filename} {flag}
            numArgs = tokenizeArgs(userInput, argTokens); 
         
            if(strcmp(argTokens[0],"find") == 0) 
            {
                if (fork() == 0)
                {
                    int foundFlag = 0;

                    if (*numChildren == 10)
                    {
                        cout << "Too many processes running, please wait\n";
                    }
                    else
                    {
                        int listCounter = 0;
                        while (childrenList[listCounter] != 0) // find place for child
                        {
                            listCounter++;
                        }
                        childrenList[listCounter] = getpid();
                        (*numChildren)++;

                        char RetVal[ALLDIRECTORIES]; // retvalue[100 rows][1000 columns]
                       

                        if (numArgs == 2)
                        {
                            //current directory
                            char buffer[BUFFERSIZE];
                            getcwd(buffer, BUFFERSIZE);
                            searchSub = 0;
                            char* result = findFile(buffer, argTokens[1], RetVal, searchSub, foundFlag); // run with sudo to get access
                            searchSub = 1;                  
                        }
                        // if flag is f
                        else if(strcmp(argTokens[2],"f\n") == 0) 
                        {                        
                            //find the root directory and subdirectories
                            char buffer[BUFFERSIZE];
                            getcwd(buffer, BUFFERSIZE);
                            strcpy(buffer, "/home");
                            searchSub = 1;
                            char* result = findFile(buffer, argTokens[1], RetVal, searchSub, foundFlag);
                        }
                        else if(strcmp(argTokens[2],"s\n") == 0) // if flag is s: 
                        {
                            //current directory and subdirectories
                            char buffer[BUFFERSIZE];
                            getcwd(buffer, BUFFERSIZE);
                            searchSub = 1;
                            char* result = findFile(buffer, argTokens[1], RetVal, searchSub, foundFlag); // dummyRetVal is the directory where found
                        }
                        if (foundFlag == 1)
                        {
                            kill(parentPID, SIGUSR1); // overwrite STDIN with pipe output in parent (parent can print)                     
                            write(fd[1], RetVal, 10000);  
                        }
                        else 
                        {
                            kill(parentPID, SIGUSR1); // overwrite STDIN with pipe output in parent (parent can print)
                            char text[100] = "Nothing found";
                            write(fd[1], text, 90); 
                        } 
                        
                    }
                    exit(0);
                } // endfork
            }
            else if(strncmp(argTokens[0],"quit", 4) == 0)
            {
                *breakChild = 1;
                int status = 0;

                for (int i = 0; i < 10; i++)
                {
                    if (childrenList[i] != 0)
                    {
                        kill(childrenList[i], SIGKILL);
                        usleep(100);
                        if (waitpid(childrenList[i], &status, WNOHANG) != 0)
                        {
                            childrenList[i] = 0;
                        }
                    }
                }

                close(fd[0]);
                close(fd[1]);
                break;
            }

            else 
            {
                cout << "Not a valid input. Input should be formatted as follows:\n"
                    << "{find or quit} {filename} {optional flag: s or f}\n";
                write(STDOUT_FILENO, "findstuff$ ", sizeof("findstuff$"));
            }

        }
        else
        {
            cout << userInput << endl;
            write(STDOUT_FILENO, "findstuff$ ", sizeof("findstuff$"));
            (*numChildren)--;
        }

        int status = 0; 
        for (int i = 0; i < 10; i++)
        {
            if (childrenList[i] != 0)
            {
                if (waitpid(childrenList[i], &status, WNOHANG) != 0)
                {
                    childrenList[i] = 0;
                }
                 
            } 
        }
    }

    munmap(comesFromPipe, sizeof(int));
    munmap(childrenList, 10* sizeof(int));
    munmap(breakChild,  sizeof(int));
    munmap(numChildren,  sizeof(int));

    return 0;
}



// find a file in a directory and subdirctory
char* findFile(char* cwd, char* filename, char* retVal, int searchSub, int &foundFlag) 
{ 
    DIR *dir = opendir(cwd);
    struct dirent *entry; 

    char directories[100] = {0};
    strcpy(directories, filename);
  
    if (*breakChild == 1)
    {
        exit(0);
    }
    else
    {
        while((entry = readdir(dir) )!= NULL) 
        {
            if(strcmp(entry -> d_name, filename) == 0) 
            { 
                foundFlag = 1;
                strcpy(directories, filename);
                strcat(directories, " ");
                strcat(directories, cwd);
                strcat(directories, "\n");
                strcat(retVal, directories);
                closedir(dir);
                
                return retVal;
                

            }
            if((entry -> d_type == DT_DIR) && (strcmp(entry -> d_name, "..")!= 0) && (strcmp(entry -> d_name,".") != 0)
                                            && (searchSub == 1))
            {
                char newcwd[1000];
                strcpy(newcwd, cwd); // copy cwd into new
                strcat(newcwd, "/"); // concatenate a "/"
                strcat(newcwd, entry -> d_name);
                findFile(newcwd, filename, retVal, searchSub, foundFlag); 
            }
        
            
        }
        if (searchSub == 0)
        { // file not found, end of while loop
            strcpy(retVal, "Nothing found\n");
        }
        closedir(dir);
        return retVal;
    }
}


void sigHandler(int i)
{
    dup2(fd[0], STDIN_FILENO); // STDIN_FILENO is now the output of the pipe
    *comesFromPipe = 1;
    return;
}


int tokenizeArgs(char* args, char **arg_tokens)
{
    int num_args = 0;
    char *token = strtok(args, " ");

    while (token != NULL)
    {
        arg_tokens[num_args] = token;
        token = strtok(NULL, " ");
        num_args++;
    }
    return num_args;
}


int getArgLen(char **arg_tokens)
{
    int i = 0;
    
    while (arg_tokens[i])
    {
        i++;
    }
    return i;
}
