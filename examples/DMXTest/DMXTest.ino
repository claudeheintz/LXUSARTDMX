/*
 This sketch demonstrates output using a DMX shield.
 It outputs DMX in several patterns depending on the
 state of pins which is read at boot time.

 This sketch requires a DMXLibrary,
 LXUSARTDMX is a basic, easy to understand library.

  The sketch uses the microcontroller's USART to transmit
  dmx levels as serial data.  A DMX shield is used to convert
  this output to a differential signal (actual DMX).

  This is the circuit for a simple unisolated DMX Shield:

 Arduino                    SN 75176 A or MAX 481CPA
 pin                      _______________
 |                        | 1      Vcc 8 |------ +5v
 V                        |              |                 DMX Output
   |                 +----| 2        B 7 |---------------- Pin 2
   |                 |    |              |
2  |----------------------| 3 DE     A 6 |---------------- Pin 3
   |                      |              |
TX |----------------------| 4 DI   Gnd 5 |---+------------ Pin 1
   |                                         |
   |                                        GND
 5 |--------[ 330 ohm ]---[ LED ]------------|


 Created August 16th, 2015 by Claude Heintz
 Copyright 2014-2016 see LXUSARTDMX for license

 */

//*********************** includes ***********************

#include <LXUSARTDMX.h>

//*********************** globals ***********************

#define RXTX_PIN 2
   
// choose the range of addresses for the fade sequence:
#define LOW_ADDRESS 1
#define HIGH_ADDRESS 512

// these pins are used at startup to determine which program will loop
#define SELECT_PIN_0 8
#define SELECT_PIN_1 9

#define PROGRAM_FADE_ALL 0
#define PROGRAM_SEQUENCE 1

uint8_t program = 0;
int address = LOW_ADDRESS;
uint8_t level = 0;
uint8_t direction = 1;

// out(address, level) is a utility function for
// setting the level of a DMX address

void out(int addr, uint8_t vx) {
  LXSerialDMX.setSlot(addr,vx);
}

void setup() {
	LXSerialDMX.setDirectionPin(RXTX_PIN);
	LXSerialDMX.startOutput();
	LXSerialDMX.setMaxSlots(HIGH_ADDRESS);
  
  pinMode(SELECT_PIN_0, INPUT);
  pinMode(SELECT_PIN_1, INPUT);
  if ( digitalRead(SELECT_PIN_0) == HIGH ) {
    program += 1;
  }
  if ( digitalRead(SELECT_PIN_1) == HIGH ) {
    program += 2;
  }
}

// ***** fadeUpDownOneAtATime  *****
//
// fades a single address up and then down
// 

void fadeUpDownOneAtATime() {
  out(address,level);
  delay(5);

  if ( direction ) {
    level++;
    if ( level == 255 ) {
      direction = 0;
    }
  } else {
    level--;
    if ( level == 0 ) {
      direction = 1;
      out(address,level);
      address++;
      if ( address > HIGH_ADDRESS) {
        address = LOW_ADDRESS;
      }
    }
  }
}

// ***** fadeAll  *****
//
// fades all addresses up and then down
// 

void fadeAll() {
  for (int i=LOW_ADDRESS; i<= HIGH_ADDRESS; i++) {
    out(i, level);
  }
  delay(10);
  if ( direction ) {
    level++;
    if ( level == 255 ) {
      direction = 0;
    }
  } else {
    level--;
    if ( level < 0 ) {
      level = 0;
      direction = 1;
    }
  }
}

// ***** allOnThenAllOff  *****
//
// turns all addresses on sequentially
// then turns all addresses off sequentially

void allOnThenAllOff() {
  if ( direction == 1 ) {
    out(address,255);
  } else {
    out(address,0);
  }
  delay(5);
  
  address++;
  if ( address > HIGH_ADDRESS) {
    address = LOW_ADDRESS;
    if ( direction == 1 ) {
      direction = 0;
    } else {
      direction = 1;
    }
  }
}

void loop() {
  if ( program == PROGRAM_FADE_ALL ) {
    fadeAll();
  } else if ( program == PROGRAM_SEQUENCE ) {
    allOnThenAllOff();
  } else {
    fadeUpDownOneAtATime();
  }
}

