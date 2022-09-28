
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#define MATRIX_DIMENSION_XY 10
#define MATRIX_SIZE MATRIX_DIMENSION_XY*MATRIX_DIMENSION_XY
using namespace std;


//SEARCH FOR TODO


//************************************************************************************************************************
// sets one element of the matrix
void set_matrix_elem(float *M,int x,int y,float f)
{
    M[x + y*MATRIX_DIMENSION_XY] = f;
}
//************************************************************************************************************************
// lets see it both are the same
int quadratic_matrix_compare(float *A,float *B)
{
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
        if(A[a +b * MATRIX_DIMENSION_XY]!=B[a +b * MATRIX_DIMENSION_XY]) 
            return 0;
    
    return 1;
}
//************************************************************************************************************************
//print a matrix
void quadratic_matrix_print(float *C)
{
        printf("\n");
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
        {
        printf("\n");
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
            printf("%.2f,",C[a + b* MATRIX_DIMENSION_XY]);
        }
    printf("\n");
}
//************************************************************************************************************************
// multiply two matrices
void quadratic_matrix_multiplication(float *A,float *B,float *C)
{
        //nullify the result matrix first
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++) // a - cols
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++) // b = rows
            C[a + b*MATRIX_DIMENSION_XY] = 0.0;
    //multiply
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++) // over all cols a
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++) // over all rows b
            for(int c = 0;c<MATRIX_DIMENSION_XY;c++) // over all rows/cols left
                {
                    C[a + b*MATRIX_DIMENSION_XY] += A[c + b*MATRIX_DIMENSION_XY] * B[a + c*MATRIX_DIMENSION_XY]; 
                }
}
//************************************************************************************************************************
void synch(int par_id,int par_count,int *ready, int ri)
{
// ALL processes get stuck here until all ARE here
    //ready[par_id]++;
    int leave = 1;
    while(leave)
    {
        leave = 0;
        for (int i = 0; i < par_count; i++)
        {
            if (ready[i] < ri)
                leave = 1;
        }
    }

}
//************************************************************************************************************************

//************************************************************************************************************************
// multiply two matrices in parallel: //TODO: quadratic_matrix_multiplication_parallel(par_id, par_count,A,B,C, ...);
//************************************************************************************************************************
// multiply two matrices in parallel: //TODO: quadratic_matrix_multiplication_parallel(par_id, par_count,A,B,C, ...);
void quadratic_matrix_multiplication_parallel(int par_id, int par_count, float *A,float *B,float *C)
{
    int rowsPerProcess = MATRIX_DIMENSION_XY/par_count;    
    int startingRow = par_id*rowsPerProcess;
    int endSegmentRow = (par_id + 1)*rowsPerProcess;

    if (par_id == par_count - 1)
    {
        endSegmentRow = MATRIX_DIMENSION_XY;
    }
        //break into rows by par_count

        //nullify the result matrix first
    for(int a = 0; a < MATRIX_DIMENSION_XY; a++)
        for(int b = startingRow; b < endSegmentRow; b++) // controls the row we go to
            C[a + b*MATRIX_DIMENSION_XY] = 0.0;

    for(int a = 0; a < MATRIX_DIMENSION_XY; a++)
        for(int b = startingRow; b < endSegmentRow; b++) // controls the row we go to
            for(int c = 0; c < MATRIX_DIMENSION_XY; c++)
                C[a + b*MATRIX_DIMENSION_XY] += A[c + b*MATRIX_DIMENSION_XY]*B[a + c*MATRIX_DIMENSION_XY]; 
            
}



