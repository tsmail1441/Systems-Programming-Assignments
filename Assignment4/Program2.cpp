
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;


//  argv[1] = program name, argv[2] = #instances
int main(int argc, char *argv[]){

    // START MPI HERE:
    char* args[4];
    args[0] = new char[100]; // program name
    args[1] = new char[100]; // number of programs
    args[2] = new char[100];
    args[3] = NULL;
    
    // formatting: exe = matrix program, argument[0] = program name, argument[1] = par_id, argument[2] = par_num
    // par_id which part of the matrix is being multiplied

    strcpy(args[0], argv[1]); // get program name
    strcpy(args[2], argv[2]); // get number of programs

    //turn string into an integer
    int num_programs = atoi(argv[2]);

    // buffer to hold executable command
    char exe[100];
    // put formatted string into exe
    sprintf(exe, "./%s", argv[1]);

    //call the number of functions specified by n
    for (int i = 0; i < num_programs; i++)
    {
        sprintf(args[1], "%d", i);
        if (fork() == 0)
        {
            // call program passed in the specified args
            execv(exe, args); // if successful, terminates the caller
            // name of executable, then args passing in (par_id, and par_count)
            cout << "Could not perform execv with " << exe << endl;
            return 0; // failsave
        }
    }
    wait(0);

    delete[] args[0];
    delete[] args[1];
    delete[] args[2];
}
