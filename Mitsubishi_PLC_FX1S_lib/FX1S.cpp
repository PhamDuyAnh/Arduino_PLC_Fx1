#include "FX1S.h"
#include "HardwareSerial.h"

#define FX1S_ENQ 1
#define FX1S_IDLE 2
#define FX1S_WAITING_FOR_REPLY 3


#define FX1S_BUFFER_SIZE 128

unsigned char FX1Sstate;


unsigned char msgframe[FX1S_BUFFER_SIZE]; 
unsigned char msgbuffer;
unsigned int FX1Stimeout; 
unsigned int FX1S_T1; 
unsigned long FX1SdelayStart; 
unsigned int FX1S_total_no_of_packets; 
FX1SPacket* FX1SpacketArray; 
FX1SPacket* FX1Spacket; 
HardwareSerial* FX1SPort;

void FX1Senq();
void FX1Sidle();
void FX1SconstructPacket();
unsigned char construct_FX1S_WRITE_D();
unsigned char construct_FX1S_READ_D();
void FX1Swaiting_for_reply();
void FX1SprocessReply();
void process_FX1S_READ_D();
void process_FX1S_WRITE_D();
void FX1SprocessError();
void FX1SprocessSuccess();
void FX1SsendPacket(unsigned char bufferSize);

unsigned int iBUFF0,iBUFF1;
String sBUFF0,sBUFF1,sBUFF2;
unsigned char ASCII_Normalize(unsigned char ASCII_Val);  
bool FX1S_ENQ_PROC;
byte HexCharByte(unsigned char HexChar);

  
void FX1S_update() 
{
	switch (FX1Sstate)
	{
		case FX1S_ENQ:		
		FX1Senq();
		break;
		case FX1S_IDLE:
		FX1Sidle();
		break;
		case FX1S_WAITING_FOR_REPLY:
		FX1Swaiting_for_reply();
		break;
	}
}

void FX1Senq()
{
	msgframe[0] = ENQ;
    FX1SsendPacket(1);
	FX1S_ENQ_PROC = true;
	FX1Sstate = FX1S_WAITING_FOR_REPLY;
}


void FX1Sidle()
{
    static unsigned int packet_index;	
	
	unsigned int failed_connections = 0;
	
	unsigned char current_connection;
	
	do
	{		
		if (packet_index == FX1S_total_no_of_packets) // wrap around to the beginning
			packet_index = 0;
				
		// proceed to the next FX1Spacket
		FX1Spacket = &FX1SpacketArray[packet_index];
		
		// get the current FX1Sconnection status
		current_connection = FX1Spacket->FX1Sconnection;
		
		if (!current_connection)
		{			
			// If all the FX1Sconnection attributes are false return
			// immediately to the main sketch
			if (++failed_connections == FX1S_total_no_of_packets)
				return;
		}
		packet_index++;     
    
	// if a FX1Spacket has no FX1Sconnection get the next one		
	}while (!current_connection); 
		
	FX1SconstructPacket();
}
  
  
void FX1SconstructPacket()
{	
  FX1Spacket->FX1Srequests++;
  FX1Spacket->FX1Serror=99;
   
  msgframe[0] = STX;
  msgframe[1] = FX1Spacket->FNC;
  msgframe[2] = 0x31;  
  
  iBUFF0 = FX1Spacket->ADDR * 2;
  sBUFF0 = "000" + String(iBUFF0, HEX);
  sBUFF0.toUpperCase();  
  iBUFF1 = sBUFF0.length()-3;
  sBUFF2 = sBUFF0.substring(iBUFF1);  

  msgframe[3] = sBUFF2[0];
  msgframe[4] = sBUFF2[1];
  msgframe[5] = sBUFF2[2];
  
  
  iBUFF0 = FX1Spacket->SIZE * 2;
  sBUFF0 = "00" + String(iBUFF0, HEX);
  sBUFF0.toUpperCase();  
  iBUFF1 = sBUFF0.length()-2;
  sBUFF2 = sBUFF0.substring(iBUFF1);  

  msgframe[6] = sBUFF2[0];
  msgframe[7] = sBUFF2[1]; 	
	
  unsigned char frameSize;    
	
	if (FX1Spacket->FNC == FX1S_WRITE_D) {
		frameSize = construct_FX1S_WRITE_D();
	}else if (FX1Spacket->FNC == FX1S_READ_D){
	    frameSize=construct_FX1S_READ_D();	
	}
    
  FX1SsendPacket(frameSize);
  FX1Sstate = FX1S_WAITING_FOR_REPLY;
}

