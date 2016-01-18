/* DMXUSBSerial
 *  
 *    This sketch allows a Teensy 2.0++ board to emulate parts of an
 *  ENTTEC DMX USB Pro's functions.
 *    The sketch uses the LXUSARTDMX library for DMX input/output
 *  through the teensy's USART 1 serial port.
 *
 *    For the sketch to compile, USE_UART_1 must be defined in the 
 *  LXUSARTDMX library.  USE_UART_1 is automatically
 *  defined for a Teensy 2.0++ with an AVR_AT90USB processor.
 *  However, the sketch should be able to work with any board
 *  with a processor that has more than one USART.
 *  To use with other boards, modify the #define in LXUSARTDMX.cpp 
 *  
 *    This is the circuit for a simple unisolated DMX Shield:

Teensy 2.0++           SN 75176 A or MAX 481CPA
pin                       _______________
 5 |--------------------- | 1      Vcc 8 |------ +5v (Teensy 2.0++ pin 21)
   |                      |              |
   |                 +----| 2        B 7 |---------------- Pin 2
   |                 |    |              |
 7 |----------------------| 3 DE     A 6 |---------------- Pin 3
   |                      |              |
 6 |----------------------| 4 DI   Gnd 5 |---+------------ Pin 1
   |                      _______________    |             DMX Output
   |                                         |
 8 |--------[ 330 ohm ]---[ LED ]------------|
 9 |--------[ 330 ohm ]---[ LED ]------------|
10 |--------[ 330 ohm ]---[ LED ]------------|
 1 |                                        GND 

 *  Created January 18th, 2015 by Claude Heintz
 *  Current version 1.1
 *
 *  Copyright 2014-2016 see LXUSARTDMX for license
 */

#include <LXUSARTDMX.h>
#include "LXENTTECSerial.h"

// ********************** defines **********************

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
#define RED_LED_PIN 6
#define GRN_LED_PIN 7
#define BLU_LED_PIN 8

#define RED_LED_BIT 1
#define GRN_LED_BIT 2
#define BLU_LED_BIT 4

#define RXTX_PIN 4

#define MODE_OUTPUT_DMX 0
#define MODE_INPUT_DMX 1

#define PACKET_LABEL_DMX 6
#define PACKET_LABEL_RECEIVE 8
#define PACKET_LABEL_GET_INFO 3
#define PACKET_LABEL_GET_SERIAL 10

// ********************** globals **********************

uint8_t mode = MODE_OUTPUT_DMX;
int got_dmx = 0;
uint8_t buffer[513];
LXENTTECSerial eSerial = LXENTTECSerial();

// ***************** setup() runs once  ****************

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GRN_LED_PIN, OUTPUT);           
  pinMode(BLU_LED_PIN, OUTPUT);
  LXSerialDMX.setDirectionPin(RXTX_PIN); 
  Serial.begin(57600);//115200 doesn't matter because teensy changes to USB speed
}

// ***************** utility functions  ****************

/*  setLED(uint8_t flags)
 *  sets color of RGB LED
 */

void setLED(uint8_t flags) {
  digitalWrite(RED_LED_PIN, 1 & flags);
  digitalWrite(GRN_LED_PIN, 1 & (flags>>1));
  digitalWrite(BLU_LED_PIN, 1 & (flags>>2));
}


// ***************** input/output functions *************

void gotDMXCallback(int slots) {
  got_dmx = slots;
}

void doOutputMode() {
  uint8_t label = eSerial.readPacket();
  if ( label == ENTTEC_LABEL_SEND_DMX ) {
    int s = eSerial.numberOfSlots() + 1;
    for(int i=0; i<s; i++) {
      LXSerialDMX.setSlot(i,eSerial.getSlot(i));
    }
    LXSerialDMX.startOutput();  //ignored if already started
    setLED(GRN_LED_BIT);
  } else if ( label == ENTTEC_LABEL_RECEIVE_DMX ) {
	LXSerialDMX.setDataReceivedCallback(&gotDMXCallback);
	mode = MODE_INPUT_DMX;
	LXSerialDMX.startInput();
	setLED(BLU_LED_BIT);
  } else if ( label == ENTTEC_LABEL_NONE ) {
    setLED(RED_LED_BIT);
  }
}

void doInputMode() {
  if ( Serial.available()) {  //writing anything to the USB switched to output mode
	LXSerialDMX.setDataReceivedCallback(0);
	mode = MODE_OUTPUT_DMX;
	setLED(0);
    return;
  }
  if ( got_dmx ) {
    int msg_size = got_dmx;
	eSerial.writeDMXPacket(LXSerialDMX.dmxData(), msg_size);
    got_dmx = 0;
  }
  delay(50);         // wait to allow serial to keep up
}

// ***************** main loop  ****************

void loop() {
  if ( mode == MODE_OUTPUT_DMX ) {
    doOutputMode();
  } else {
    doInputMode();
  }
}        //main loop


