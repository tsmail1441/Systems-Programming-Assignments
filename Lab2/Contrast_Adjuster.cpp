#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <cmath>

using namespace std;

#define BITSPERPIXEL 24
#define BYTESPERPIXEL (BITSPERPIXEL/8)

typedef unsigned short WORD; 
typedef unsigned int DWORD; 
typedef unsigned int LONG;
typedef unsigned char BYTE; 

void changeContrast(BYTE *pixelData, int imageSize, float contrastFactor);

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
 
 int main() {

     tagBITMAPFILEHEADER pictureHeader;
     tagBITMAPINFOHEADER pictureInfo;
    
     char *programName = new char[30]; // definition for program name string

     char *imageFile = new char[30]; // definition for image file name from cin
     char *localImageFile = new char[30]; // definition for ./ + image file name
     strcpy(localImageFile, "./");

     char *outputFile = new char[30]; // definition for image output file name
     char *localOutputFile = new char[30]; // definition for image output file name
     strcpy(localOutputFile, "./");

     float contrastFactor = 0; 

     cout << "Enter the following [programname] [imagefile1] [outputfile] [contrastfactor]\n";
     cin >> programName >> imageFile >> outputFile >> contrastFactor;
     strcat(localImageFile, imageFile); // ./ + image file name
     strcat(localOutputFile, outputFile); // ./ + image file name

    // FILE *pictureFile = fopen("./lion.bmp", "rb"); // C code. wb when creating file for new, constrasted image
    FILE *pictureFile = fopen(localImageFile, "rb"); // C code. wb when creating file for new, constrasted image

    delete[] imageFile; delete[] localImageFile; delete[] outputFile; delete[] programName;
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

        int imageBytes = pictureInfo.biHeight*pictureInfo.biWidth*BYTESPERPIXEL; // width*height*bytes
        //per pixel of image for resolution (bytes)
        
        // create memory the size of the image data (bytes) and put starting address in pixelTable
        BYTE *pixelTable = (BYTE*)sbrk(imageBytes);   
        // fill RAM with image data starting at pixelTable[0]
        for (int i = 0; i < imageBytes; i++) {
            fread(&pixelTable[i], 1, 1, pictureFile);
        }
       // fread(&pixelTable, 1, imageBytes, pictureFile);
        
        fclose(pictureFile); //close the file
        
        //Change the contrast in the dynamically allocated pixel array
        changeContrast(pixelTable, imageBytes, contrastFactor);
        
        //write data to new file
        FILE *newFile = fopen(localOutputFile, "wb");
        delete[] localOutputFile;
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
        (BYTE*)sbrk(-imageBytes); 
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