unsigned char construct_FX1S_WRITE_D()
{
	
  unsigned char no_of_D = FX1Spacket->SIZE;     
  unsigned char index = 8;
		
  for (unsigned char i = 0; i < no_of_D; i++)
  {
    iBUFF0 = FX1Spacket->DATA_ARRAY[i];
	sBUFF0 = "0000" + String(iBUFF0, HEX);
	sBUFF0.toUpperCase();  
	iBUFF1 = sBUFF0.length()-4;
	sBUFF2 = sBUFF0.substring(iBUFF1); 
  
    msgframe[index] = sBUFF2[2];
    index++;
    msgframe[index] = sBUFF2[3];
    index++;
	msgframe[index] = sBUFF2[0];
    index++;
	msgframe[index] = sBUFF2[1];
    index++;
  }
  
    msgframe[index] = ETX;
	
 	unsigned int iCheckSum = 0;
	for (unsigned char i = 1; i <= index; i++)
		iCheckSum += msgframe[i];

  iBUFF0 = iCheckSum;
  sBUFF0 = "00" + String(iBUFF0, HEX);
  sBUFF0.toUpperCase();  
  iBUFF1 = sBUFF0.length()-2;
  sBUFF2 = sBUFF0.substring(iBUFF1); 
  
  index++;
  msgframe[index] = sBUFF2[0];
  index++;
  msgframe[index] = sBUFF2[1];
  index++;
  
  return index;
}

unsigned char construct_FX1S_READ_D()
{
	unsigned char index = 8;
    msgframe[index] = ETX;
	
 	unsigned int iCheckSum = 0;
	for (unsigned char i = 1; i <= index; i++)
		iCheckSum += msgframe[i];

  iBUFF0 = iCheckSum;
  sBUFF0 = "00" + String(iBUFF0, HEX);
  sBUFF0.toUpperCase();  
  iBUFF1 = sBUFF0.length()-2;
  sBUFF2 = sBUFF0.substring(iBUFF1); 
  
  index++;
  msgframe[index] = sBUFF2[0];
  index++;
  msgframe[index] = sBUFF2[1];
  index++;
  
	return index;
}


void FX1Swaiting_for_reply()
{

	if ((*FX1SPort).available()) // is there something to check?
	{
		unsigned char overflowFlag = 0;
		msgbuffer = 0;		
		while ((*FX1SPort).available())
		{

			if (overflowFlag) 
				(*FX1SPort).read();
			else
			{
				if (msgbuffer == FX1S_BUFFER_SIZE)
					overflowFlag = 1;

				msgframe[msgbuffer] = ASCII_Normalize((*FX1SPort).read());
				msgbuffer++;
			}

			delayMicroseconds(FX1S_T1); // inter character time out
		}		
		
		if (overflowFlag){
			FX1SprocessError();  			
		}else if (msgframe[0] == STX || msgframe[0] == ACK){
			FX1SprocessReply();					
		}else{
			FX1SprocessError();	
		}
			
	}
	else if ((millis() - FX1SdelayStart) > FX1Stimeout) // check FX1Stimeout
	{
		FX1SprocessError();
	}
}

void FX1SprocessReply()
{
	if(FX1S_ENQ_PROC){
		  if (msgframe[0]==ACK){
			FX1Sstate = FX1S_IDLE;
			FX1S_ENQ_PROC = false;
		  }else{
			FX1Sstate = FX1S_ENQ;
			FX1S_ENQ_PROC = true;
		  }
	}else{
		switch (FX1Spacket->FNC)
		{
			case FX1S_WRITE_D:
				process_FX1S_WRITE_D();
				break;
			case FX1S_READ_D:
				process_FX1S_READ_D();
				break;	
			default:
				FX1SprocessError();
				break;
		}		
	}	
}

void process_FX1S_WRITE_D()
{	
  if (msgframe[0]==ACK){
    FX1SprocessSuccess();
  }else{
    FX1SprocessError();
  }
}

void process_FX1S_READ_D()
{
unsigned int TotalDataRead = (FX1Spacket->SIZE * 4) + 4;

  if (msgframe[0]==STX && TotalDataRead==msgbuffer) 
  {
	unsigned long Sum = 0;
	for (unsigned char i = 1; i < msgbuffer-2; i++)
		Sum +=msgframe[i];

	  sBUFF0 = "00" + String(Sum, HEX);
	  sBUFF0.toUpperCase();  
	  iBUFF1 = sBUFF0.length()-2;
	  sBUFF2 = sBUFF0.substring(iBUFF1); 
 
	  if(msgframe[msgbuffer-2] == sBUFF2[0] && msgframe[msgbuffer-1] == sBUFF2[1]){
		unsigned int index = 1;
		for (unsigned char i = 0; i < FX1Spacket->SIZE; i++)
		{
			iBUFF0 = HexCharByte(msgframe[index+2]) * pow(16,3);
			iBUFF0 += HexCharByte(msgframe[index+3]) * pow(16,2);
			iBUFF0 += HexCharByte(msgframe[index]) * pow(16,1);
			iBUFF0 += HexCharByte(msgframe[index+1]) * pow(16,0);
			FX1Spacket->DATA_ARRAY[i] = iBUFF0; 
			index += 4;
		}
		
		FX1SprocessSuccess(); 
	  }else{

		FX1SprocessError();   
	  }
  }else{
	  
    FX1SprocessError();  
  }
}


