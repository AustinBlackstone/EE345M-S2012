// filename ************** eFile.c *****************************
// SD Card File System Interface for Lab5
// Austin Blackstone, Cruz Monnreal II 2/16/2012
// Middle-level routines to implement a solid-state disk 
// Jonathan W. Valvano 3/16/11


#include "efile.h"
#include "uart.h"

 int OPENFILEINDEX;
 union dirUnion DIRBlock;
 union nodeUnion FileBlock;
 int StreamToFile=0;


//---------- eFile_Init-----------------
// Activate the file system, without formating
// Input: none
// Output: 0 if successful and 1 on failure (already initialized)
// since this program initializes the disk, it must run with 
//    the disk periodic task operating
int eFile_Init(void){ // initialize file system
	int x;
	x= eDisk_Init(DRIVE);			//use eDisk init fn
	return x;
}

//---------- eFile_Format-----------------
// Erase all files, create blank directory, initialize free space manager
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Format(void){ // erase disk, add format
	int x;
	int y;
	int val;
	fHeaderType empty;
	
//set up empty file header
	empty.next=0;
	empty.prev=0;
	empty.size=0;
	empty.padding=0xFFFF;
	for(x=0;x<10;x++){empty.name[x]=0;}

//write to all entries in DIRBlock
	for(x=0;x<NUMFILES;x++){DIRBlock.headers[x]=empty;}		//clears all entries in DIR except for free space
	//setup freespace pointer
	DIRBlock.headers[FREESPACEINDEX].next=1;				// nextPT
	DIRBlock.headers[FREESPACEINDEX].prev=TOTALNUMBLOCKS;	// prevPT
	DIRBlock.headers[FREESPACEINDEX].size=TOTALNUMBLOCKS;	// size(number of empty blocks in this case
	//DIRBlock.headers[FREESPACEINDEX].padding=0xFA11;		// padding
	val=eDisk_WriteBlock(DIRBlock.byte,0);					//write Directory
	if(val!=0){return val;}	 								// error code check

//link free space together
	for(x=0;x<508;x++){FileBlock.node.data[x]=0;}	//clear all information / data from blocks
	FileBlock.node.num=0;
	for(x=1, y=TOTALNUMBLOCKS;x<=TOTALNUMBLOCKS;x++, y=(y+1)%TOTALNUMBLOCKS){//link all the freespace blocks together
		FileBlock.node.prev=y;
		FileBlock.node.next=(x%TOTALNUMBLOCKS)+1;
		//if(x=TOTALNUMBLOCKS){FileBlock.node.next=0;} 		// null terminate last free space
		val=eDisk_WriteBlock(FileBlock.byte,x);	// POSSIBLE ERROR, not sure if this is going to work properly
		if(val!=0){return val;}			// error code check
	
	} //NOTE: unlike other file pointers the free space list is not null terminated!!!

	return val; //should be zero by this point, anything else and it would have already returned
}


//---------- eFile_Create-----------------
// Create a new, empty file with one allocated block
// Input: file name is an ASCII string up to seven characters 
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Create( char name[]){  // create new file, make it empty 
	int x;
	int y;
	int z;
	int val;
	union nodeUnion temp;
	//val=eDisk_ReadBlock(DIRBlock.byte,0);
	//if(val!=0){return val;}			// error code check

	x=eFileRAM_EmptyFileIndex();
	if(FREESPACEINDEX==x){return 1;} //no more file name space available, exit

	y=eFile_GetBlock();	
	if(0==y){return 1;} //exit, no more free space left to allocate

	val=eDisk_ReadBlock(temp.byte, y);	//get recetly free'd node
	if(val!=0){return val;}			// error code check
	
	for(z=0;z<508;z++){temp.node.data[z]=0;}  //clear data in block

	DIRBlock.headers[FREESPACEINDEX].next=y;
	DIRBlock.headers[FREESPACEINDEX].prev=y;
	temp.node.prev=y;
	temp.node.next=0;


	val=eDisk_WriteBlock(temp.byte,y);
	if(val!=0){return val;}			// error code check

	return 0;

}


