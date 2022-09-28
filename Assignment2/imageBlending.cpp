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
 
void readBMPData(BYTE *&pixelTable, FILE *& pictureFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo, int &imageBytes, int &byteWidth);

void writeBMPData1(FILE *&newFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo, int imageBytes);

 void imageBlending(BYTE *pixelData1, BYTE *pixelData2, float blendRatio, int byteWidth1, 
            int byteWidth2, int pixelWidth1, int pixelWidth2, int pixelHeight1, int pixelHeight2); 

void manpages();

int main(int argc, char* argv[]) {

     // command line inputs
     if ((argc < 6)||(argc > 7))
     {
         cout << "Too few or too many arguments\n\n";

         manpages();
     }
     else
     {

     

        char* localImageFile1 = argv[2];
        char* localImageFile2 = argv[3];
        float blendRatio = atof(argv[4]);
        char* localOutputFile = argv[5];

        tagBITMAPFILEHEADER pictureHeader1, pictureHeader2;
        tagBITMAPINFOHEADER pictureInfo1, pictureInfo2;

        FILE *pictureFile1 = fopen(localImageFile1, "rb"); // C code. wb when creating file for new, constrasted image
        FILE *pictureFile2 = fopen(localImageFile2, "rb");

        if((pictureFile1 == NULL)||(pictureFile2 == NULL)) {
            cout << "Error opening at least one of the files\n";
        }
        else {
            int imageBytes1, imageBytes2, byteWidth1, byteWidth2;
            // function reads the bmp header info into the struct variables
            BYTE *pixelTable1, *pixelTable2;
            
            // first image read
            readBMPData(pixelTable1, pictureFile1, pictureHeader1, pictureInfo1, imageBytes1, byteWidth1);
            fclose(pictureFile1); //close the file
            //write data to new file

            // second image read
            readBMPData(pixelTable2, pictureFile2, pictureHeader2, pictureInfo2, imageBytes2, byteWidth2);
            fclose(pictureFile2); //close the file
            //write data to new file
            
    //void imageBlending(BYTE *pixelData1, BYTE *pixelData2, float blendRatio, int byteWidth1, 
    // int byteWidth2, int pixelWidth1, int pixelWidth2, int pixelHeight1, int pixelHeight2) 

            imageBlending(pixelTable1, pixelTable2, blendRatio, byteWidth1, byteWidth2, pictureInfo1.biWidth,
                                pictureInfo2.biWidth, pictureInfo1.biHeight, pictureInfo2.biHeight);

            FILE *newFile = fopen(localOutputFile, "wb");
            //delete[] localOutputFile;
            if (byteWidth1 >= byteWidth2)
            {
                writeBMPData1(newFile, pictureHeader1, pictureInfo1, imageBytes1);
                // fill RAM with new image data starting at pixelTable[0]   
                fwrite(pixelTable1, imageBytes1, 1, newFile);
            }
            else { 
                writeBMPData1(newFile, pictureHeader2, pictureInfo2, imageBytes2);
            // fill RAM with new image data starting at pixelTable[0]   
                fwrite(pixelTable2, imageBytes2, 1, newFile);
            }


            fclose(newFile); // close the new generated file
            
            // free memory the size of the image data (bytes) and put starting address in pixelTable
            delete[] pixelTable1;
            delete[] pixelTable2;

        }
     }
     return 0;
 }

 void manpages()
 {
     cout << "Enter the following inputs separated by spaces:\n"
          << "[program name] [imagefile1] [imagefile2] [ratio] [output file]\n"
          << "program name: name of the program\n"
          << "imagefile1: first image name that will be blended. Include directory\n"
          << "imagefile2: second image name that will be blended. Include directory\n"
          << "The ratio as a decimal from 0-1 representing how much of the first image is\n"
          << "present vs the second image\n"
          << "output file: The output file of the blended image including the directory\n"
          << "Example:\n"
          << "Assignment1 Images/flowers.bmp Images/flowers.bmp 0.1 OutputImages/lion_wolf.bmp";
 }


 void imageBlending(BYTE *pixelData1, BYTE *pixelData2, float blendRatio, int byteWidth1, 
            int byteWidth2, int pixelWidth1, int pixelWidth2, int pixelHeight1, int pixelHeight2) 
 {
     float newByte, smallerXRatio, smallerYRatio; // smaller image is determined only by width;
     int largerIndex_x, largerIndex_y, largerIndex, smallerIndex_x, smallerIndex_y, smallerIndex, 
                        smallerImageFlag, largerWidth, smallerWidth, 
                        largerPixelWidth, largerHeight, widerImageFlag;

    if (byteWidth1 >= byteWidth2) // image 2 is not as wide
    { // image 1 is at least as wide as 2
        smallerXRatio = ((float)byteWidth2/(float)byteWidth1); 
        // y increment for image 2 is 
        smallerYRatio = ((float)pixelHeight2/(float)pixelHeight1); 
        largerWidth = byteWidth1;
        smallerWidth = byteWidth2;
        largerHeight = pixelHeight1;
        largerPixelWidth = pixelWidth1;
        widerImageFlag = 1;
    }
    else { // second image is wide
        smallerXRatio = ((float)byteWidth1/(float)byteWidth2); 
        // for the same sized image, smallerXIncrement will be 1
        smallerYRatio = ((float)pixelHeight1/(float)pixelHeight2);
        largerWidth = byteWidth2;
        smallerWidth = byteWidth1;
        largerHeight = pixelHeight2;
        largerPixelWidth = pixelWidth2;
        widerImageFlag = 2;
    }

    int X1Y1_position, X2Y1_position, X1Y2_position, X2Y2_position, bytePosition, dx, dy;
    float left_upper, right_upper, left_lower, right_lower, left, right, result;
    // di (float) - width delta
    // dj (float) - length delta
    // i  - index of large image width
    // ii - index of small image width
    // j  - index of large image length
    // jj - index of small image length
    // i ratio (float) - small image width / large image width
    // j ratio (float) - small image length / large image length

    // Iterate i over large image width
        // ii = int(i * (i ratio))
        // di = i * (i ratio) - ii
        // Iterate j over large image length
            // jj = int(j * (j ratio))
            // dj = j * (j ratio) - jj
            // Interpolate to calculate (R, G, B) of (i, j) in output image

    for (int k = 0; k < largerHeight; k++) { // iterate rows
        for (int j = 0; j < largerPixelWidth; j++) { // iterate pixels
            // i: largerIndex_x is the horizontal index of large image 
            //      largerIndex includes the offsetting vertically with each row
            // ii: smallerIndex is the index of the small image: calculate
            //largerIndex = k*largerWidth + j*BYTESPERPIXEL + i; 
            largerIndex_x = j; // index in pixels           
            smallerIndex_x = int(largerIndex_x*smallerXRatio); // X1 coordinate      
            dx = largerIndex_x*smallerXRatio - smallerIndex_x;

            largerIndex_y = k; // index in pixels
            smallerIndex_y = int(largerIndex_y*smallerYRatio); // Y1 coordinate
            dy = largerIndex_y*smallerYRatio - smallerIndex_y;

            // Find coordinates in terms of smaller image bytes
            X1Y1_position = smallerIndex_y*smallerWidth + smallerIndex_x*BYTESPERPIXEL;
            X2Y1_position = X1Y1_position + 1;
            X1Y2_position = X1Y1_position + smallerWidth;
            X2Y2_position = X1Y2_position + 1;

            for (int i = 0; i < 3; i++)
            {
                bytePosition = k*largerWidth + j*BYTESPERPIXEL + i;
               // X_coordinate = (int)((j*BYTESPERPIXEL)*smallerXRatio) + i;

                if (widerImageFlag == 1)
                {// BILINEAR INTERPOLATION:
                    left_upper = (float)pixelData2[X1Y2_position + i];
                    right_upper = (float)pixelData2[X2Y2_position + i];
                    left_lower = (float)pixelData2[X1Y1_position + i];
                    right_lower = (float)pixelData2[X2Y1_position + i];

                    left = left_upper*dy + left_lower*(1-dy);
                    right = right_upper*dy + right_lower*(1-dy);
                    result = left*(1-dx) + right*dx; //resulting byte from smaller image after
                    // bilinear interpolation
                    // byte calculated between the larger image and the smaller image based on the blend ratio
                    newByte = ((float)pixelData1[bytePosition])*blendRatio +
                              result*(1 - blendRatio);

                    pixelData1[bytePosition] = (BYTE)newByte;

                }
                else { // wider image in bytes is pixelData2
                    // BILINEAR INTERPOLATION:
                    left_upper = (float)pixelData1[X1Y2_position + i];
                    right_upper = (float)pixelData1[X2Y2_position + i];
                    left_lower = (float)pixelData1[X1Y1_position + i];
                    right_lower = (float)pixelData1[X2Y1_position + i];

                    left = left_upper*dy + left_lower*(1-dy);
                    right = right_upper*dy + right_lower*(1-dy);
                    result = left*(1-dx) + right*dx; //resulting byte from smaller image after
                    // bilinear interpolation
                    // byte calculated between the larger image and the smaller image based on the blend ratio
                    newByte = result*(blendRatio) + 
                                ((float)pixelData2[bytePosition])*(1-blendRatio);
                              

                    pixelData2[bytePosition] = (BYTE)newByte;
                }
            }
        }
    }
 }







void writeBMPData1(FILE *&newFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo, int imageBytes)
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


void readBMPData(BYTE *&pixelTable, FILE *&pictureFile, tagBITMAPFILEHEADER &pictureHeader, 
                tagBITMAPINFOHEADER &pictureInfo, int &imageBytes, int &byteWidth)
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

    // biWidth is not necessarily multiples of 4. Actually image will be with padding
    int pixelWidth = pictureInfo.biWidth;
    byteWidth = (pixelWidth)*BYTESPERPIXEL; // width of image in bytes
    if (byteWidth%4 != 0) {
         byteWidth += (4 - byteWidth%4);
    }  
    //byteWidth: image width in bytes with padding accounted for

    imageBytes = pictureInfo.biHeight*byteWidth; // width*height in bytes
    
    int pixelHeight = pictureInfo.biHeight; // width of image in pixels
    // create memory the size of the image data (bytes) and put starting address in pixelTable
    pixelTable = new BYTE[imageBytes]; // create dynamic memory for reading image pixels
    // fill RAM with image data starting at pixelTable[0] 
    fread(pixelTable, imageBytes, 1, pictureFile);
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


