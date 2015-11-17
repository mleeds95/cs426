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

#define SWAP_FILENAME "BACKING_STORE.bin"
#define MEMORY_SIZE 65536
#define TLB_SIZE 16
#define NUMBER_PAGES 256

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
  
  char memory[MEMORY_SIZE]; // simulated RAM
  const PageTableEntry defaultPageTableEntry = { .valid = false };
  PageTableEntry pageTable[NUMBER_PAGES] = {defaultPageTableEntry}; // page table for the single process
  const TLBEntry defaultTLBEntry = { .valid = false };
  TLBEntry TLB[TLB_SIZE] = {defaultTLBEntry}; // translation lookaside buffer
  uint64_t numberAccesses = 0; // total number of memory accesses
  uint64_t pageFaults = 0; // number of page faults
  uint64_t tlbHits = 0; // number of TLB hits

  // read the logical addresses and simulate a demand paging system
  char line[256]; // temporary buffer
  while (fgets(line, sizeof(line), addressFile) != NULL) {
    uint32_t logicalAddr = atoi(line);
    uint8_t pageNumber = (logicalAddr & 0x0000FFFF) >> 8;
    uint8_t pageOffset = logicalAddr & 0x000000FF;
    printf("%d %d %d\n", logicalAddr, pageNumber, pageOffset);
    //TODO check the TLB for pageNumber
    //TODO check the page table for pageNumber (if necessary)
    //TODO bring the page into memory (if necessary)
    //TODO update the pageTable/TLB (if necessary)
    //TODO read and print the value at the specified location
    numberAccesses++;
  }

  fclose(addressFile);
  fclose(swapFile);

  // print page fault rate and TLB hit rate statistics
  double pageFaultRate = (double)pageFaults / (double)numberAccesses;
  printf("page fault rate: %05.2f%%\n", pageFaultRate);
  double tlbHitRate = (double)tlbHits / (double)numberAccesses;
  printf("TLB hit rate: %05.2f%%\n", tlbHitRate);

  return 0;
}
