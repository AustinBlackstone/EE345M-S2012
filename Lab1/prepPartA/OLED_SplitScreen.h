// Austin Blackstone
// January 25, 2012
// EE345M - Lab1 Prep Part1

// OLED_SplitScreen.h
// Runs on LM3S8962
// Implements oLED_Message(int Device, int line, char *string, long value) 
// allowing for split screen displaying

 #define	top		1
 #define	bottom 	0

//------------oLED_Message------------
// displays message split screen on OLED
//  
// Input: device: top, bottom; line: 0-3; *string: string to print; value: int value to print
// Output: none
// outputs String then Value appended, max 21 characters total per line
void oLED_Message(int device, int line, char *string, long value);
