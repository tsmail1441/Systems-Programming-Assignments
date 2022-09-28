
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
#define BITSPERPIXEL 24
#define BYTESPERPIXEL (BITSPERPIXEL/8)
#define MATRIX_DIMENSION_XY 300
#define IMAGEBYTES (MATRIX_DIMENSION_XY*MATRIX_DIMENSION_XY*BYTESPERPIXEL)
#define MATRIX_SIZE (MATRIX_DIMENSION_XY*MATRIX_DIMENSION_XY)
#define BYTEWIDTH (MATRIX_DIMENSION_XY*BYTESPERPIXEL)
using namespace std;



typedef unsigned short WORD; 
typedef unsigned int DWORD; 
typedef unsigned int LONG;
typedef unsigned char BYTE; 


struct tagBITMAPFILEHEADER 
    { 
    WORD bfType;  //specifies the file type 
    DWORD bfSize;  //specifies the size in bytes of the bitmap file 
    WORD bfReserved1;  //reserved; must be 0 
    WORD bfReserved2;  //reserved; must be 0 
    DWORD bfOffBits;  //species the offset in bytes from the bitmapfileheader to 
    //the bitmap bits 
 }; 

struct tagBITMAPINFOHEADER 
 { 
    DWORD biSize;  //specifies the number of bytes required by the struct 
    LONG biWidth;  //specifies width in pixels 
    LONG biHeight;  //species height in pixels 
    WORD biPlanes; //specifies the number of color planes, must be 1 
    WORD biBitCount; //specifies the number of bit per pixel 
    DWORD biCompression;//spcifies the type of compression 
    DWORD biSizeImage;  //size of image in bytes 
    LONG biXPelsPerMeter;  //number of pixels per meter in x axis 
    LONG biYPelsPerMeter;  //number of pixels per meter in y axis 
    DWORD biClrUsed;  //number of colors used by th ebitmap 
    DWORD biClrImportant;  //number of colors that are important 
 };

void readBMPData(BYTE *&pixelTable, FILE *& pictureFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo);

void writeBMPData1(FILE *&newFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo);

void matrixMultiplication(BYTE *pixelData1, BYTE *pixelData2, BYTE *resultData, int par_id, int par_count);


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



