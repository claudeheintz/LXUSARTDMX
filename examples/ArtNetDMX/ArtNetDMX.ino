/*
 Art-Net DMX

 This sketch demonstrates converting Art-Net packets recieved using an
 Arduino Ethernet Shield to DMX output using a DMX shield.

 This sketch requires software such as LXConsole, QLC+ or DMXWorkshop
 that can send Art-Net packets to the Arduino.

 This sketch requires a DMXLibrary,
 LXUSARTDMX is a basic, easy to understand library.
 
 This example uses a simplified version of the LXDMXEthernet library:
 https://github.com/claudeheintz/LXDMXEthernet_library

  The sketch receives incoming UDP packets.  If a received packet is
  Art-Net DMX output, it uses the microcontroller's USART to transmit
  the dmx levels as serial data.  A DMX shield is used to convert
  this output to a differential signal (actual DMX) for controlling lighting.

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



 Created January 7th, 2014 by Claude Heintz
 Current version 1.5
 (see bottom of file for revision history)

 Copyright 2014-2016 see LXUSARTDMX for license
 Art-Net(tm) Designed by and Copyright Artistic Licence Holdings Ltd.

 */

//*********************** includes ***********************

// Choose which DMX library to include:
// (comment out one or the other, do not include both)
#include <LXUSARTDMX.h>
//#include <DMXSerial.h>

#include <SPI.h>
// *** note if using original Arduino Ethernet Shield
//     this next 2 lines should read:
//     #include <Ethernet.h>
//     #include <EthernetUDP.h>
//     you must also make this change in LXArtNet.h
#include <Ethernet2.h>
#include <EthernetUDP2.h>
#include "LXArtNetEthernet2.h"

//*********************** defines ***********************

/*
   Enter a MAC address and IP address for your controller below.
   The MAC address can be random as is the one shown, but should
   not match any other MAC address on your network.
   
   If BROADCAST_IP is not defined, ArtPollReply will be sent directly to server
   rather than being broadcast.  (If a server' socket is bound to a specific network
   interface ip address, it will not receive broadcast packets.)
*/

#define MAC_ADDRESS 0xDD, 0x43, 0x34, 0x4C, 0x29, 0x7E
#define IP_ADDRESS 192,168,1,140
#define GATEWAY_IP 192,168,1,1
#define SUBNET_MASK 255,255,255,0
#define BROADCAST_IP 192,168,1,255

// this sketch flashes an indicator led:
#define MONITOR_PIN 5

// the driver direction is controlled by:
#define RXTX_PIN 2

//the Ethernet Shield has an SD card that also communicates by SPI
//set its select pin to output to be safe:
#define SDSELECT_PIN 4


//*********************** globals ***********************

//network addresses
byte mac[] = {  MAC_ADDRESS };
IPAddress ip(IP_ADDRESS);
IPAddress gateway(GATEWAY_IP);
IPAddress subnet(SUBNET_MASK);

#if defined( BROADCAST_IP )
IPAddress broadcast_ip( BROADCAST_IP);
#else
IPAddress broadcast_ip = INADDR_NONE;
#endif

// buffer
unsigned char packetBuffer[ARTNET_BUFFER_MAX];

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP eUDP;
LXArtNet artnet = LXArtNet(ip);

//  used to toggle on and off the LED when DMX is Received
int monitorstate = LOW;


/* setup initializes Ethernet and opens the UDP port
   it also sends an Art-Net Poll Reply packet telling other
   Art-Net devices that it can transmit DMX from the network  */

//  ***** blinkLED  *******************************************
//
//  toggles the monitor LED on and off as an indicator

void blinkLED() {
  if ( monitorstate == LOW ) {
    monitorstate = HIGH;
  } else {
    monitorstate = LOW;
  }
  digitalWrite(MONITOR_PIN, monitorstate);
}

//  ***** setup  *******************************************
//
//  initializes Ethernet and opens the UDP port
//  it also sends an Art-Net Poll Reply packet telling other
//  Art-Net devices that it can transmit DMX from the network

void setup() {
  pinMode(MONITOR_PIN, OUTPUT);  //status LED
  blinkLED();
  #if defined(SDSELECT_PIN)
    pinMode(SDSELECT_PIN, OUTPUT);
  #endif

  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  eUDP.begin(ARTNET_PORT);

  LXSerialDMX.setDirectionPin(RXTX_PIN);
  LXSerialDMX.startOutput();
  
  artnet.send_art_poll_reply(eUDP);
  blinkLED();
}


/************************************************************************

  The main loop checks for and reads packets from UDP ethernet socket
  connection.  When a packet is recieved, it is checked to see if
  it is valid Art-Net and the art DMXReceived function is called, sending
  the DMX values to the output.

*************************************************************************/

void loop() {
  if ( artnet.readDMXPacket(eUDP) ) {
     for (int i = 1; i <= artnet.numberOfSlots(); i++) {
        LXSerialDMX.setSlot(i , artnet.getSlot(i));
        blinkLED();
     }
  }
}

/*
    Revision History
    v1.0 initial release January 2014
    v1.1 added monitor LED to code and circuit
    v1.2 8/14/15 changed library support and clarified code
    v1.3 moved control of options to "includes" and "defines"
    v1.4 uses LXArtNet class which encapsulates Art-Net functionality
    v1.5 revised for refactor of DMX driver LXSerialDMX January-2016
*/
