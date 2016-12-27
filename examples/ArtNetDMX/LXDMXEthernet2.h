/* LXDMXEthernet2.h
   Copyright 2015 by Claude Heintz Design
   see: http://www.claudeheintzdesign.com/lx/opensource.html for license
*/

#ifndef LXDMXETHERNET_H
#define LXDMXETHERNET_H

#include <Arduino.h>
#include <EthernetUdp2.h>
#include <inttypes.h>

/*!   
@class LXDMXEthernet
@abstract
   LXDMXEthernet encapsulates functionality for sending and receiving DMX over ethernet.
   It is a virtual class with concrete subclasses LXArtNet and LXSACN which specifically
   implement the Artistic Licence Art-Net and PLASA sACN 1.31 protocols.
   
   Note:  LXDMXEthernet2_library requires
          Ethernet2 library used with Arduino Ethernet Shield 2
          Use LXDMXEthernet_library with the original Ethernet shield and Ethernet Library
          
          For multicast, EthernetUDP2.h and EthernetUDP2.cpp in the Ethernet2 library
          must be modified to add the beginMulticast method.
          See the code at the bottom of LXDMXEthernet2.h
*/

class LXDMXEthernet {

  public:
/*!
* @brief UDP port used by protocol
*/
   virtual uint16_t dmxPort      ( void );

/*!
* @brief universe for sending and receiving dmx
* @discussion First universe is zero for Art-Net and one for sACN E1.31.
* @return universe 0/1-255
*/
   virtual uint8_t universe      ( void );
/*!
* @brief set universe for sending and receiving
* @discussion First universe is zero for Art-Net and one for sACN E1.31.
* @param u universe 0/1-255
*/
   virtual void    setUniverse   ( uint8_t u );
 
 /*!
 * @brief number of slots (aka addresses or channels)
 * @discussion Should be minimum of ~24 depending on actual output speed.  Max of 512.
 * @return number of slots/addresses/channels
 */  
   virtual int  numberOfSlots    ( void );
 /*!
 * @brief set number of slots (aka addresses or channels)
 * @discussion Should be minimum of ~24 depending on actual output speed.  Max of 512.
 * @param slots 1 to 512
 */  
   virtual void setNumberOfSlots ( int n );
 /*!
 * @brief get level data from slot/address/channel
 * @param slot 1 to 512
 * @return level for slot (0-255)
 */  
   virtual uint8_t  getSlot      ( int slot );
 /*!
 * @brief set level data (0-255) for slot/address/channel
 * @param slot 1 to 512
 * @param level 0 to 255
 */  
   virtual void     setSlot      ( int slot, uint8_t value );
 /*!
 * @brief direct pointer to dmx buffer uint8_t[]
 * @return uint8_t* to dmx data buffer
 */  
   virtual uint8_t* dmxData      ( void );

 /*!
 * @brief read UDP packet
 * @return 1 if packet contains dmx
 */   
   virtual uint8_t readDMXPacket ( EthernetUDP eUDP );
   virtual void    sendDMX       ( EthernetUDP eUDP, IPAddress to_ip );
};

#endif // ifndef LXDMXETHERNET_H


/*
@code
******************  multicast methods added to Ethernet2 library *******************

see  https://github.com/aallan/Arduino/blob/3811729f82ef05f3ae43341022e7b65a92d333a2/libraries/Ethernet/EthernetUdp.cpp

1) Locate the Ethernet2 library
   If you have installed this library, it will be located with other libraries in the <sketchbook>/libraries folder.
   If it is included in the IDE, you will need to search within the IDE files to find the libraries folder.
   
2) Duplicate the Ethernet2 library in your sketchbook folder (~/Documents/Arduino/libraries)
   Add the required method as follows:

3)  In EthernetUDPh or EthernetUDP2.h  add the beginMulticast method declaration:
...
  EthernetUDP();  // Constructor
  virtual uint8_t begin(uint16_t);	// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
  uint8_t beginMulticast(IPAddress ip, uint16_t port);  //############## added to standard library
...

4)  In EthernetUDP2.cpp add the method body:

uint8_t EthernetUDP::beginMulticast(IPAddress ip, uint16_t port) {
    if (_sock != MAX_SOCK_NUM)
        return 0;
    
    for (int i = 0; i < MAX_SOCK_NUM; i++) {
        uint8_t s = w5500.readSnSR(i);
        if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT) {
            _sock = i;
            break;
        }
    }
    
    if (_sock == MAX_SOCK_NUM)
        return 0;
    
    
    // Calculate MAC address from Multicast IP Address
    byte mac[] = {  0x01, 0x00, 0x5E, 0x00, 0x00, 0x00 };
    
    mac[3] = ip[1] & 0x7F;
    mac[4] = ip[2];
    mac[5] = ip[3];
    
    w5500.writeSnDIPR(_sock, rawIPAddress(ip));   //239.255.0.1
    w5500.writeSnDPORT(_sock, port);
    w5500.writeSnDHAR(_sock,mac);
    
    _remaining = 0;
    
    socket(_sock, SnMR::UDP, port, SnMR::MULTI);
    
    return 1;
}

**************************************************************************************
*/