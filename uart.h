void UARTIntHandler(void);

void UARTInit(void);

void UARTParse(void);

//puts 1 char of data out to the UART, this is our external access function
//returns 1 for sucess, 0 for failure
int UARTPut(unsigned char);

int UARTGet(char * data);
