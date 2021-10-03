#ifndef SERIAL_H_
#define SERIAL_H_

#include "windows.h"

extern HANDLE hComm;
extern BOOL  Status;
extern DWORD NoBytesRead;

void SerialConnect(int baud_rate, int byte_size, int stop_bits, int parity);

#endif