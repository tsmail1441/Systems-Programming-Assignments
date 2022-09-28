
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#define MATRIX_DIMENSION_XY 10

using namespace std;


int main(){

    //sleep(1); // might not need
    //PROGRAM 2 at the start of the project
    int fd = shm_open("sharedMem", O_RDWR, 0600);
    char* myStr = (char*) mmap(NULL, 100, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if (fd != -1){
        cout << "Link successful!" << endl;
    }


    // print program linked successfully
    while(myStr[0] == '\0'){}
    
    cout << myStr;


    // close everything
    close(fd);
    shm_unlink("sharedMem");
    munmap(myStr, 100);


}