int main(int argc, char *argv[]){

    int par_id = 0; // the parallel ID of this process
    int par_count = 1; // the amount of processes
    float *A,*B,*C; //matrices A,B and C
    int *ready; //needed for synch
    if(argc!=3) // if there are fewer than 3 arguments, no sharing of the process between children
        {printf("no shared\n");}
    else
        {
        par_id= atoi(argv[1]); // 
        par_count= atoi(argv[2]); // number of instances
    // strcpy(shared_mem_matrix,argv[3]);
        }
    if(par_count==1){printf("only one process\n");}

    int fd[4];
    if(par_id==0)
    {
        //TODO: init the shared memory for A,B,C, ready. shm_open with C_CREAT here! then ftruncate! then mmap
        fd[0] = shm_open("MatrixA",O_CREAT|O_RDWR, 0600);
        fd[1] = shm_open("MatrixB",O_CREAT|O_RDWR, 0600);
        fd[2] = shm_open("MatrixC",O_CREAT|O_RDWR, 0600);
        fd[3] = shm_open("synchobject",O_CREAT|O_RDWR, 0600);

        ftruncate(fd[0], MATRIX_SIZE*sizeof(int)); 
        ftruncate(fd[1], MATRIX_SIZE*sizeof(int)); 
        ftruncate(fd[2], MATRIX_SIZE*sizeof(int)); 
        ftruncate(fd[3], 10*sizeof(int)); 

        A = (float*) mmap(NULL, MATRIX_SIZE*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd[0], 0);
        B = (float*) mmap(NULL, MATRIX_SIZE*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd[1], 0);
        C = (float*) mmap(NULL, MATRIX_SIZE*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd[2], 0);
        ready = (int*) mmap(NULL, 10*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd[3], 0);

        for (int i = 0; i < par_count; i++)
        {
            ready[i] = 0;
        }
    }
    else
    {
        //TODO: init the shared memory for A,B,C, ready. shm_open withOUT C_CREAT here! NO ftruncate! but yes to mmap
        sleep(1);
        fd[0] = shm_open("MatrixA",O_CREAT|O_RDWR, 0600);
        fd[1] = shm_open("MatrixB",O_CREAT|O_RDWR, 0600);
        fd[2] = shm_open("MatrixC",O_CREAT|O_RDWR, 0600);
        fd[3] = shm_open("synchobject",O_CREAT|O_RDWR, 0600);
        
        A = (float*) mmap(NULL, MATRIX_SIZE*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd[0], 0);
        B = (float*) mmap(NULL, MATRIX_SIZE*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd[1], 0);
        C = (float*) mmap(NULL, MATRIX_SIZE*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd[2], 0);
        ready = (int*) mmap(NULL, 10*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd[3], 0);
        sleep(2); //needed for initalizing synch
    }

    
    ready[par_id] = 1;

    synch(par_id,par_count,ready, 1);
    
   // cout << "made it here\n";
    // cout << "ready array\n";
    // cout << par_count << endl;
    // for (int i = 0; i < par_count; i++){
    //     cout << " entry: "<<ready[i] << endl;
    // }

    if(par_id==0) // initialize A and B
    {
        for (int row = 0; row < MATRIX_DIMENSION_XY; row++)
        {
            for(int col = 0; col < MATRIX_DIMENSION_XY; col++)
            {
                float randNumA = (float)(rand()) / ((float) (RAND_MAX/10));
                float randNumB = (float) (rand()) / ((float) (RAND_MAX/10));
                set_matrix_elem(A, row, col, randNumA);
                set_matrix_elem(B, row, col, randNumB);
            }
        }
        
    }   
    
    ready[par_id] = 2;

    synch(par_id,par_count,ready, 2);

    //TODO: quadratic_matrix_multiplication_parallel(par_id, par_count,A,B,C, ...);
    //fork and all that shit after divvying it up between # processes

    clock_t ca, cb;

    quadratic_matrix_multiplication_parallel(par_id, par_count, A, B, C);


    ready[par_id] = 3;
    synch(par_id,par_count,ready, 3);
    
    if (par_id == 0)
    {
        cb = clock();
        cout << "The time taken for the multiplication was "    << double(cb - ca) << "usecs" << endl;
    }

    if(par_id==0)
        quadratic_matrix_print(C);
    ready[par_id] = 4;
    synch(par_id, par_count, ready, 4);

    //lets test the result:
    float M[MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY];
    quadratic_matrix_multiplication(A, B, M);

     
    // //quadratic_matrix_print(M);
    // if (par_id == 0)
    // {
    //     //quadratic_matrix_print(M);
    // }

    if (quadratic_matrix_compare(C, M))
       cout <<"full points!\n";
    else
        cout << "buuug!\n";


    close(fd[0]);
    close(fd[1]);
    close(fd[2]);
    close(fd[3]);
    shm_unlink("matrixA");
    shm_unlink("matrixB");
    shm_unlink("matrixC");
    shm_unlink("synchobject");
}