int main(int argc, char* argv[]) {

    char localImageFile1[20] = {0};
    strcpy(localImageFile1, "f1.bmp");

    char localImageFile2[20] = {0};
    strcpy(localImageFile2, "f2.bmp");

    char localOutputFile[20] = {0};
    strcpy(localOutputFile, "OutputImage.bmp");

    int par_id = 0; // the parallel ID of this process
    int par_count = 1; // the amount of processes
    int *ready; //needed for synch

    if(argc!=3) // if there are fewer than 3 arguments, no sharing of the process between children
        {printf("no shared\n");}
    else
    {
        par_id= atoi(argv[1]); // instance
        par_count= atoi(argv[2]); // number of instances
    // strcpy(shared_mem_matrix,argv[3]);
    }

    if(par_count==1){printf("only one process\n");}

    int fd[4];

    tagBITMAPFILEHEADER pictureHeader1, pictureHeader2;
    tagBITMAPINFOHEADER pictureInfo1, pictureInfo2;


     // read the bmp header info into the struct variables
    BYTE *pixelTable1, *pixelTable2, *resultTable;

    if(par_id==0)
    {
        //TODO: init the shared memory for A,B,C, ready. shm_open with C_CREAT here! then ftruncate! then mmap
        fd[0] = shm_open("Image1",O_CREAT|O_RDWR, 0600);
        fd[1] = shm_open("Image2",O_CREAT|O_RDWR, 0600);
        fd[2] = shm_open("OutputImage",O_CREAT|O_RDWR, 0600);
        fd[3] = shm_open("synchobject",O_CREAT|O_RDWR, 0600);

        ftruncate(fd[0], IMAGEBYTES); 
        ftruncate(fd[1], IMAGEBYTES); 
        ftruncate(fd[2], IMAGEBYTES); 
        ftruncate(fd[3], 10*sizeof(int)); 

        pixelTable1 = (BYTE*) mmap(NULL, IMAGEBYTES, PROT_READ|PROT_WRITE, MAP_SHARED, fd[0], 0);
        pixelTable2 = (BYTE*) mmap(NULL, IMAGEBYTES, PROT_READ|PROT_WRITE, MAP_SHARED, fd[1], 0);
        resultTable = (BYTE*) mmap(NULL, IMAGEBYTES, PROT_READ|PROT_WRITE, MAP_SHARED, fd[2], 0);
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
        fd[0] = shm_open("Image1",O_CREAT|O_RDWR, 0600);
        fd[1] = shm_open("Image2",O_CREAT|O_RDWR, 0600);
        fd[2] = shm_open("OutputImage",O_CREAT|O_RDWR, 0600);
        fd[3] = shm_open("synchobject",O_CREAT|O_RDWR, 0600);
        
        pixelTable1 = (BYTE*) mmap(NULL, IMAGEBYTES, PROT_READ|PROT_WRITE, MAP_SHARED, fd[0], 0);
        pixelTable2 = (BYTE*) mmap(NULL, IMAGEBYTES, PROT_READ|PROT_WRITE, MAP_SHARED, fd[1], 0);
        resultTable = (BYTE*) mmap(NULL, IMAGEBYTES, PROT_READ|PROT_WRITE, MAP_SHARED, fd[2], 0);
        ready = (int*) mmap(NULL, 10*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd[3], 0);
        sleep(2); //needed for initalizing synch
    }

    ready[par_id] = 1;
    synch(par_id,par_count,ready, 1);


    if (par_id == 0) // read inimage data
    {
        FILE *pictureFile1 = fopen(localImageFile1, "rb"); 
        FILE *pictureFile2 = fopen(localImageFile2, "rb");

        if((pictureFile1 == NULL)||(pictureFile2 == NULL)) {
            cout << "Error opening at least one of the files\n";
        }
        else {
            // first image read
            readBMPData(pixelTable1, pictureFile1, pictureHeader1, pictureInfo1);
            fclose(pictureFile1); //close the file
            //write data to new file

            // second image read
            readBMPData(pixelTable2, pictureFile2, pictureHeader2, pictureInfo2);
            fclose(pictureFile2); //close the file
        }

    } 

    ready[par_id] = 2;
    synch(par_id,par_count,ready, 2);       

    struct timeval start, end;

    if (par_id == 0)
    {
        gettimeofday(&start, NULL); // get start time
    }

    matrixMultiplication(pixelTable1, pixelTable2, resultTable, par_id, par_count);

    ready[par_id] = 3;
    synch(par_id,par_count,ready, 3);

    if (par_id == 0)
    {
        gettimeofday(&end, NULL); //get end time
        float multTime = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        cout << "The time taken for the multiplication was "    << multTime/1000 << "msecs" << endl;
    }


    if(par_id == 0)
    {
        FILE *newFile = fopen(localOutputFile, "wb");
       
        writeBMPData1(newFile, pictureHeader1, pictureInfo1);
                // fill RAM with new image data starting at pixelTable[0]   
        fwrite(resultTable, IMAGEBYTES, 1, newFile);
        fclose(newFile); // close the new generated file
    }
    
    ready[par_id] = 4;
    synch(par_id,par_count,ready, 4);

    close(fd[0]);
    close(fd[1]);
    close(fd[2]);
    close(fd[3]);
    shm_unlink("Image1");
    shm_unlink("Image2");
    shm_unlink("OutputImage");
    shm_unlink("synchobject");
        
    
    return 0;
 
}




void readBMPData(BYTE *&pixelTable, FILE *&pictureFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo)
{
    // read into lion file header
    fread(&pictureHeader.bfType, sizeof(WORD), 1, pictureFile);
    fread(&pictureHeader.bfSize, sizeof(DWORD), 1, pictureFile);
    fread(&pictureHeader.bfReserved1, sizeof(WORD), 1, pictureFile);
    fread(&pictureHeader.bfReserved2, sizeof(WORD), 1, pictureFile);
    fread(&pictureHeader.bfOffBits, sizeof(DWORD), 1, pictureFile);

    // read into lion file info header
    fread(&pictureInfo.biSize, sizeof(DWORD), 1, pictureFile);
    fread(&pictureInfo.biWidth, sizeof(LONG), 1, pictureFile);
    fread(&pictureInfo.biHeight, sizeof(LONG), 1, pictureFile); // not negative for lion
    fread(&pictureInfo.biPlanes, sizeof(WORD), 1, pictureFile);
    fread(&pictureInfo.biBitCount, sizeof(WORD), 1, pictureFile);
    fread(&pictureInfo.biCompression, sizeof(DWORD), 1, pictureFile);
    fread(&pictureInfo.biSizeImage, sizeof(DWORD), 1, pictureFile);
    fread(&pictureInfo.biXPelsPerMeter, sizeof(LONG), 1, pictureFile);
    fread(&pictureInfo.biYPelsPerMeter, sizeof(LONG), 1, pictureFile);
    fread(&pictureInfo.biClrUsed, sizeof(DWORD), 1, pictureFile);
    fread(&pictureInfo.biClrImportant, sizeof(DWORD), 1, pictureFile);

    //pixelTable = new BYTE[IMAGEBYTES]; // create dynamic memory for reading image pixels
    // fill RAM with image data starting at pixelTable[0] 
    fread(pixelTable, IMAGEBYTES, 1, pictureFile);
}