//---------- eFile_WOpen-----------------
// Open the file, read into RAM last block
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_WOpen(char name[]){      // open a file for writing 
	int x, y;
	int val;
//TODO: implement semaphore here!!!	

	for(x=0;x<NUMFILES && DIRBlock.headers[x].name[0]!=name[0];x++){;} //find filename's index in DIR headers
	if(x==FREESPACEINDEX){return 1;} //file doesnt exist
	OPENFILEINDEX=x;
	y=DIRBlock.headers[x].prev;
	val=eDisk_ReadBlock(FileBlock.byte,y);
	return val;
}

//---------- eFile_Write-----------------
// save at end of the open file
// Input: data to be saved
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Write( char data){
	int x;
	int val;
	union nodeUnion temp;
	
	val=eDisk_ReadBlock(temp.byte, DIRBlock.headers[OPENFILEINDEX].prev); //get block to be written to
	if(temp.node.num+1<508){ //if theres space left in file block write it
		temp.node.num++;
		temp.node.data[temp.node.num]=data;
		val=eDisk_WriteBlock(temp.byte,DIRBlock.headers[OPENFILEINDEX].prev);
		return val;
	
	}else{ // no space left in current block, allocate a new block and write data there
		DIRBlock.headers[OPENFILEINDEX].size++;
		x=eFile_GetBlock();
		temp.node.next=x;
		val=eDisk_WriteBlock(temp.byte,DIRBlock.headers[OPENFILEINDEX].prev);
		if(val!=0){return 1;}
		val=eDisk_ReadBlock(temp.byte, x);
		if(val!=0){return 1;}
		temp.node.prev=DIRBlock.headers[OPENFILEINDEX].prev;
		temp.node.next=0;
		DIRBlock.headers[OPENFILEINDEX].prev=x;
		temp.node.num=0;
		temp.node.data[temp.node.num]=data;
		temp.node.num++;
		return eDisk_WriteBlock(temp.byte,x);
	}

}  

//---------- eFile_Close-----------------
// Deactivate the file system
// Input: none
// Output: 0 if successful and 1 on failure (not currently open)
int eFile_Close(void){
	//TODO: free the semaphores
	
	return eDisk_WriteBlock(DIRBlock.byte,0);
} 


//---------- eFile_WClose-----------------
// close the file, left disk in a state power can be removed
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_WClose(void){ // close the file for writing
	//TODO: release semaphores

	return eDisk_WriteBlock(DIRBlock.byte,0);
}

//---------- eFile_ROpen-----------------
// Open the file, read first block into RAM 
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble read to flash)
int eFile_ROpen( char name[]){      // open a file for reading 
  	int x, y;
	int val;
//TODO: implement semaphore here!!!	

	for(x=0;x<NUMFILES && DIRBlock.headers[x].name[0]!=name[0];x++){;} //find filename's index in DIR headers
	if(x==FREESPACEINDEX){return 1;} //file doesnt exist
	OPENFILEINDEX=x;
	y=DIRBlock.headers[x].prev;
	val=eDisk_ReadBlock(FileBlock.byte,y);
	return val; 

}
//---------- eFile_ReadNext-----------------
// retreive data from open file
// Input: none
// Output: return by reference data
//         0 if successful and 1 on failure (e.g., end of file)
int eFile_ReadNext(char *pt){       // get next byte 
 	int val;

	if(&FileBlock.node.data[0]<pt && pt< &FileBlock.node.data[FileBlock.node.num]){
		pt++;
		return 0;
	}else{
		if(0==FileBlock.node.next){return 1;} //no more data to read

		val=eDisk_ReadBlock(FileBlock.byte,FileBlock.node.next);
		if(val!=0){return 1;}
		pt=FileBlock.node.data;		//POSSIBLE ERROR: its supposed to make it point to the beggining of the new allocated block's data
	} 

	return 0;                             
}

//---------- eFile_RClose-----------------
// close the reading file
// Input: none
// Output: 0 if successful and 1 on failure (e.g., wasn't open)
int eFile_RClose(void){ // close the file for writing
  	// TODO: Release semaphore
	
	return 0;
}

