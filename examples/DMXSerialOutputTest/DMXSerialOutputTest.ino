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
4  |----------------------| 3 DE     A 6 |---------------- Pin 3
   |                      |              |
TX |----------------------| 4 DI   Gnd 5 |---+------------ Pin 1
   |                                         |
   |                                        GND
 7 |--------[ 330 ohm ]---[ LED ]------------|


 Created August 16th, 2015 by Claude Heintz
 Copyright 2014-2016 see LXUSARTDMX for license

 */

//*********************** includes ***********************

#include <LXUSARTDMX.h>

//*********************** globals ***********************

#define RXTX_PIN 4

uint8_t level = 0;

void setup() {
	LXSerialDMX.setDirectionPin(RXTX_PIN);
	LXSerialDMX.startOutput();
	LXSerialDMX.setMaxSlots(512);
}

void loop() {
  LXSerialDMX.setSlot(7, level);
  LXSerialDMX.setSlot(8, level);
  level++;
  delay(20);
}

