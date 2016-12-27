/* LXArtNet2.cpp
   Copyright 2015 by Claude Heintz Design
   see: http://www.claudeheintzdesign.com/lx/opensource.html for license
      
	Art-Net(TM) Designed by and Copyright Artistic Licence Holdings Ltd.
*/

#include "LXArtNetEthernet2.h"

LXArtNet::LXArtNet ( IPAddress address )
{
	//zero buffer including _dmx_data[0] which is start code
    for (int n=0; n<ARTNET_BUFFER_MAX; n++) {
    	_packet_buffer[n] = 0;
    }
    
    _dmx_slots = 0;
    _universe = 0;
    _my_address = address;
    _broadcast_address = INADDR_NONE;
    _dmx_sender = INADDR_NONE;
    _sequence = 1;
}

LXArtNet::LXArtNet ( IPAddress address, IPAddress subnet_mask )
{
	//zero buffer including _dmx_data[0] which is start code
    for (int n=0; n<ARTNET_BUFFER_MAX; n++) {
    	_packet_buffer[n] = 0;
    }
    
    _dmx_slots = 0;
    _universe = 0;
    _my_address = address;
    uint32_t a = (uint32_t) address;
    uint32_t s = (uint32_t) subnet_mask;
    _broadcast_address = IPAddress(a | ~s);
    _dmx_sender = INADDR_NONE;
    _sequence = 1;
}

LXArtNet::~LXArtNet ( void )
{
    //no need for specific destructor
}

uint8_t  LXArtNet::universe ( void ) {
	return _universe;
}

void LXArtNet::setUniverse ( uint8_t u ) {
	_universe = u;
}

void LXArtNet::setSubnetUniverse ( uint8_t s, uint8_t u ) {
   _universe = ((s & 0x0f) << 4) | ( u & 0x0f);
}

void LXArtNet::setUniverseAddress ( uint8_t u ) {
	if ( u != 0x7f ) {
	   if ( u & 0x80 ) {
	     _universe = (_universe & 0xf0) | (u & 0x07);
	   }
	}
}

void LXArtNet::setSubnetAddress ( uint8_t s ) {
	if ( s != 0x7f ) {
	   if ( s & 0x80 ) {
	     _universe = (_universe & 0x0f) | ((s & 0x07) << 4);
	   }
	}
}

int  LXArtNet::numberOfSlots ( void ) {
	return _dmx_slots;
}

void LXArtNet::setNumberOfSlots ( int n ) {
	_dmx_slots = n;
}

uint8_t LXArtNet::getSlot ( int slot ) {
	return _packet_buffer[ARTNET_ADDRESS_OFFSET+slot];
}

void LXArtNet::setSlot ( int slot, uint8_t value ) {
	_packet_buffer[ARTNET_ADDRESS_OFFSET+slot] = value;
}

uint8_t* LXArtNet::dmxData( void ) {
	return &_packet_buffer[ARTNET_ADDRESS_OFFSET+1];
}

uint8_t LXArtNet::readDMXPacket ( EthernetUDP eUDP ) {
   uint16_t opcode = readArtNetPacket(eUDP);
   return ( opcode == ARTNET_ART_DMX );
}

/*
  attempts to read a packet from the supplied EthernetUDP object  
  returns opcode
  sends ArtPollReply with IPAddress if packet is ArtPoll
  replies directly to sender unless reply_ip != INADDR_NONE allowing specification of broadcast
  only returns ARTNET_ART_DMX if packet contained dmx data for this universe
  Packet size checks that packet is >= expected size to allow zero termination or padding
*/

uint16_t LXArtNet::readArtNetPacket ( EthernetUDP eUDP ) {
   uint16_t packetSize = eUDP.parsePacket();
   uint16_t opcode = ARTNET_NOP;
   if ( packetSize ) {
      packetSize = eUDP.read(_packet_buffer, ARTNET_BUFFER_MAX);
      _dmx_slots = 0;
      /* Buffer now may not contain dmx data for desired universe.
         After reading the packet into the buffer, check to make sure
         that it is an Art-Net packet and retrieve the opcode that
         tells what kind of message it is.                            */
      opcode = parse_header();
      switch ( opcode ) {
		   case ARTNET_ART_DMX:
		   	// ignore sequence[12], physical[13] and subnet/universe hi byte[15]
				if (( _packet_buffer[14] == _universe ) && ( _packet_buffer[11] >= 14 )) { //protocol version [10] hi byte [11] lo byte 
					packetSize -= 18;
					uint16_t slots = _packet_buffer[17];
					slots += _packet_buffer[16] << 8;
				   if ( packetSize >= slots ) {
						if ( (uint32_t)_dmx_sender == 0 ) {		//if first sender, remember address
							_dmx_sender = eUDP.remoteIP();
						}
						if ( _dmx_sender == eUDP.remoteIP() ) {
						   _dmx_slots = slots;
						}	// matched sender
					}		// matched size
				}			// matched universe
				if ( _dmx_slots == 0 ) {	//only set >0 if all of above matched
					opcode = ARTNET_NOP;
				}
				break;
			case ARTNET_ART_ADDRESS:
				if (( packetSize >= 108 ) && ( _packet_buffer[11] >= 14 )) {  //protocol version [10] hi byte [11] lo byte 
		   	   opcode = parse_art_address();
		   	   send_art_poll_reply( eUDP );
		   	}
		   	break;
			case ARTNET_ART_POLL:
				if (( packetSize >= 14 ) && ( _packet_buffer[11] >= 14 )) {
				   send_art_poll_reply( eUDP );
				}
				break;
		}
   }
   return opcode;
}