//---------- eFile_Directory-----------------
// Display the directory with filenames and sizes
// Input: pointer to a function that outputs ASCII characters to display
// Output: characters returned by reference
//         0 if successful and 1 on failure (e.g., trouble reading from flash)
int eFile_Directory(int(*fp)(unsigned char)){
 	//TODO: i have no clue what to do here....
	
	return 1;
}   

//---------- eFile_Delete-----------------
// delete this file
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Delete( char name[]){  // remove this file 
 	int x;
	int t,t2,t3;
	int val;
	union nodeUnion temp, //first block in free list    	Step 1]conect to temp 2		  
					temp2,//last block of deleted file		Step 2]conect to temp
					temp3;//first block of deleted file		Step 3]conect to freepsace header

	for(x=0;DIRBlock.headers[x].name[0] != name[0] && x<FREESPACEINDEX;x++){;}  //x=index of file to be deleted
	if(x==FREESPACEINDEX){return 1;} // exit : trying to delete a non-existant file
	
	t=DIRBlock.headers[FREESPACEINDEX].next;
	t2=DIRBlock.headers[x].prev;
	t3=DIRBlock.headers[x].next;

	val=eDisk_ReadBlock(temp.byte,DIRBlock.headers[FREESPACEINDEX].next);
	if(val!=0){return 1;}
	val=eDisk_ReadBlock(temp2.byte,DIRBlock.headers[x].prev);
	if(val!=0){return 1;}
	val=eDisk_ReadBlock(temp3.byte,DIRBlock.headers[x].next);
	if(val!=0){return 1;}

	//save block addresses, so we can write them back
	t=DIRBlock.headers[FREESPACEINDEX].next;
	t2=DIRBlock.headers[x].prev;
	t3=DIRBlock.headers[x].next;

	//connect in the delete file blocks into the free space listing
	temp.node.next=t2;
	temp2.node.prev=t;
	temp3.node.prev=DIRBlock.headers[FREESPACEINDEX].prev;
	DIRBlock.headers[FREESPACEINDEX].next=t3;

	//write the blocks back into memory
	val=eDisk_WriteBlock(temp.byte,t);
	if(val!=0){return 1;}
	val=eDisk_WriteBlock(temp2.byte,t2);
	if(val!=0){return 1;}
	val=eDisk_WriteBlock(temp3.byte,t3);
	if(val!=0){return 1;}
	

	return 0;
}

//---------- eFile_RedirectToFile-----------------
// open a file for writing 
// Input: file name is a single ASCII letter
// stream printf data into file
// Output: 0 if successful and 1 on failure (e.g., trouble read/write to flash)
//NOTE: copied directly from lab5.pdf
int eFile_RedirectToFile(char *name){
	eFile_Create(name); // ignore error if file already exists
	if(eFile_WOpen(name)) return 1; // cannot open file
	StreamToFile = 1;
	return 0;
}

//---------- eFile_EndRedirectToFile-----------------
// close the previously open file
// redirect printf data back to UART
// Output: 0 if successful and 1 on failure (e.g., wasn't open)
//NOTE: copied directly from lab5.pdf
int eFile_EndRedirectToFile(void){
	StreamToFile = 0;
	if(eFile_WClose()) return 1; // cannot close file
	return 0;
}

//---------- eFileRAM_EmptyFileIndex-----------------
// finds the first available empty file index
// Input: none
// Output: index of first available empty file, 
// ERRORS: outputs index of free space pointer if no file names are available
int eFileRAM_EmptyFileIndex(void){
	int x;
	//struct fHeader dirNode[32];
	
	//eDisk_ReadBlock(DIRBlock.byte,0);
	for(x=0; 0 != DIRBlock.headers[x].name && x<10 ; x++){;}
	return x;	
}