void matrixMultiplication(BYTE *pixelData1, BYTE *pixelData2, BYTE *resultData, int par_id, int par_count) 
 {
    float newByte;

    int A_pos, B_pos, C_pos, bytePosition;
    int rowsPerProcess = MATRIX_DIMENSION_XY/par_count;
    int startingRow = par_id*rowsPerProcess;
    int endSegmentRow = (par_id + 1)*rowsPerProcess; // should be in pixels

    float C[IMAGEBYTES] = {0};

    if (par_id == par_count - 1)
    {
        endSegmentRow = MATRIX_DIMENSION_XY;
    }

    //nullify the result matrix first
    for(int a = 0; a < MATRIX_DIMENSION_XY*BYTESPERPIXEL; a++)
        for(int b = startingRow; b < endSegmentRow; b++) // controls the row we go to
            resultData[a + b*MATRIX_DIMENSION_XY] = 0;

    for(int a = 0; a < MATRIX_DIMENSION_XY; a++) // MATRIX_DIMENSION is in pixels
    {
        for(int b = startingRow; b < endSegmentRow; b++) // controls the row we go to
        {
            for(int c = 0; c < MATRIX_DIMENSION_XY; c++)
            {
                for (int i = 0; i < 3; i++)
                {
                    C_pos = b*MATRIX_DIMENSION_XY*BYTESPERPIXEL + a*BYTESPERPIXEL + i;
                    B_pos = c*MATRIX_DIMENSION_XY*BYTESPERPIXEL + a*BYTESPERPIXEL + i;
                    A_pos = b*MATRIX_DIMENSION_XY*BYTESPERPIXEL + c*BYTESPERPIXEL + i;

                    float floatA = (float)pixelData1[A_pos];
                    float floatB = (float)pixelData2[B_pos];

                    floatA /=255;
                    floatB /=255;

                    float newFloat = floatA*floatB;
                    newFloat *= 0.025;
                    
                    C[C_pos] += newFloat;
 
                }
                
            }
        }
    }

    for (int k = startingRow; k < endSegmentRow; k++) { // iterate rows
        for (int j = 0; j < MATRIX_DIMENSION_XY; j++) { // iterate pixels
            for (int i = 0; i < 3; i++)
            {
                bytePosition = k*BYTEWIDTH + j*BYTESPERPIXEL + i;

                if (C[bytePosition] != 0) {
                    int flag = 1;
                }
                resultData[bytePosition] = (BYTE)((C[bytePosition])*255);
            } 
        }
    }




 }





void writeBMPData1(FILE *&newFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo)
{
    // read into picture file header
    fwrite(&pictureHeader.bfType, sizeof(WORD), 1, newFile);
    fwrite(&pictureHeader.bfSize, sizeof(DWORD), 1, newFile);
    fwrite(&pictureHeader.bfReserved1, sizeof(WORD), 1, newFile);
    fwrite(&pictureHeader.bfReserved2, sizeof(WORD), 1, newFile);
    fwrite(&pictureHeader.bfOffBits, sizeof(DWORD), 1, newFile);

    // read into picture file info header
    fwrite(&pictureInfo.biSize, sizeof(DWORD), 1, newFile);
    fwrite(&pictureInfo.biWidth, sizeof(LONG), 1, newFile);
    fwrite(&pictureInfo.biHeight, sizeof(LONG), 1, newFile); // not negative for lion
    fwrite(&pictureInfo.biPlanes, sizeof(WORD), 1, newFile);
    fwrite(&pictureInfo.biBitCount, sizeof(WORD), 1, newFile);
    fwrite(&pictureInfo.biCompression, sizeof(DWORD), 1, newFile);
    fwrite(&pictureInfo.biSizeImage, sizeof(DWORD), 1, newFile);
    fwrite(&pictureInfo.biXPelsPerMeter, sizeof(LONG), 1, newFile);
    fwrite(&pictureInfo.biYPelsPerMeter, sizeof(LONG), 1, newFile);
    fwrite(&pictureInfo.biClrUsed, sizeof(DWORD), 1, newFile);
    fwrite(&pictureInfo.biClrImportant, sizeof(DWORD), 1, newFile);
}



















