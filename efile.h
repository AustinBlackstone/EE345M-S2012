// filename ************** eFile.h *****************************
// SD Card File System Interface for Lab5
// Austin Blackstone, Cruz Monnreal II 2/16/2012
// Middle-level routines to implement a solid-state disk 
// Jonathan W. Valvano 3/16/11

 #include "edisk.h"

 #ifndef __EFILE_H
 #define __EFILE_H 1
 
 #define FHEADERSIZE 16
 #define BLOCKSIZE 512
 #define FREESPACEINDEX 31 
 #define TOTALNUMBLOCKS 255
 #define NUMFILES 31
 #define DRIVE 0



// Struct for directory File header
// 16bytes total, 16total of these per directory entry, 
// last one is reserved as free space pointer
struct fHeader{
	char name[10]; 			// 10 bytes	// name of file, 9 characters null terminated
	unsigned char next;		// 1 byte	// next block (1->TOTALNUMBLOCKS)
	unsigned char prev;		// 1 byte	// prev block (1->TOTALNUMBLOCKS)
	unsigned short size;	// 2 bytes	//total size of file (in bytes)
	unsigned short padding;	// 2 bytes 	// dead space, 
};
typedef struct fHeader fHeaderType;

// Struct for node (aka block of file)
// total of 512 bytes
struct fNode{
	unsigned short num;	  	// 2 bytes 	// number of used bytes in this file
	unsigned char next;		// 1 byte	// next block (1->TOTALNUMBLOCKS)
	unsigned char prev;		// 1 byte	// prev block (1->TOTALNUMBLOCKS)
	char   data[508];	// 508 bytes// bytes of data
};
typedef struct fNode fNodeType;

union dirUnion{					   // union used for file directory
	BYTE byte[512];			   // used for eDisk_Write
	fHeaderType headers[NUMFILES+1];   // used to access with struct
};

union nodeUnion{
	BYTE byte[512];
	fNodeType node;
};

//---------- eFile_Init-----------------
// Activate the file system, without formating
// Input: none
// Output: 0 if successful and 1 on failure (already initialized)
// since this program initializes the disk, it must run with 
//    the disk periodic task operating
int eFile_Init(void); // initialize file system

//---------- eFile_Format-----------------
// Erase all files, create blank directory, initialize free space manager
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Format(void); // erase disk, add format

//---------- eFile_Create-----------------
// Create a new, empty file with one allocated block
// Input: file name is an ASCII string up to seven characters 
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Create( char name[]);  // create new file, make it empty 


//---------- eFile_WOpen-----------------
// Open the file, read into RAM last block
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_WOpen(char name[]);      // open a file for writing 

//---------- eFile_Write-----------------
// save at end of the open file
// Input: data to be saved
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Write( char data);  

//---------- eFile_Close-----------------
// Deactivate the file system
// Input: none
// Output: 0 if successful and 1 on failure (not currently open)
int eFile_Close(void); 


//---------- eFile_WClose-----------------
// close the file, left disk in a state power can be removed
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_WClose(void); // close the file for writing

//---------- eFile_ROpen-----------------
// Open the file, read first block into RAM 
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble read to flash)
int eFile_ROpen( char name[]);      // open a file for reading 
   
//---------- eFile_ReadNext-----------------
// retreive data from open file
// Input: none
// Output: return by reference data
//         0 if successful and 1 on failure (e.g., end of file)
int eFile_ReadNext( char *pt);       // get next byte 
                              
//---------- eFile_RClose-----------------
// close the reading file
// Input: none
// Output: 0 if successful and 1 on failure (e.g., wasn't open)
int eFile_RClose(void); // close the file for writing

//---------- eFile_Directory-----------------
// Display the directory with filenames and sizes
// Input: pointer to a function that outputs ASCII characters to display
// Output: characters returned by reference
//         0 if successful and 1 on failure (e.g., trouble reading from flash)
int eFile_Directory(int(*fp)(unsigned char));   

//---------- eFile_Delete-----------------
// delete this file
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Delete( char name[]);  // remove this file 

//---------- eFile_RedirectToFile-----------------
// open a file for writing 
// Input: file name is a single ASCII letter
// stream printf data into file
// Output: 0 if successful and 1 on failure (e.g., trouble read/write to flash)
int eFile_RedirectToFile(char *name);

//---------- eFile_EndRedirectToFile-----------------
// close the previously open file
// redirect printf data back to UART
// Output: 0 if successful and 1 on failure (e.g., wasn't open)
int eFile_EndRedirectToFile(void);

//---------- eFileRAM_EmptyFileIndex-----------------
// finds the first available empty file index
// Input: none
// Output: index of first available empty file, 
// ERRORS: outputs index of free space pointer if no file names are available
int eFileRAM_EmptyFileIndex(void);
						   
//---------- eFile_WriteFileIndex-----------------
// write provided information to file index
// Input: name, next pt, prev pt, size
// Output: 0=sucess, 1=failure 
int eFile_WriteFileIndex(int index,char *name, unsigned char nextpt, unsigned char prevpt, unsigned short totalSize);

//---------- eFile_GetBlock-----------------
// take block from beggining of free space
// Input:  none
// Output: sector # removed from head of free list
// ERRORS: 0 on failure, address of free sector on success
int eFile_GetBlock(void);

//---------- eFileRAM_DIRWriteName-----------------
// write given file name to DIR
// Input:  char pointer to name of file
// Output: none
// NOTE: this only modifies the DIR in RAM, it doesnt write or read from memory
void eFileRAM_DIRWriteName(int index, char *name);

//---------- eFileRAM_ClearFileBlock-----------------
// clear all feilds in RAM FileBlock
// Input:  none
// Output: none
// NOTE: this only modifies the DIR in RAM, it doesnt write or read from memory
void eFileRAM_ClearFileBlock(void);

#endif
