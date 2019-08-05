#ifndef FX1S_H
#define FX1S_H

#include "Arduino.h"

#define FX1S_READ_D 0x30 
#define FX1S_WRITE_D 0x31

#define ACK 0x06 
#define ENQ 0x05 
#define STX 0x02
#define ETX 0x03

typedef struct
{
	unsigned char FNC;
	unsigned int ADDR;
	unsigned int SIZE; 
	unsigned int* DATA_ARRAY;
  
  // FX1S information counters
	unsigned int FX1Srequests;
	unsigned int FX1Ssuccessful_requests;
	unsigned int FX1Sfailed_requests;
	unsigned int FX1Serror;
  	
  // FX1Sconnection status of FX1Spacket
	unsigned char FX1Sconnection; 
  
}FX1SPacket;

typedef FX1SPacket* FX1SpacketPointer;


void FX1S_update();

void FX1S_construct(FX1SPacket *_FX1Spacket,  
											unsigned char FNC, 
											unsigned int ADDR, 
											unsigned int SIZE, 
											unsigned int* DATA_ARRAY);
											
void FX1S_configure(HardwareSerial* SerialPort,
											long FX1Sbaud, 
											unsigned char FX1SbyteFormat,
											unsigned int _FX1Stimeout,
											FX1SPacket* _FX1Spackets, 
											unsigned int _FX1Stotal_no_of_packets);

#endif