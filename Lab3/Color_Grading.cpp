#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <cmath>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>

using namespace std;

#define BITSPERPIXEL 24
#define BYTESPERPIXEL (BITSPERPIXEL/8)

typedef unsigned short WORD; 
typedef unsigned int DWORD; 
typedef unsigned int LONG;
typedef unsigned char BYTE; 

void changeContrast(BYTE *pixelData, int imageSize, float contrastFactor);
//void changeColorGrading(BYTE *pixelData, int imageSize, int byteWidth, 
 //                               int pixelHeight, float colorGrad[3]); 
void changeColorGrading(BYTE *pixelData, int byteWidth, int pixelWidth,
                                int topRow, int startingRow,float colorGrad[3]);                           
void changeColorGrading2Processes(BYTE *pixelData, int byteWidth, int pixelWidth,
                                int pixelHeight, float colorGrad[3]);

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
 
 int main(int argc, char* argv[]) {

     // command line inputs
     char* localImageFile = argv[1];
     float normalizedRed = atof(argv[2]);
     float normalizedGreen = atof(argv[3]);
     float normalizedBlue = atof(argv[4]);
     char* localOutputFile = argv[5];

     tagBITMAPFILEHEADER pictureHeader;
     tagBITMAPINFOHEADER pictureInfo;

     float contrastFactor; 

     float colorGradients[3] = {normalizedBlue, normalizedGreen, normalizedRed};
     float inverseColorGradients[3] = {1/normalizedBlue, 1/normalizedGreen, 1/normalizedRed};

 

    FILE *pictureFile = fopen(localImageFile, "rb"); // C code. wb when creating file for new, constrasted image
    
     if(pictureFile == NULL) {
         cout << "Error opening file\n";
     }
     else {
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

        
        
        //per pixel of image for resolution (bytes)
        // biWidth is not necessarily multiples of 4. Actually image will be with padding
        int pixelWidth = pictureInfo.biWidth;
        int byteWidth = (pixelWidth)*BYTESPERPIXEL; // width of image in bytes
        if (byteWidth%4 != 0) {
            byteWidth += (4 - byteWidth%4);
        }  
        //byteWidth: image width in bytes with padding accounted for

        int imageBytes = pictureInfo.biHeight*byteWidth; // width*height*bytes
        // create memory the size of the image data (bytes) and put starting address in pixelTable

        //imageBytes does not account for null characters
        
        int pixelHeight = pictureInfo.biHeight; // width of image in pixels
        // create memory the size of the image data (bytes) and put starting address in pixelTable
       //void *mmap(void *addr, size_t length, int prot, int flags,
                 // int fd, off_t offset)
       BYTE *pixelTable =  (BYTE*) mmap(NULL, imageBytes, PROT_READ|PROT_WRITE, 
            MAP_SHARED|MAP_ANONYMOUS, -1, 0);
       // PREV: BYTE *pixelTable = (BYTE*)sbrk(imageBytes); 

        // fill RAM with image data starting at pixelTable[0]
        for (int i = 0; i < imageBytes; i++) {
            fread(&pixelTable[i], 1, 1, pictureFile);
        }
        
        fclose(pictureFile); //close the file
        
        //Change the contrast in the dynamically allocated pixel array
       // changeContrast(pixelTable, imageBytes, contrastFactor);
        
        int topHalfRow = pixelHeight/2; // half way up row value for forking
        //TIME THE FUNCTION 
        struct timeval start, end;

        gettimeofday(&start, NULL); // get start time
        //Color gradient change without forking
        changeColorGrading(pixelTable, byteWidth, pixelWidth, pixelHeight, 0, colorGradients);
        
        gettimeofday(&end, NULL); // get end time

        float timeNoFork = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

        // REVERSE IMAGE: inverseColorGradients
        changeColorGrading(pixelTable, byteWidth, pixelWidth, pixelHeight, 0, inverseColorGradients);

        
        gettimeofday(&start, NULL); // get start time

        changeColorGrading2Processes(pixelTable, byteWidth, pixelWidth, // byteWidth - width in bytes
                            pixelHeight, colorGradients);

        gettimeofday(&end, NULL); // get end time
        
        float timeWithFork = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

        cout << "The time required to change the color contrast without forking was: " 
                << timeNoFork << "us\n"
                << "The time required to change the color contrast with forking was: " 
                << timeWithFork << "us\n"; 
        
        //write data to new file
        FILE *newFile = fopen(localOutputFile, "wb");
        //delete[] localOutputFile;

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

        // fill RAM with new image data starting at pixelTable[0]
        for (int i = 0; i < imageBytes; i++) {
            fwrite(&pixelTable[i], 1, 1, newFile);
        }

        fclose(newFile); // close the new generated file
        
        // free memory the size of the image data (bytes) and put starting address in pixelTable
        // int munmap(void *addr, size_t length);
        int deleteMem = munmap(pixelTable, imageBytes);
        if (deleteMem == -1) {cout << "Memory deallocation failed\n";}
     }
     return 0;
 }

