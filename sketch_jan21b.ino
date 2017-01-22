/*
 * HID RFID Reader Wiegand Interface for Arduino Uno
 * Originally by  Daniel Smith, 2012.01.30 -- http://www.pagemac.com/projects/rfid/arduino_wiegand
 * 
 * Updated 2016-11-23 by Jon "ShakataGaNai" Davis.
 * See https://obviate.io/?p=7470 for more details & instructions
*/
// needed for SD card
#include <SPI.h>
#include <SD.h>

File myFile; 

// needed for RTC on the Datalogger shield
// https://learn.adafruit.com/adafruit-data-logger-shield
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// end of RTC setup
 
#define MAX_BITS 100                 // max number of bits 
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
 
unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code

int LED_GREEN = 11;
int LED_RED = 12;
int BEEP_BEEP = 10;

// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0() {
  //Serial.print("0");   // uncomment this line to display raw binary
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
 
}
 
// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1() {
  //Serial.print("1");   // uncomment this line to display raw binary
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
void setup() {
  pinMode(LED_RED, OUTPUT);  
  pinMode(LED_GREEN, OUTPUT);  
  pinMode(BEEP_BEEP, OUTPUT);  
  digitalWrite(LED_RED, HIGH); // High = Off
  digitalWrite(BEEP_BEEP, HIGH); // High = off
  digitalWrite(LED_GREEN, LOW);  // Low = On
  pinMode(2, INPUT);     // DATA0 (INT0)
  pinMode(3, INPUT);     // DATA1 (INT1)
 
  Serial.begin(9600);
  Serial.println("RFID Readers");
  // SD card setup
  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

// end of SD Card setup

// RTC chip setup
#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

 // end of RTC setup
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(0, ISR_INT0, FALLING);  
  attachInterrupt(1, ISR_INT1, FALLING);
 
 
  weigand_counter = WEIGAND_WAIT_TIME;
}
 
void loop()
{
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1;  
  }
 
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
 
    Serial.print("Read ");
    Serial.print(bitCount);
    Serial.print(" bits. ");
 
    if (bitCount == 35) {
      // 35 bit HID Corporate 1000 format
      // facility code = bits 2 to 14
      for (i=2; i<14; i++) {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 15 to 34
      for (i=14; i<34; i++) {
         cardCode <<=1;
         cardCode |= databits[i];
      }
 
      printBits();
    }
    else if (bitCount == 26) {
      // standard 26 bit format
      // facility code = bits 2 to 9
      for (i=1; i<9; i++) {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 10 to 23
      for (i=9; i<25; i++) {
         cardCode <<=1;
         cardCode |= databits[i];
      }
 
      printBits();  
    }
    else {
     // you can add other formats if you want!
     // Serial.println("Unable to decode."); 
    }
 
     // cleanup and get ready for the next card
     bitCount = 0;
     facilityCode = 0;
     cardCode = 0;
     for (i=0; i<MAX_BITS; i++) 
     {
       databits[i] = 0;
     }
  }
}
 
void printBits() {
      Serial.print("FC = ");
      Serial.print(facilityCode);
      Serial.print(", CC = ");
      Serial.println(cardCode);
      if (cardCode == 941719) {
      writeBits(cardCode);
      } 
      // Now lets play with some LED's for fun:
      digitalWrite(LED_RED, LOW); // Red
      if(cardCode == 862){
        // If this one "bad" card, turn off green
        // so it's just red. Otherwise you get orange-ish
        digitalWrite(LED_GREEN, HIGH); 
      }
      delay(500);
      digitalWrite(LED_RED, HIGH);  // Red Off
      digitalWrite(LED_GREEN, LOW);  // Green back on
 
      // Lets be annoying and beep more
    /*
      digitalWrite(BEEP_BEEP, LOW);
      delay(500);
      digitalWrite(BEEP_BEEP, HIGH);
      delay(500);
      digitalWrite(BEEP_BEEP, LOW);
      delay(500);
      digitalWrite(BEEP_BEEP, HIGH);
    */
}

void writeBits(long cc) {

    DateTime now = getDateTime();
    String year =  String(now.year(),DEC);
    String day =  String(now.day(),DEC);
    String month = String(now.month(),DEC);
    String hour = String(now.hour(),DEC);
    String minute = String(now.minute(),DEC);
    String second = String(now.second(), DEC);
    
    String fn = year + month + day + ".csv";
   // function to write data to data logger
   // open the file. note that only one file can be open at a time,
   // so you have to close this one before opening another.
  myFile = SD.open(fn, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.print(year + '/' + month + '/' + day + "(" + hour + ":" +minute+ ":" + second + "), " );
    myFile.println(cc);
    // close the file:
    myFile.close();
    Serial.println("done.");
    testBits(fn);
  } else {
    // if the file didn't open, print an error:
    Serial.println("error in initial opening and writing of your file.");
  }



} // end of write bits

void testBits(String fn) {
  // function to open the file and re-read what was just written

    // re-open the file for reading:
  myFile = SD.open(fn);
  if (myFile) {
    Serial.println(fn);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening " + fn + " for re-reading and tesitng");
  }

} // end of testBits()

DateTime getDateTime() {
    DateTime now = rtc.now();
    return now;

} // end of getDataTime


