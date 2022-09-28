#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#define PAGESIZE 4096
typedef unsigned char BYTE;

using namespace std;

void *startofheap = NULL; // WHY IS THIS VOID? No Heap

typedef struct chunkinfo 
{ 
    int size; //size of the complete chunk including the chunkinfo 
    int inuse; //is it free? 0 or occupied? 1 
    BYTE *next = NULL; BYTE *prev = NULL;//address of next and prev chunk 
} chunkinfo;

int chunkInfoSize = sizeof(chunkinfo);
chunkinfo* getLastChunk();
void analyze();
void myFree(BYTE* address);
BYTE* myMalloc(int demand);



int main() {
    
    BYTE* a[100];
    clock_t ca, cb;
    analyze();
    for (int i = 0; i<100; i++) 
        a[i] = myMalloc(1000); // a[i] stores address of each chunk data start
    analyze();
    for (int i = 0; i<90; i++)
        myFree(a[i]);
    myFree(a[95]);
    analyze();
    a[95] = myMalloc(1000); 
    analyze();

    for (int i=90; i < 100; i++) 
        myFree(a[i]);
    analyze();
    cb = clock();
    printf("\nduration: %f\n", (double)(cb - ca));


    return 0;
}

BYTE* myMalloc(int demand) 
{
    int demandBytes = demand + chunkInfoSize; // total size of data plus info in bytes
    //int pagesRequired = demandBytes/PAGESIZE + 1; // 
    int realDemand = (demandBytes/PAGESIZE + 1)<<12; // actual size allocated including info

    if (startofheap == NULL) // if first page not allocated
    {
        chunkinfo *chunk = (chunkinfo*) sbrk(chunkInfoSize);
        chunk -> size = realDemand;
        chunk -> inuse = 1;
        startofheap = chunk; // now start points to this node (first)
        sbrk(realDemand - chunkInfoSize);
        
        return (BYTE*) (chunk + chunkInfoSize); // address AFTER the chunk info header
    }
    
    chunkinfo* lastChunkPtr = getLastChunk();
    chunkinfo* chunkPtr = (chunkinfo*)startofheap;
    chunkinfo* bestFit = NULL;

    // while the current pointer isn't the last 
    while (chunkPtr != NULL) 
    {   // if the chunk is free and big enough, check if it's the best option
        if ((chunkPtr-> inuse == 0) && (chunkPtr-> size >= realDemand))
        { // potentially the best fit, check
            if ((bestFit == NULL) || (bestFit-> size > chunkPtr-> size))
            {
                bestFit = chunkPtr;
            }
        }
        chunkPtr = (chunkinfo*)chunkPtr->next;
    }

    if (bestFit == NULL) // none free
    { // create new chunk at end
        chunkinfo *endchunk = (chunkinfo*) sbrk(chunkInfoSize);
        endchunk -> size = realDemand;
        endchunk -> inuse = 1;
        endchunk -> prev = (BYTE*)lastChunkPtr; 
        lastChunkPtr -> next = (BYTE*)endchunk;
        sbrk(realDemand - chunkInfoSize);
        
        return (BYTE*) (endchunk + chunkInfoSize); // address AFTER the chunk info header
    }

    else // does realDemand require fewer than available pages? If so, split
    { 
        if (((bestFit -> size) - realDemand) >= PAGESIZE)
        { 
            // split so the first chunk has the required size 
            chunkinfo *nextChunk = (chunkinfo*)(bestFit + realDemand);
            nextChunk -> prev = (BYTE*)bestFit;
            nextChunk -> inuse = 0;
            nextChunk -> next = bestFit -> next;
            nextChunk -> size = (bestFit -> size) - realDemand;

            bestFit -> next = (BYTE*)nextChunk; 
            bestFit -> size = realDemand;
        }
        bestFit -> inuse = 1;

        return (BYTE*) (bestFit + chunkInfoSize);
    }


    return NULL;
}

void myFree(BYTE* address)
{ // sets chunk free at given address (address is after chunk info)

    chunkinfo* headerAddress = (chunkinfo*)(address - 576);
    chunkinfo* prevPointer = (chunkinfo*)headerAddress->prev;
    chunkinfo* nextPointer = (chunkinfo*)headerAddress->next;
    headerAddress -> inuse = 0;
    //if last chunk
    if (nextPointer == NULL) //if pointing at the last chunk
    {    
        if (prevPointer->inuse == 0) // if the prev chunk also free
        { 
            chunkinfo* prevPrevPointer = (chunkinfo*)(prevPointer->prev);
            // set prev pointer next to NULL (last chunk)
            if (prevPrevPointer == NULL) // means only two chunks remaining
            {
                startofheap = NULL;                 
            }
            else // not only two remaining, can access third block up
            {   // set third block up next to NULL, indicating last chunk
                prevPrevPointer->next = NULL; 
                // set size to size of al 3 chunks
                prevPrevPointer->size 
                        += ((headerAddress->size) + (prevPointer->size));
            }      
            brk(prevPointer); 
            return;
        }
        // set prev pointer next to NULL (last chunk)
        prevPointer->next = NULL;
        brk(headerAddress);
        return;
    } // TO MERGE WITH NEXT CHUNK:
    else if (nextPointer-> inuse == 0) // if next chunk free
    {   
            // set size to size of current and next
            headerAddress-> size += nextPointer->size;
            // set next chunk's next chunk's prev to current chunk
            ((chunkinfo*)(nextPointer->next))->prev = (BYTE*)headerAddress;       
            // set next to next of following chunk
            headerAddress-> next = nextPointer->next; // NEXTPOINTER CHANGES HERE
    }
    // TO MERGE WITH PREVIOUS CHUNK:
    //prevPointer = (chunkinfo*)headerAddress->prev;
    if (prevPointer != NULL) // if not first chunk
    { 
        //chunkinfo* nextPointer = (chunkinfo*)headerAddress->next;
        if (prevPointer-> inuse == 0) // if prev chunk is free
        {   
            // set size to size of both chunks
            prevPointer-> size += headerAddress->size;
            // set next of prev to next of current
            prevPointer-> next = headerAddress->next;
            // set next chunk prev to previous chunk
            ((chunkinfo*)(headerAddress->next))-> prev = headerAddress->prev;
        }
    }
}




chunkinfo* getLastChunk() 
{//you can change it when you aim for performance 
 
    if(!startofheap) //I have a global void *startofheap = NULL; 
    return NULL;   
 
    chunkinfo* ch = (chunkinfo*)startofheap; // address at start of heap
    for (; ch->next; ch = (chunkinfo*)ch->next);
    return ch; // returns address of last chunk (at start of info header?)
 } 


 
 void analyze() 
 {
    printf("\n--------------------------------------------------------------\n"); 
    if(!startofheap) { 
        printf("no heap"); 
        return; 
    } 
    chunkinfo* ch = (chunkinfo*)startofheap; 
    
    for (int no=0; ch; ch = (chunkinfo*)ch->next,no++) { 
        printf("%d | current addr: %x |", no, ch); 
        printf("size: %d | ", ch->size); 
        printf("info: %d | ", ch->inuse); 
        printf("next: %x | ", ch->next); 
        printf("prev: %x", ch->prev); 
        printf("      \n"); 
    } 
    printf("program break on address: %x\n",sbrk(0)); 
 }
