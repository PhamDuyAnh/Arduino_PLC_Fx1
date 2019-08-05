 /*
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you  can usually tell them by the 16-pin interface.
 The circuit:
 * LCD RS pin to digital pin 8
 * LCD En pin to digital pin 9
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 6
 * LCD D7 pin to digital pin 7
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
//                RS  EN  D4  D5 D6 D7
LiquidCrystal lcd( 8,  9,  4,  5, 6, 7);


/*
this program writen by Cartiman for program-plc.blogspot.com
No sell and No Complaint
by program-plc.blogspot.com
*/
#include <FX1S.h>

/*
#define FX1Sbaud 19200
#define FX1Sformat SERIAL_8N1
#define FX1Stimeout 1000
*/
#define FX1Sbaud    9600
#define FX1Sformat  SERIAL_7E1
#define FX1Stimeout 1000

enum
{
  FX1SPACKET1,
  FX1SPACKET2,
  FX1STOTAL_NO_OF_PACKETS
};

FX1SPacket FX1Spackets[FX1STOTAL_NO_OF_PACKETS];

FX1SpacketPointer FX1Spacket1 = &FX1Spackets[FX1SPACKET1];
FX1SpacketPointer FX1Spacket2 = &FX1Spackets[FX1SPACKET2];

unsigned int FX1SreadD[1];
unsigned int FX1SwriteD[1];
boolean readPLC = true;
unsigned int D0, D1;
#define AnalogInputPin A0
#define AnalogOutputPin 9


void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Serial test");
  Setup_LCD();
  
  FX1S_construct(FX1Spacket1, FX1S_READ_D,  0, 1, FX1SreadD);
  FX1S_construct(FX1Spacket2, FX1S_WRITE_D, 1, 1, FX1SwriteD);
  FX1S_configure(&Serial, FX1Sbaud, FX1Sformat, FX1Stimeout, FX1Spackets, FX1STOTAL_NO_OF_PACKETS);
}


void loop()
{
  if (readPLC)
  {
    FX1S_update();
  
    //Write to D1
    D1 = map(analogRead(AnalogInputPin), 0, 1023, 0, 32767);
    FX1SwriteD[0] = D1;
  
    //Read from D0
    D0 = FX1SreadD[0];
    //D0 = map(D0, 0, 32767, 0, 255); 
    readPLC = false;
  }
  else
  {
    /*
     * default  : 327xx
     * Select   : 205xx
     * Left     : 131xx
     * Right    : 000xx
     * Up       : 031xx
     * Down     : 081xx
    */
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Key val: " + String(D1) + "     ");
    lcd.setCursor(0, 1);
    lcd.print("PLC val: " + String(D0) + "     ");
    readPLC = true;
  }
}

void Setup_LCD()
{
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);  
  // startup LOGO  
  lcd.setCursor(4,0);
  lcd.print("CNCProVN");
  lcd.setCursor(2,1);
  lcd.print("Pham Duy Anh");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("www.cncprovn.com");
  lcd.setCursor(0,1);
  lcd.print("Arduino & PLC test");
  delay(1000);
  lcd.clear(); 
}


