/**
Filename:       ringbuffer.c
Name:           Cruz Monrreal II, Austin Blackstone
Creation Date:  01/30/2012
Lab #:          1
TA:             Zahidul
Last Revision:  01/30/2012
Description:    Generic RingBuffer "Class"
*/

#include <stdlib.h>

typedef struct{
  unsigned int size;
  unsigned short buffer[];
  unsigned int ptr;
} RingBuffer;

// Initializes the ring buffer
void RingBuffer_Init(RingBuffer* ring, unsigned int size){
  unsigned int i;
  
  // Allocate memory
  ring = malloc(sizeof(ring) + size*sizeof(unsigned short));
  
  // Update size in buffer
  ring->size = size;
  
  // Pre-populate buffer for the LOLZ
  for(i=0; i<size; i++)
    ring->buffer[i] = i;
}

// Add data to the given ring buffer
void RingBuffer_Add(RingBuffer* ring, unsigned short data){
  // Put data into buffer
  buffer->buffer[buffer->ptr++] = data;
  
  // Check for ptr overflow
  if (buffer->ptr == buffer->size)
    buffer->ptr = 0;
}

void RingBuffer_Remove(RingBuffer* ring, )

// Fetches data from the ring buffer
short getDatafromRingBuffer(short ndx){
  // Normalize Index
  while(ndx < 0)
    ndx += 100;
  
  while(ndx >= 100)
    ndx -= 100;
  
  return ringBuffer[ndx];
}

// Fetches the pointer to the head of the ring buffer
short getRingBufferHead() { return ringBufferPtr; }