byte HexCharByte(unsigned char HexChar)
{
byte HexChar2byte = 0;
if(HexChar==0x30)HexChar2byte=0x00;
if(HexChar==0x31)HexChar2byte=0x01;
if(HexChar==0x32)HexChar2byte=0x02;
if(HexChar==0x33)HexChar2byte=0x03;
if(HexChar==0x34)HexChar2byte=0x04;
if(HexChar==0x35)HexChar2byte=0x05;
if(HexChar==0x36)HexChar2byte=0x06;
if(HexChar==0x37)HexChar2byte=0x07;
if(HexChar==0x38)HexChar2byte=0x08;
if(HexChar==0x39)HexChar2byte=0x09;
if(HexChar==0x41)HexChar2byte=0x0A;
if(HexChar==0x42)HexChar2byte=0x0B;
if(HexChar==0x43)HexChar2byte=0x0C;
if(HexChar==0x44)HexChar2byte=0x0D;
if(HexChar==0x45)HexChar2byte=0x0E;
if(HexChar==0x46)HexChar2byte=0x0F;		
return HexChar2byte;
}


void FX1SprocessError()
{
	FX1Spacket->FX1Serror=1;
	FX1Spacket->FX1Sfailed_requests++;	
	FX1Sstate = FX1S_ENQ;
	FX1SdelayStart = millis(); // start the turnaround delay
}

void FX1SprocessSuccess()
{
	FX1Spacket->FX1Serror=0;
	FX1Spacket->FX1Ssuccessful_requests++; // transaction sent successfully
	FX1Sstate = FX1S_IDLE;
	FX1SdelayStart = millis(); // start the turnaround delay
}
  
void FX1S_configure(HardwareSerial* SerialPort,
											long FX1Sbaud,
											unsigned char FX1SbyteFormat,
											unsigned int _FX1Stimeout, 
											FX1SPacket* _FX1Spackets, 
											unsigned int _FX1Stotal_no_of_packets)
{ 
	
	if (FX1Sbaud > 19200)
		FX1S_T1 = 750; 
	else 
		FX1S_T1 = 16500000/FX1Sbaud; // 1T * 1.5 = T1.5
	
	// initialize
	FX1Sstate = FX1S_ENQ;
	FX1Stimeout = _FX1Stimeout;
	FX1S_total_no_of_packets = _FX1Stotal_no_of_packets;
	FX1SpacketArray = _FX1Spackets;
		
	FX1SPort = SerialPort;
	(*FX1SPort).begin(FX1Sbaud, FX1SbyteFormat);
	
} 

void FX1S_construct(FX1SPacket *_FX1Spacket, 
											unsigned char FNC, 
											unsigned int ADDR, 
											unsigned int SIZE, 
											unsigned int* DATA_ARRAY)
{
	_FX1Spacket->FNC = FNC;
	_FX1Spacket->ADDR = ADDR;
	_FX1Spacket->SIZE = SIZE;
	_FX1Spacket->DATA_ARRAY = DATA_ARRAY;
	_FX1Spacket->FX1Sconnection = 1;
}

void FX1SsendPacket(unsigned char bufferSize)
{
	for (unsigned char i = 0; i < bufferSize; i++)
		(*FX1SPort).write(msgframe[i]);		
		
	
	(*FX1SPort).flush();
		
	FX1SdelayStart = millis(); // start the FX1Stimeout delay	
}

unsigned char ASCII_Normalize(unsigned char ASCII_Val)
{
	unsigned char ASCII_Normal = 0;
	switch (ASCII_Val)
	{
		case 0x082:
			ASCII_Normal = 0x02;
			break;
		case 0x0B1:
			ASCII_Normal = 0x031;
			break;
		case 0x0B2:
			ASCII_Normal = 0x032;
			break;
		case 0x0B4:
			ASCII_Normal = 0x034;
			break;
		case 0x0B7:
			ASCII_Normal = 0x037;
			break;
		case 0x0B8:
			ASCII_Normal = 0x038;
			break;
		case 0x0C3:
			ASCII_Normal = 0x043;
			break;
		case 0x0C5:
			ASCII_Normal = 0x045;
			break;
		case 0x0C6:
			ASCII_Normal = 0x046;	
			break;			
		default:
			ASCII_Normal = ASCII_Val;
			break;
	}
  
	return ASCII_Normal;
}
