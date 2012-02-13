// Austin Blackstone
// January 25, 2012
// EE345M - Lab1 Prep Part1

// OLED_SplitScreen.c
// Runs on LM3S8962
// Implements oLED_Message(int Device, int line, char *string, long value) 
// allowing for split screen displaying


#include <stdio.h>
#include <string.h>
#include "rit128x96x4.h"
#include "OLED_SplitScreen.h"
#define  COLUMWIDTH 6


// output string or integer to top / bottom half of the screen
// Top = 1, bottom = 0
// works by printing the string, then printing the number where the string stopped printing
// no limit on length, will fall off screen

void oLED_Message(int device, int line, char *string, long value){
	char valueToString[21];
	char combined[21];
	int stringLength;
	line=line%4; //limit line to value 0-3
	sprintf(valueToString,"%d",value); //convert integer to string
	stringLength = strlen(string);
	if(device == top){
		line=((line)*12);
		RIT128x96x4StringDraw(string , 0, line, 15);
		RIT128x96x4StringDraw(valueToString , stringLength*COLUMWIDTH, line, 15);	
	} else if(device == bottom){
		line=(line*12)+48;
		RIT128x96x4StringDraw(string, 0, line, 15);
		RIT128x96x4StringDraw(valueToString , stringLength*COLUMWIDTH, line, 15);
		
	}else printf("ERROR: Top/Bottom not specified");

	return; 
}