void LXArtNet::sendDMX ( EthernetUDP eUDP, IPAddress to_ip ) {
   _packet_buffer[0] = 'A';
   _packet_buffer[1] = 'r';
   _packet_buffer[2] = 't';
   _packet_buffer[3] = '-';
   _packet_buffer[4] = 'N';
   _packet_buffer[5] = 'e';
   _packet_buffer[6] = 't';
   _packet_buffer[7] = 0;
   _packet_buffer[8] = 0;        //op code lo-hi
   _packet_buffer[9] = 0x50;
   _packet_buffer[10] = 0;
   _packet_buffer[11] = 14;
   if ( _sequence == 0 ) {
     _sequence = 1;
   } else {
     _sequence++;
   }
   _packet_buffer[12] = _sequence;
   _packet_buffer[13] = 0;
   _packet_buffer[14] = _universe;
   _packet_buffer[15] = 0;
   _packet_buffer[16] = _dmx_slots >> 8;
   _packet_buffer[17] = _dmx_slots & 0xFF;
   //assume dmx data has been set
  
   eUDP.beginPacket(to_ip, ARTNET_PORT);
   eUDP.write(_packet_buffer, 18+_dmx_slots);
   eUDP.endPacket();
}

void LXArtNet::send_art_poll_reply( EthernetUDP eUDP ) {
  unsigned char  replyBuffer[ARTNET_REPLY_SIZE];
  int i;
  for ( i = 0; i < ARTNET_REPLY_SIZE; i++ ) {
    replyBuffer[i] = 0;
  }
  replyBuffer[0] = 'A';
  replyBuffer[1] = 'r';
  replyBuffer[2] = 't';
  replyBuffer[3] = '-';
  replyBuffer[4] = 'N';
  replyBuffer[5] = 'e';
  replyBuffer[6] = 't';
  replyBuffer[7] = 0;
  replyBuffer[8] = 0;        // op code lo-hi
  replyBuffer[9] = 0x21;
  replyBuffer[10] = ((uint32_t)_my_address) & 0xff;      //ip address
  replyBuffer[11] = ((uint32_t)_my_address) >> 8;
  replyBuffer[12] = ((uint32_t)_my_address) >> 16;
  replyBuffer[13] = ((uint32_t)_my_address) >>24;
  replyBuffer[14] = 0x36;    // port lo first always 0x1936
  replyBuffer[15] = 0x19;
  replyBuffer[16] = 0;       // firmware hi-lo
  replyBuffer[17] = 0;
  replyBuffer[18] = 0;       // subnet hi-lo
  replyBuffer[19] = 0;
  replyBuffer[20] = 0;       // oem hi-lo
  replyBuffer[21] = 0;
  replyBuffer[22] = 0;       // ubea
  replyBuffer[23] = 0;       // status
  replyBuffer[24] = 0x50;    //     Mfg Code
  replyBuffer[25] = 0x12;    //     seems DMX workshop reads these bytes backwards
  replyBuffer[26] = 'A';     // short name
  replyBuffer[27] = 'r';
  replyBuffer[28] = 'd';
  replyBuffer[29] = 'u';
  replyBuffer[30] = 'i';
  replyBuffer[31] = 'n';
  replyBuffer[32] = 'o';
  replyBuffer[33] =  0;
  replyBuffer[44] = 'A';     // long name
  replyBuffer[45] = 'r';
  replyBuffer[46] = 'd';
  replyBuffer[47] = 'u';
  replyBuffer[48] = 'i';
  replyBuffer[49] = 'n';
  replyBuffer[50] = 'o';
  replyBuffer[51] =  0;
  replyBuffer[173] = 1;    // number of ports
  replyBuffer[174] = 128;  // can output from network
  replyBuffer[182] = 128;  //  good output... change if error
  replyBuffer[190] = _universe;
  
  IPAddress a = _broadcast_address;
  if ( a == INADDR_NONE ) {
    a = eUDP.remoteIP();   // reply directly if no broadcast address is supplied
  }
  eUDP.beginPacket(a, ARTNET_PORT);
  eUDP.write(replyBuffer, ARTNET_REPLY_SIZE);
  eUDP.endPacket();
}

uint16_t LXArtNet::parse_header( void ) {
  if ( strcmp((const char*)_packet_buffer, "Art-Net") == 0 ) {
    return _packet_buffer[9] * 256 + _packet_buffer[8];  //opcode lo byte first
  }
  return ARTNET_NOP;
}

/*
  reads an ARTNET_ART_ADDRESS packet
  can set output universe
  can cancel merge which resets address of dmx sender
     (after first ArtDmx packet, only packets from the same sender are accepted
     until a cancel merge command is received)
*/

uint16_t LXArtNet::parse_art_address( void ) {
	//[12] to [29] short name <= 18 bytes
	//[30] to [93] long name  <= 64 bytes
	//[94][95][96][97]                  input universe   ch 1 to 4
	//[98][99][100][101]               output universe   ch 1 to 4
	setUniverseAddress(_packet_buffer[98]);
	//[102][103][104][105]                      subnet   ch 1 to 4
	setSubnetAddress(_packet_buffer[102]);
	//[106]                                   reserved
	uint8_t command = _packet_buffer[107]; // command
	switch ( command ) {
	   case 0x01:	//cancel merge: resets ip address used to identify dmx sender
	   	_dmx_sender = (uint32_t)0;
	   	break;
	   case 0x90:	//clear buffer
	   	_dmx_sender = (uint32_t)0;
	   	for(int j=18; j<ARTNET_BUFFER_MAX; j++) {
	   	   _packet_buffer[j] = 0;
	   	}
	   	_dmx_slots = 512;
	   	return ARTNET_ART_DMX;	// return ARTNET_ART_DMX so function calling readPacket
	   	   						   // knows there has been a change in levels
	   	break;
	}
	return ARTNET_ART_ADDRESS;
}