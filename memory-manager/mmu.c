/*
 * Matthew Leeds, CS 426, 2015-11-17, OS Concepts Ch. 9 Project 1
 * This program simulates a virtual memory manager with an address
 * space of 2^16 bytes. It reads a list of logical memory addresses i
 * from a file, interpets the lower 16 bits of each as an 8 bit page 
 * number and 8 bit page offset, translates it into a physical address,
 * and reads the data at that address using a BACKING_STORE.bin file.
 * For each address, the logical addr, physical addr, and value read 
 * are outputted, and the page fault rate and TLB hit rate are printed
 * at the end.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define SWAP_FILENAME "BACKING_STORE.bin"
#define MEMORY_SIZE 65536
#define TLB_SIZE 16
#define NUMBER_PAGES 256
#define PAGE_SIZE 256

typedef struct {
  uint8_t pageNumber;
  uint8_t frameNumber;
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
  PageTableEntry pageTable[NUMBER_PAGES] = {defaultPageTableEntry}; // page table for the single process
  TLBEntry TLB[TLB_SIZE]; // translation lookaside buffer (will be FIFO)
  uint8_t tlbLength = 0; // number of TLB entries in the TLB
  uint8_t tlbHead = 0; // index of most recent TLBEntry
  bool freeFrames[NUMBER_PAGES] = {[0 ... NUMBER_PAGES - 1] = true}; // keep track of which frames are free
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
    if (tlbLength > 0) { // check the TLB if it has any entries
      for (uint8_t i = 0; i < tlbLength; ++i) {
        if (TLB[i].pageNumber == pageNumber) {
          frameNumber = (int)TLB[i].frameNumber;
          ++tlbHits;
        }
      }
    }
    if (frameNumber == -1) { // if it was a TLB miss...
      if (pageTable[pageNumber].valid) { // the page is already in memory
        frameNumber = pageTable[pageNumber].frameNumber;
      } else { // we need to bring the page into memory
        ++pageFaults;
        // find the first available frame
        for (uint8_t i = 0; i < NUMBER_PAGES; ++i) {
          if (freeFrames[i]) {
            freeFrames[i] = false;
            // copy the page from the backing store into memory
            fseek(swapFile, pageNumber * PAGE_SIZE, SEEK_SET);
            fread(&memory[i * PAGE_SIZE], 1, PAGE_SIZE, swapFile);
            // update the page table and TLB
            pageTable[pageNumber].frameNumber = i;
            pageTable[pageNumber].valid = true;
            frameNumber = i;
            TLBEntry newTLBEntry = { .pageNumber = pageNumber, .frameNumber = i };
            TLB[tlbHead] = newTLBEntry;
            tlbHead = ++tlbHead % TLB_SIZE;
            if (tlbLength < TLB_SIZE) ++tlbLength;
            break;
          } // physical memory == virtual memory, so no need to do frame replacement
        }
      }
    }
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
