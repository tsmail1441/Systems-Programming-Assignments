#include <iostream>
# include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <string>
#include <unistd.h>

using namespace std;

void sigNoEntryHandler(int u);

int pi[2];


int main() {

    pipe(pi);

    int parentPID = getpid();
    // need shared memory between child and parent
    int* childPID =  (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, 
            MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    int* parentEntered = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, 
            MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *parentEntered = 0;
    int* pipeFull = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, 
            MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *pipeFull = 0;

    int* breakChild = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, 
            MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *breakChild = 0;

    int t = dup(STDIN_FILENO);
 
    signal(SIGUSR1, sigNoEntryHandler);

    if (fork() == 0) { //child
        *childPID = getpid();
        for (;;) {

            *parentEntered = 0;
            sleep(10);

            if (*breakChild == 1) {
                exit(0);
            }
            else if (*parentEntered == 0) {
             
                kill(parentPID, SIGUSR1); // overwrite stdin with pipe output'

                char text[100] = "No activity detected!"; // length wo null: 21
                write(pi[1], text, 90); 
                *pipeFull = 1;
                
            }
        }
    } // parent
    
    int kidsPID = *childPID;

    for (;;) {
        
        usleep(100000);
        if (*pipeFull == 1) {
            char text[90];
      
            read(STDIN_FILENO,text,90);
            text[strlen("No activity detected!")] = 0; 
            
            *pipeFull = 0;
        }
        else {
            
            dup2(t, STDIN_FILENO); //restore STDIN for letting parent read from keyboard
      
            //read from the keyboard
            char text[100] = {0};
            int readBytes = read(STDIN_FILENO, text, 30); // going to be stuck here until I take keyboard access
            text[strlen(text)] = 0;

            if (strcmp(text, "quit\n") == 0) {
                *breakChild = 1;
                wait(0);
                close(pi[0]);
                close(pi[1]);
                break;
            }
            else if (text != 0) {
                *parentEntered = 1;

                if (strcmp(text, "No activity detected!") != 0) {
                    cout << "!";
                    text[strlen(text) - 1] = '!';
                }
                cout << text << endl;
            }
        }
    }
   
  
    munmap(childPID, sizeof(int));
    munmap(parentEntered, sizeof(int));
    munmap(pipeFull, sizeof(int));
    munmap(breakChild, sizeof(int));
    return 0;
}

void sigNoEntryHandler(int u) {
     
    dup2(pi[0], STDIN_FILENO);
    
}
