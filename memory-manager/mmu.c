/*
 * Matthew Leeds, CS 426, 2015-11-18, OS Concepts Ch. 9 Project 1
 * This program simulates a virtual memory manager with an address
 * space of 2^16 bytes. It reads a list of logical memory addresses
 * from a file, interpets the lower 16 bits of each as an 8 bit page 
 * number and 8 bit page offset, translates it into a physical address,
 * and reads the data at that address using a BACKING_STORE.bin file.
 * For each address, the logical addr, physical addr, and value read 
 * are outputted, and the page fault rate and TLB hit rate are printed
 * at the end.
 * Compilation: gcc -std=c99 -o mmu mmu.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define SWAP_FILENAME "BACKING_STORE.bin"
#define TLB_SIZE 16
#define NUMBER_PAGES 256
#define PAGE_SIZE 256
#define NUMBER_FRAMES (MEMORY_SIZE / PAGE_SIZE)
// MEMORY_SIZE will be defined at runtime

typedef struct {
  uint8_t pageNumber;
  uint8_t frameNumber;
  bool valid;
} TLBEntry;

typedef struct {
  uint8_t frameNumber;
  bool valid;
} PageTableEntry;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: ./mmu <filename>\n");
    return 1;
  }

  // prompt the user for which physical memory size they want to simulate
  char userInput[2] = {'0', '\0'};
  while (userInput[0] != '1' && userInput[0] != '2') {
    printf("Enter 1 for a physical address space of 2^16 bytes or 2 for 2^15 bytes:");
    fflush(stdout);
    fgets(userInput, 2, stdin);
  }
  unsigned int MEMORY_SIZE = 0;
  if (userInput[0] == '1') MEMORY_SIZE = 65536;
  if (userInput[0] == '2') MEMORY_SIZE = 32768;

  // open the memory stored on disk (swap space)
  FILE* swapFile = fopen(SWAP_FILENAME, "r");
  if (swapFile == NULL) {
    fprintf(stderr, "Error: swap file not accessible\n");
    return 2;
  }

  // open the file of logical addresses to be read
  FILE* addressFile = fopen(argv[1], "r");
  if (addressFile == NULL) {
    fprintf(stderr, "Error: %s not accessible\n", argv[1]);
    return 3;
  }
  
  int8_t memory[MEMORY_SIZE]; // simulated RAM
  const PageTableEntry defaultPageTableEntry = { .valid = false };
  PageTableEntry pageTable[NUMBER_PAGES] = { [0 ... NUMBER_PAGES - 1] = defaultPageTableEntry}; // page table for the single process
  TLBEntry TLB[TLB_SIZE]; // translation lookaside buffer (FIFO)
  uint8_t tlbLength = 0; // number of TLB entries in the TLB
  uint8_t tlbHead = 0; // index of most recent TLBEntry
  bool freeFrames[NUMBER_FRAMES]; // keep track of which frames are free
  for (unsigned int i = 0; i < NUMBER_FRAMES; ++i) {
    freeFrames[i] = true;
  }
  unsigned int nextFrame = 0; // next frame to be used (FIFO)
  uint64_t numberAccesses = 0; // total number of memory accesses
  uint64_t pageFaults = 0; // number of page faults
  uint64_t tlbHits = 0; // number of TLB hits

  // read the logical addresses and simulate a demand paging system
  char line[256]; // temporary buffer
  while (fgets(line, sizeof(line), addressFile) != NULL) {
    uint32_t logicalAddr = atoi(line);
    // get bits 15-8 (0 is the LSB)
    uint8_t pageNumber = (logicalAddr & 0x0000FFFF) >> 8;
    // get bits 7-0
    uint8_t pageOffset = logicalAddr & 0x000000FF;
    int frameNumber = -1; // -1 indicates unknown frame number
    // check the TLB if it has any entries
    if (tlbLength > 0) { 
      for (uint8_t i = 0; i < tlbLength; ++i) {
        if (TLB[i].valid && TLB[i].pageNumber == pageNumber) {
          ++tlbHits;
          frameNumber = (int)TLB[i].frameNumber;
        }
      }
    }
    // check the page table if necessary
    if (frameNumber == -1) { // a TLB miss
      if (pageTable[pageNumber].valid) { // the page is already in memory
        frameNumber = pageTable[pageNumber].frameNumber;
      } else { // we need to bring the page into memory
        ++pageFaults;
        frameNumber = nextFrame;
        if (!freeFrames[frameNumber]) {
          // we don't need to copy the old page to the backing store since writes are not supported
          // invalidate frameNumber in the page table
          for (uint8_t i = 0; i < NUMBER_PAGES; ++i) {
            if (pageTable[i].valid && pageTable[i].frameNumber == frameNumber) {
              pageTable[i].valid = false;
              break;
            }
          }
          // invalidate frameNumber in the TLB
          for (uint8_t i = 0; i < tlbLength; ++i) {
            if (TLB[i].valid && TLB[i].frameNumber == frameNumber) {
              TLB[i].valid = false;
              break;
            }
          }
        } else {
          freeFrames[frameNumber] = false;
        }

        // copy the page from the backing store into memory
        fseek(swapFile, pageNumber * PAGE_SIZE, SEEK_SET);
        fread(&memory[frameNumber * PAGE_SIZE], 1, PAGE_SIZE, swapFile);

        // update the page table
        pageTable[pageNumber].frameNumber = frameNumber;
        pageTable[pageNumber].valid = true;

        // update the TLB
        TLBEntry newTLBEntry = { .pageNumber = pageNumber,
                                 .frameNumber = frameNumber,
                                 .valid = true };
        TLB[tlbHead] = newTLBEntry;

        // update tlbHead, tlbLength, and nextFrame
        tlbHead = ++tlbHead % TLB_SIZE; // FIFO TLB entry replacement
        if (tlbLength < TLB_SIZE) ++tlbLength;
        nextFrame = ++nextFrame % NUMBER_FRAMES; // FIFO frame replacement
      }
    }

    // read the desired value from memory
    uint16_t physicalAddr = (frameNumber * PAGE_SIZE) + pageOffset;
    int8_t value = memory[physicalAddr];
    printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddr, physicalAddr, value);
    numberAccesses++;
  }

  fclose(addressFile);
  fclose(swapFile);

  // print page fault rate and TLB hit rate statistics
  printf("Number of Translated Addresses = %d\n", (int)numberAccesses);
  printf("Page Faults = %d\n", (int)pageFaults);
  double pageFaultRate = (double)pageFaults / (double)numberAccesses;
  printf("Page Fault Rate: %.3f\n", (double)pageFaultRate);
  printf("TLB Hits = %d\n", (int)tlbHits);
  double tlbHitRate = (double)tlbHits / (double)numberAccesses;
  printf("TLB Hit Rate: %.3f\n", (double)tlbHitRate);

  return 0;
}
