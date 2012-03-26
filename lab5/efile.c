// filename ************** eFile.c *****************************
// SD Card File System Interface for Lab5
// Austin Blackstone, Cruz Monnreal II 2/16/2012
// Middle-level routines to implement a solid-state disk 
// Jonathan W. Valvano 3/16/11


#include "efile.h"
#include "edisk.h"

//---------- eFile_Init-----------------
// Activate the file system, without formating
// Input: none
// Output: 0 if successful and 1 on failure (already initialized)
// since this program initializes the disk, it must run with 
//    the disk periodic task operating
int eFile_Init(void){ // initialize file system
	return;
}

//---------- eFile_Format-----------------
// Erase all files, create blank directory, initialize free space manager
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Format(void){ // erase disk, add format
	int x;
	int y;
	int val;
	fNodeType emptyNode;
	//const BYTE *emptyNodePT =(const BYTE)&emptyNode;  
	unsigned char blankDIR[512]={0};

	//write freespace file information
	blankDIR[506]=1;	// nextPT
	blankDIR[507]=TOTALNUMBLOCKS;		// prevPT
	blankDIR[509]=TOTALNUMBLOCKS;		// size (number of empty blocks in this case)
	val=eDisk_WriteBlock(blankDIR,0); 	// write Directory
	if(val!=0){return val;}	 			// error code check

	// link free space together
	for(x=0;x<508;x++){emptyNode.data[x]=0;} // clear all information from blocks
	for(x=1, y=TOTALNUMBLOCKS;x<=TOTALNUMBLOCKS;x++, y=(y+1)%TOTALNUMBLOCKS){//link all the freespace blocks together
		emptyNode.num=0;
		emptyNode.prev=y;
		emptyNode.next=(x%TOTALNUMBLOCKS)+1;
		//if(x=TOTALNUMBLOCKS){emptyNode.next=0;} 		// null terminate last free space
		val=eDisk_WriteBlock((const BYTE)emptyNode,x);	// POSSIBLE ERROR, not sure if this is going to work properly
		//val=eDisk_WriteBlock(emptyNodePT,x);		// POSSIBLE ERROR, not sure if this is going to work properly
		if(val!=0){return val;}			// error code check
	}

	//NOTE: unlike other file pointers the free space list is not null terminated!!!

	return val; //should be zero by this point, anything else and it would have already returned
}


//---------- eFile_Create-----------------
// Create a new, empty file with one allocated block
// Input: file name is an ASCII string up to seven characters 
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Create( char name[]){  // create new file, make it empty 
	int x;
	int val;

	x=eFile_EmptyFileIndex();
	if(FREESPACEINDEX==x)(return 1;) //no more free space available

	//TODO: Finish this 

}


//---------- eFile_WOpen-----------------
// Open the file, read into RAM last block
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_WOpen(char name[]){      // open a file for writing 
	return;
}

//---------- eFile_Write-----------------
// save at end of the open file
// Input: data to be saved
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Write( char data){
	return;
}  

//---------- eFile_Close-----------------
// Deactivate the file system
// Input: none
// Output: 0 if successful and 1 on failure (not currently open)
int eFile_Close(void){
	return;
} 


//---------- eFile_WClose-----------------
// close the file, left disk in a state power can be removed
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_WClose(void){ // close the file for writing
	return;
}

//---------- eFile_ROpen-----------------
// Open the file, read first block into RAM 
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble read to flash)
int eFile_ROpen( char name[]){      // open a file for reading 
  	return; 

}
//---------- eFile_ReadNext-----------------
// retreive data from open file
// Input: none
// Output: return by reference data
//         0 if successful and 1 on failure (e.g., end of file)
int eFile_ReadNext( char *pt){       // get next byte 
 	return;                             
}

//---------- eFile_RClose-----------------
// close the reading file
// Input: none
// Output: 0 if successful and 1 on failure (e.g., wasn't open)
int eFile_RClose(void){ // close the file for writing
  	return;
}

//---------- eFile_Directory-----------------
// Display the directory with filenames and sizes
// Input: pointer to a function that outputs ASCII characters to display
// Output: characters returned by reference
//         0 if successful and 1 on failure (e.g., trouble reading from flash)
int eFile_Directory(int(*fp)(unsigned char)){
 	return;
}   

//---------- eFile_Delete-----------------
// delete this file
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Delete( char name[]){  // remove this file 
 	return;
}

//---------- eFile_RedirectToFile-----------------
// open a file for writing 
// Input: file name is a single ASCII letter
// stream printf data into file
// Output: 0 if successful and 1 on failure (e.g., trouble read/write to flash)
int eFile_RedirectToFile(char *name){
 	return;
}

//---------- eFile_EndRedirectToFile-----------------
// close the previously open file
// redirect printf data back to UART
// Output: 0 if successful and 1 on failure (e.g., wasn't open)
int eFile_EndRedirectToFile(void){
 	return;
}

//---------- eFile_EmptyFileIndex-----------------
// finds the first available empty file index
// Input: none
// Output: index of first available empty file, 
// ERRORS: outputs index of free space pointer if no file names are available
int eFile_EmptyFileIndex(void){
	int x;
	struct fHeader dirNode[32];
	
	eDisk_ReadBlock(dirNode,0);
	for(x=0; 0 != dirNode[x].name ; x++){;}
	return x;	
}

//---------- eFile_WriteFileIndex-----------------
// write provided information to file index
// Input: name, next pt, prev pt, size
// Output: 0=sucess, 1=failure 
int eFile_WriteFileIndex(int index, char *name, unsigned char nextpt, unsigned char prevpt, unsigned short totalSize){
	struct fHeader dirNode[32];
	int val;
	int x;

	val=eDisk_ReadBlock(dirNode,0);
	if(0!=val){return val;}
	//set next, prev, size, and name values of new File
	dirNode[index].next = nextpt;
	dirNode[index].prev = prevpt;
	dirNode[index].size = totalSize;
	for(x=0;x<10 && 0!=name[x];x++){  	//copy name
		dirNode[index].name[x]=name[x];
	}
	val=eDisk_WriteBlock(dirNode,0);

	return val;

}
