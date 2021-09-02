#include "../global.h"
#include "serial.h"

HANDLE hComm;                          // Handle to the Serial port
char  ComPortName[] = "COM5";  // Name of the Serial port(May Change) to be opened,
BOOL  Status;                          // Status of the various operations 
DWORD dwEventMask;                     // Event mask to trigger
DWORD NoBytesRead;   

void SerialConnect(int baud_rate, int byte_size, int stop_bits, int parity){
    /*---------------------------------- Opening the Serial Port -------------------------------------------*/
	
	hComm = CreateFile(	ComPortName,					// Name of the Port to be Opened
						GENERIC_READ | GENERIC_WRITE,	// Read/Write Access
						0,							// No Sharing, ports cant be shared
						NULL,						// No Security
						OPEN_EXISTING,                // Open existing port only
						0,							// Non Overlapped I/O
						NULL							// Null for Comm Devices		
	);

	if (hComm == INVALID_HANDLE_VALUE){
		printf("\n Error! - Port %s can't be opened\n", ComPortName);
	}else{
		printf("\n Port %s Opened\n ", ComPortName);
	}
	
	/*------------------------------- Setting the Parameters for the SerialPort ------------------------------*/
	
	DCB dcbSerialParams = { 0 };                         // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	Status = GetCommState(hComm, &dcbSerialParams);      //retreives  the current settings

	if (Status == false){
		printf("\n Error! in GetCommState()");
	}
	
	// dcbSerialParams.BaudRate = CBR_115200;      // Setting BaudRate = 9600
	dcbSerialParams.BaudRate = baud_rate;      // Setting BaudRate = 9600
	dcbSerialParams.ByteSize = byte_size;              // Setting ByteSize = 8
	dcbSerialParams.StopBits = stop_bits;      // Setting StopBits = 1
	dcbSerialParams.Parity = parity;         // Setting Parity = None 

	Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB 

	if (Status == false){
		printf("\n Error! in Setting DCB Structure");
	}else{//If Successfull display the contents of the DCB Structure
	
		// printf("\n\n    Setting DCB Structure Successfull\n");
		printf("\n       Baudrate = %ld", dcbSerialParams.BaudRate);
		printf("\n       ByteSize = %d", dcbSerialParams.ByteSize);
		printf("\n       StopBits = %d", dcbSerialParams.StopBits);
		printf("\n       Parity   = %d", dcbSerialParams.Parity);
	}

	/*------------------------------------ Setting Timeouts --------------------------------------------------*/
	
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 50;
	timeouts.ReadTotalTimeoutConstant    = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.WriteTotalTimeoutConstant   = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	
	if (SetCommTimeouts(hComm, &timeouts) == false){
		printf("\n\n Error! in Setting Time Outs");
	}
	/*------------------------------------ Setting Receive Mask ----------------------------------------------*/
	
	Status = SetCommMask(hComm, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

	if (Status == false){
		printf("\n\n Error! in Setting CommMask");
	}

	// SetCommState(hComm, (LPDCB)&dcbSerialParams);
	
   /*------------------------------------ Setting WaitComm() Event   ----------------------------------------*/
	
	printf("\n\n Waiting for Data Reception\n");

	// char tmp = 0;
	// DWORD num_written;
	// if(WriteFile(hComm, &tmp, 1, &num_written, NULL)){
	// 	printf("success!\n");
	// }

	// Status = WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received
	// Status = WaitCommEvent(hComm, EV_RXCHAR, NULL); //Wait for the character to be received

	/*-------------------------- Program will Wait here till a Character is received ------------------------*/		
}