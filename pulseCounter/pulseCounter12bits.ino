/*
 *  High Frequency Pulse Counter 12 bits
 *  Yuqian Zhou
 *  May, 2019.
 */
 
/*LIBRARIES*/
#include <stdlib.h>
#include <stdio.h>
#include "TimerOne.h"                 //Library of Timer 1
#include <SoftwareSerial.h>           //Library for serial communication with ch376s on digital pin 11 and 10
#include <usbCh376s.h>                //Library of functions that control ch376s and write file into USB Flash Drive
SoftwareSerial USB(11,12);            // Digital pin 11=RX connnects to TXD on ch376s // Digital pin 12=TX connects to RXD on ch376s 
usbCh376s usbDisk(&USB, &Serial);     //See usbCh376s.cpp and usbCh376s.h for details

/*MACROS*/
int Q0_0=A5;                          //Read pins from two LS160 chip
int Q0_1=A4;                          //Q0 is least significant chip (LSC)
int Q0_2=A3;                          //Q1 is the middle chip
int Q0_3=A2;                          //Q2 is most significant chip (MSC)
int Q1_0=3;
int Q1_1=4;
int Q1_2=5;
int Q1_3=6;
int Q2_0=7;
int Q2_1=8;
int Q2_2=9;
int Q2_3=10;
int TC=2;                             //Terminal Count Output from MSC.Every 100 counts, TC generates a failling edge. Use external interrupt 0 to capture this edge.
int LED=13;                           //Red LED is the signalLED. See user manual for details.
int SW=A0;                            //Switch that controls on/off of pulse counting and USB writing procedures
int writeCycle=10;                    //Time period in seconds between two USB writing procedure.

/*GLOBAL VARIABLES*/
long SEC=1000000;                     //One second in microseconds. For Timer 1 period setting.
int rb0=0;                            //Bits read from Parallel Outputs from three chips
int rb1=0;    
int rb2=0;
int rb3=0;
int rb4=0;
int rb5=0;
int rb6=0;
int rb7=0;
int rb8=0;
int rb9=0;
int rb10=0;
int rb11=0;
unsigned int count=0;                 //Number of counts. Increment 1 every one thousand counts.
int timing=0;                         //Time in seconds. Used to keep track of time. 
String fileName="PULSE.CSV";          //Name of the file where data are saved.
String writeString;
boolean fileCreate=false;             //Check if the file has been created before writing operation.
boolean switchOn = false;             //Check if the botton has been pressed
boolean errorState = false;           //Check if there are errors in the　procedure 

/*SET UP*/
void setup() {
  /*SERIAL COMMUNICATION*/
  Serial.begin(9600);                 //Communication with PC
  USB.begin(9600);                    //Asynchronous serial communication with CH376
  /*I/O PORT INIT*/
  pinMode(LED, OUTPUT);
  pinMode(SW, INPUT);
  pinMode(TC, INPUT);
  pinMode(Q0_0, INPUT);    
  pinMode(Q0_1, INPUT);
  pinMode(Q0_2, INPUT);
  pinMode(Q0_3, INPUT);
  pinMode(Q1_0, INPUT);
  pinMode(Q1_1, INPUT);
  pinMode(Q1_2, INPUT);
  pinMode(Q1_3, INPUT); 
  digitalWrite(LED,LOW);
  /*INTERRUPTS INIT*/
  attachInterrupt(digitalPinToInterrupt(TC),counter,FALLING);     //Initialize external interrupt triggered by a falling edge on TC. ISR: "conter()".
  Timer1.initialize(SEC);                                         //Initialize timer one
  Timer1.setPeriod(SEC);                                          //Set the frequency of timer 1 interrupt to be 1Hz
  Timer1.attachInterrupt(timer);                                  //Define ISR function "timer()"
}
/*ISR*/
void timer(){                         //Timer 1
  timing=timing+1;
}
void counter(){                       //External interrupt 0 or TC
  count=count+1;
}