//---------- eFileRAM_WriteFileIndex-----------------
// write provided information to file index
// Input: name, next pt, prev pt, size
// Output: none 
// NOTE: this only modifies the DIR in RAM, it doesnt write or read from memory
void eFileRAM_WriteFileIndex(int index, char *name, unsigned char nextpt, unsigned char prevpt, unsigned short totalSize){
	int x;

	//set next, prev, size, and name values of new File
	DIRBlock.headers[index].next = nextpt;		// next
	DIRBlock.headers[index].prev = prevpt;		// prev
	DIRBlock.headers[index].size = totalSize;	// size
	for(x=0;x<10 && 0!=name[x];x++){  			// copy name
		DIRBlock.headers[index].name[x]=name[x];
	}
	DIRBlock.headers[index].name[x]='\0';		//null terminate
	DIRBlock.headers[index].name[9]='\0';		//guarantee a null termination


	return ;

}

//---------- eFile_GetBlock-----------------
// take block from beggining of free space
// Input:  none
// Output: sector # removed from head of free list
// ERRORS: 0 on failure, address of free sector on success
int eFile_GetBlock(void){
	int x;
	int y;
	int val;
	union nodeUnion temp;

	if(0==DIRBlock.headers[FREESPACEINDEX].next){return 0;} //no more free space left, exit

	x=DIRBlock.headers[FREESPACEINDEX].next;	//get index of block to be used
	
	val=eDisk_ReadBlock(temp.byte , x); 		//get the block to be used
	if(0!=val){return 0;};		//error check
	
	if(temp.node.next==0){	 //if this is the last block
		DIRBlock.headers[FREESPACEINDEX].next=0;
		DIRBlock.headers[FREESPACEINDEX].next=0;
		DIRBlock.headers[FREESPACEINDEX].size=0;
	}else{
		DIRBlock.headers[FREESPACEINDEX].size--;
		y=temp.node.next;						// get following block index
		DIRBlock.headers[FREESPACEINDEX].next=y;	// set free space pointer = next block
		
		val=eDisk_ReadBlock(temp.byte,y);		//get next block
		if(0!=val){return 0;};		//error check
		
		if(0==temp.node.next){	//if next block is the last block
		 	temp.node.prev=0;
			DIRBlock.headers[FREESPACEINDEX].prev=y;
		}else{
			temp.node.prev=DIRBlock.headers[FREESPACEINDEX].prev; //link in next block into current block's place
		}
		
		val=eDisk_WriteBlock(temp.byte,y);		//get next block
		if(0!=val){return 0;};		//error check
	}


	return x;
}

//---------- eFileRAM_DIRWriteName-----------------
// write given file name to DIR
// Input:  char pointer to name of file
// Output: none
// NOTE: this only modifies the DIR in RAM, it doesnt write or read from memory
void eFileRAM_DIRWriteName(int index, char *name){	
	int x;
	for(x=0;x<10 && 0!=name[x];x++){
		DIRBlock.headers[index].name[x]=name[x];
	}
	if(x<10){DIRBlock.headers[index].name[x]='\0';}		//add null terminator
	DIRBlock.headers[index].name[9]='\0';				//guarantee null termination
	return;

}

//---------- eFileRAM_ClearFileBlock-----------------
// clear all feilds in RAM FileBlock
// Input:  none
// Output: none
// NOTE: this only modifies the DIR in RAM, it doesnt write or read from memory
void eFileRAM_ClearFileBlock(void){
	int x;

	FileBlock.node.next=0;
	FileBlock.node.prev=0;
	FileBlock.node.num=0;
	for(x=0;x<508;x++){
		FileBlock.node.data[x]=0;
	}
	return;
}

//---------- fputc-----------------
// redirects printf stream to File
// Input:  
// Output: 

int fputc (int ch, struct fNode file) {
	if(StreamToFile){
		if(eFile_Write(ch)){ // close file on error
			eFile_EndRedirectToFile(); // cannot write to file
			return 1; // failure
		}
	return 0; // success writing
	}

	// regular UART output
	UARTPut(ch);
	return 0;
}

//---------- fgetc-----------------
// redirect for serial outputs
// Input:  
// Output: 

int fgetc (struct fNode f){
	//return(Serial_InChar());
//TODO: implement this shit... not sure what to do....	
	return 1;
}