// function edits the pixelTable array in the same memory location, changing the 
// old bytes based on the contrast factor
// Does not have to consider placeholder bytes since these will be processed later
 void changeContrast(BYTE *pixelData, int imageSize, float contrastFactor) {

     float floatByte = 0;
     BYTE newByte = 0;
     for (int i = 0; i < imageSize; i++) {
         floatByte = (float)pixelData[i];
         floatByte /=255; //normalize to 255 (brightest a byte can be)
         floatByte = pow(floatByte, contrastFactor);
         floatByte *= 255; // new brightness value
         newByte = (BYTE)floatByte; // typecast back into BYTE

         pixelData[i] = newByte;
     }
 }

 // function edits the pixelTable array in the same memory location, changing the 
// old bytes based on the color factors
// DOES have to consider placeholder bytes since. biWidth - width in pixels

 void changeColorGrading(BYTE *pixelData, int byteWidth, int pixelWidth,// byteWidth - width in bytes
                            int topRow, int startingRow, float colorGrad[3]) {

     float colorByte;
     BYTE byteColor;
     int tableIndex = 0;
    for (int k = startingRow; k < topRow; k++) { // iterate rows
        for (int j = 0; j < pixelWidth; j++) { // iterate pixels
            for (int i = 0; i < 3; i++) { // iterate bits per pixel (3)
                // iterate by row, pixel, and byte per pixel
                tableIndex = k*byteWidth + j*BYTESPERPIXEL + i;
                /*
                if ((k == topRow - 1 && j == 0 && i == 0)||(k == topRow - 2 && j == pixelWidth - 1 && i == 2)
                        || (k == topRow - 1 && j == pixelWidth - 1 && i == 2)) {
                    cout << tableIndex << endl;
                }*/

                colorByte = (float)pixelData[tableIndex]; 
                    //read first Byte from pixel data (red)
                colorByte /= 255; // normalize to 255 (brightest a byte can be)
                colorByte *= colorGrad[i];
                colorByte *= 255; // new brightness grading for red
                byteColor = (BYTE)colorByte; //typecast into BYTE
                pixelData[tableIndex] = byteColor; // change value in pixel data to adjusted value
            }
    
        }
    }
 }

// uses forking to speed up CPU time
// function edits the pixelTable array in the same memory location, changing the 
// old bytes based on the color factors
// DOES have to consider placeholder bytes since. biWidth - width in pixels
 void changeColorGrading2Processes(BYTE *pixelData, int byteWidth, int pixelWidth, // byteWidth - width in bytes
                            int pixelHeight, float colorGrad[3]) {
                    
    int topHalfRow = pixelHeight/2;

    if (fork() == 0) { // child, top half
        changeColorGrading(pixelData, byteWidth, pixelWidth, pixelHeight, topHalfRow, colorGrad);
        exit(0);
    }
    else {
        changeColorGrading(pixelData, byteWidth, pixelWidth, topHalfRow, 0, colorGrad);   
        wait(0);
    }
 }