void loop() {
  /* Step:
   * 1. Power the device via the powerhub. PowerLEDs on powerhub(Green), MCU(Orange) and Counter ICs (Green) will be lit up. However, there is no powerLED on CH376s.
   * 2. Plug in USB disk, and then press the green botton.
   * 3. If working, the red signal LED will blink twice and a red LED on CH376s will be lit up.
   * 4. If connection failed or USB disk is not plugged in, the signal LED will continue blinking, until the error is fixed and the botton is pressed again. 
   * 5. Before unplug the USB, end the procedure by pressing the green botton again. The LEDon CH376s will turn off. Signal LED will blink twice, indicating the user can unplug USB disk now.
   */  
  if(digitalRead(SW)==LOW){           //Switch botton pressed
    errorState=false;
    /*START PROCEDURE*/
    if(!switchOn){
      usbDisk.resetAll();
      usbDisk.set_USB_Mode(0x06);
      if(usbDisk.USB_byte == 0x15){   //USB disk is attached and responding
        Timer1.start();               //Start timer 1
        count=0;                      //Clear variables
        timing=0;
        switchOn=!switchOn;           //Procedure is successfully started
        blinkTwice();
      }
      else{                           //USB disk is not present or connection to ch376s
        errorState=true;              //Failed to start procedure
      }
    }
    /*END PROCEDURE*/
    else{
      usbDisk.resetAll();
      Timer1.stop();                  //Turn off the timer
      count=0;                        //Clear variables
      timing=0;
      switchOn=!switchOn;             //Procedure is successfully ended. User can now unplug USB disk
      blinkTwice();
    }
  }
    /*USB WRITING PROCEDURE*/
  /* Writes the count value into USB disk every writing period, e.g. 30 seconds. 
   * Note that high frequency pulse with 50ns width theoretically has a count value of 30/(2*50*10^-9)/100=3*10^6
   * where higher digits are about 3*10^4, within the range of an unsigned int.
   * To avoid overflow:
   * 1. Use 12 bit counter ICs
   * 2. Decrease writeCycle
   */
  if(timing>=writeCycle && switchOn){
    Timer1.stop();                                    //Pulse timer 1 
    unsigned int remainCount = remain();              //Record lower digits from I/O 
    unsigned int finCount = count;                    //Record higher digits
    count=0;
    if(finCount==0){
      writeString=String(remainCount,DEC);
    }
    else{
      writeString=String(finCount,DEC);
      if(remainCount<10){
        writeString+="00";
      }
      else if(remainCount<100){
        writeString+="0";
      }
    }
    writeString=writeString+remainCount+",";          //Change data into .CSV format.

    digitalWrite(LED,HIGH);                           //Indicate start writing to USB disk
    if(!fileCreate){
      usbDisk.usbWrite(fileName,writeString);         //Create a file/delete the original file and create a new file
      fileCreate=true;
    }
    else{
      usbDisk.usbAppend(fileName,writeString);        //Append to the existing file
    }
    digitalWrite(LED,LOW);                            //End of writing
    timing=0;
    Timer1.restart();                                 //Restart timer
  }
  /*ERROR STATE*/
  /*If the USB is not plugged in or CH376s is not powered, singal LED will keep flashing.*/
  if(errorState && !switchOn){
    blinkFlash();
  }
}

int remain(){
  rb0=digitalRead(Q0_0);                              //Reads bits of three chips
  rb1=digitalRead(Q0_1); 
  rb2=digitalRead(Q0_2); 
  rb3=digitalRead(Q0_3); 
  rb4=digitalRead(Q1_0);
  rb5=digitalRead(Q1_1); 
  rb6=digitalRead(Q1_2); 
  rb7=digitalRead(Q1_3);
  rb8=digitalRead(Q2_0);
  rb9=digitalRead(Q2_1); 
  rb10=digitalRead(Q2_2); 
  rb11=digitalRead(Q2_3);
  int remainLow= rb0 + 2*rb1 + 4*rb2 + 8*rb3;         //Converts the bits to decimal values.
  int remainMid= rb4 + 2*rb5 + 4*rb6 + 8*rb7;
  int remainHigh = rb8 + 2*rb9 + 4*rb10 + 8*rb11;
  if(remainMid==9){                                   //LS160 is a BCD 4bit counter. TC goes high when the count reaches 9.
    remainHigh=remainHigh-1;                          //These operations are for output value correction
  }                                                   //WARNING: TC will be triggered at 989 and the first cycle starts from 0. So the first count is actually 11 smaller.
  if(remainLow==9){
    remainMid=remainMid-1;
  }
  int remainTot=(remainHigh)*100+remainMid*10+remainLow;
  if(remainTot<0){
    remainTot+=1000;
  }
  return remainTot;
}

void blinkTwice(){                                    //Indicate the procedure is successfully started/ended
  digitalWrite(LED,HIGH);
  delay(100);
  digitalWrite(LED,LOW);
  delay(100);
  digitalWrite(LED,HIGH);
  delay(100);
  digitalWrite(LED,LOW);
  delay(100);
}

void blinkFlash(){                                    //Indicate the device is at error state
  digitalWrite(LED,HIGH);
  delay(100);
  digitalWrite(LED,LOW);
  delay(100);
}
