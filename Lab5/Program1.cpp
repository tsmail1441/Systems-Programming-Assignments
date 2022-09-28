
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


int main(){
    //PROGRAM 1
    int fd = shm_open("sharedMem",O_CREAT|O_RDWR, 0600);
    ftruncate(fd, 100);
    char* myStr = (char*) mmap(NULL, 100, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    char text[100];

    myStr[0] = '\0';
    int ra = read(STDIN_FILENO, myStr, 100);

    sleep(1);
    cin >> text;
    while (strncmp(text, "quit", 4) == 0)
    { //don't break, need to end some stuff first
        cin >> text;
    }

    // waiting and reading user inputs
    // close everything


    close(fd);
    shm_unlink("sharedMem");
    munmap(myStr, 100);


}
