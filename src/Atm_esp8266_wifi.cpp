#include "Atm_esp8266.h"

// The global wifi object

Atm_esp8266_wifi wifi;

/* Add optional parameters for the state machine to begin()
 * Add extra initialization code
 */

Atm_esp8266_wifi& Atm_esp8266_wifi::begin( const char ssid[], const char password[] ) {
  // clang-format off
  const static state_t state_table[] PROGMEM = {
    /*               ON_ENTER  ON_LOOP  ON_EXIT  EVT_START  EVT_STOP  EVT_TOGGLE  EVT_TIMER  EVT_CONNECT  EVT_DISCONNECT  EVT_PACKET   ELSE */
    /*    IDLE */          -1,      -1,      -1,     START,       -1,      START,        -1,          -1,             -1,         -1,    -1,
    /*   START */   ENT_START,      -1,      -1,        -1,       -1,         -1,        -1,          -1,             -1,         -1,    WAIT,
    /*    WAIT */          -1,      -1,      -1,        -1,       -1,         -1,     CHECK,          -1,             -1,         -1,      -1,
    /*   CHECK */          -1,      -1,      -1,        -1,       -1,         -1,        -1,      ACTIVE,             -1,         -1,    WAIT,
    /*  ACTIVE */  ENT_ACTIVE,      -1,      -1,        -1,       -1,         -1,        -1,          -1,        DISCONN,         -1, WAITPKT,
    /* WAITPKT */          -1,      -1,      -1,        -1,       -1,         -1,        -1,          -1,        DISCONN,    PKTRCVD,      -1,
    /* PKTRCVD */ ENT_PKTRCVD,      -1,      -1,        -1,       -1,         -1,        -1,          -1,             -1,         -1, WAITPKT,
    /* DISCONN */ ENT_DISCONN,      -1,      -1,        -1,       -1,         -1,        -1,          -1,             -1,         -1,    WAIT,
  };
  // clang-format on
  Machine::begin( state_table, ELSE );
  WiFi.begin( ssid, password );
  WiFi.mode( WIFI_STA );
  timer.set( 500 );
  indicator = -1;
  return *this;          
}

/*
 * Converts a string to a 32 bit (uint32_t) hash using the first 8 chars of sha1()
 * then registers the result as a 32 bit mirror node-id
 */

uint16_t Atm_esp8266_wifi::reg( const char name[], atm_cb_mirror_t callback  ) {
  char buf[9];
  sha1( name ).toCharArray( buf, 9 ); 
  return reg( strtoul( buf, NULL, 16 ), callback );
}

/*
 * Registers a 32 bit mirror node-id 
 */

uint16_t Atm_esp8266_wifi::reg( uint32_t v, atm_cb_mirror_t callback ) {
  int id = 0;
  for ( int i = 0; i < ATM_WIFI_MAX_NODES; i++ ) {
    if ( mirror_regs[i].flags == 0 ) {
      id = i;
    }
  }
  mirror_regs[id].callback = callback;
  mirror_regs[id].address = v;
  mirror_regs[id].flags = 1;
/*
  Serial.print( "Stored " ); 
  Serial.print( v, HEX ); 
  Serial.print( " at " );
  Serial.println( id ); 
*/
  return id;  
}

// WARNING: Filter own packets!
// WARNING: Do not send packets as a result of other packets!
// WARNING: Ignore packets with node-id 0!

// Authorative node is the one that sent the last original state and keeps resending
// do not resnd after receiving a more recent update for the node-id ( one sender/many receivers )
// switch to sender whenever the node's own state changes autonomously
// mirrors can be dictators: (other nodes *must* follow) {is this really necessary?}

/* 
 * Broadcasts a 16 bit integer value over the local network packaged with
 * a pre-registered 32 bit mirror node-id 
 */

Atm_esp8266_wifi & Atm_esp8266_wifi::transmit( uint16_t nodeId, char op, int v ) { 
  atm_mirror_packet_t packet;
  packet.address = mirror_regs[nodeId].address;
  packet.value = v;
  packet.operation = op;
  packet.signature = 'A';
  uint8_t checksum = 0;
  for ( uint8_t i = 0; i < ( sizeof( packet.b ) - 1 ); i++ ) {
    checksum += packet.b[i]; 
  }
  packet.checksum = checksum;
  udp.beginPacket( wifi.broadcastAddress(), ATM_WIFI_MIRROR_REMOTE_PORT ); 
  udp.write( packet.b, sizeof( packet.b ) );
  udp.endPacket();
  return *this;  
}


/* Add C++ code for each internally handled event (input) 
 * The code must return 1 to trigger the event
 */
 
 
int Atm_esp8266_wifi::event( int id ) {
  switch ( id ) {
    case EVT_TIMER:
      return timer.expired( this );
    case EVT_CONNECT:
      return WiFi.status() == WL_CONNECTED;
    case EVT_DISCONNECT:
      return WiFi.status() != WL_CONNECTED;
    case EVT_PACKET:
      return udp.parsePacket() > 0;
  }
  return 0;
}

/* Add C++ code for each action
 * This generates the 'output' for the state machine
 *
 * Available connectors:
 *   push( connectors, ON_CHANGE, <sub>, <v>, <up> );
 */

void Atm_esp8266_wifi::action( int id ) {
  atm_mirror_packet_t pkt;
  switch ( id ) {
    case ENT_START:
      return;
    case ENT_ACTIVE:
      push( connectors, ON_CHANGE, true, 1, 0 );
      if ( indicator > -1 ) digitalWrite( indicator, !HIGH != !indicatorActiveLow );
      udp.begin( ATM_WIFI_MIRROR_REMOTE_PORT );
      return;
    case ENT_DISCONN:
      push( connectors, ON_CHANGE, false, 0, 0 );
      if ( indicator > -1 ) digitalWrite( indicator, !LOW != !indicatorActiveLow );
      return;
    case ENT_PKTRCVD:
      udp.read( pkt.b, sizeof( pkt.b ) );
      Serial.println( udp.remoteIP() );
      if ( pkt.signature == 'A' ) { // Check #1: signature 'A'
        uint8_t checksum = 0;
        for ( uint8_t i = 0; i < ( sizeof( pkt.b ) - 1 ); i++ ) {
          checksum += pkt.b[i]; 
        }
        if ( pkt.checksum == checksum ) { // Check #2: checksum match
          int id = -1;
          for ( int i = 0; i < ATM_WIFI_MAX_NODES; i++ ) { // Check #3: address match
            if ( mirror_regs[i].address == pkt.address ) {
              id = i;
            }
          }
          if ( id > -1 ) {
            ( *mirror_regs[id].callback )( pkt.value ); // Send sender IP as well
          } else {
            Serial.println( "Address unknown" );
          }
        } else {
          Serial.print( "Failed checksum:" );
          Serial.print( checksum, HEX );
          Serial.print( " != " );
          Serial.println( pkt.checksum, HEX );
        }       
      } else {
        Serial.println( "Failed signature" );
      }
      return;
  }
}

IPAddress Atm_esp8266_wifi::ip( void ) {
  return WiFi.localIP();
}

IPAddress Atm_esp8266_wifi::netmask( void ) {
  return WiFi.subnetMask();
}

IPAddress Atm_esp8266_wifi::broadcastAddress( void ) {
  IPAddress r;
  IPAddress ip = this->ip();
  IPAddress netmask = this->netmask();
  for ( uint8_t i = 0; i < 4; i++ ) {
    r[i] = ip[i] | ~netmask[i];
  }
  return r;
}

Atm_esp8266_wifi& Atm_esp8266_wifi::led( int led, bool activeLow /* = false */ ) {
  indicator = led;
  indicatorActiveLow = activeLow;
  pinMode( indicator, OUTPUT );
  digitalWrite( indicator, !LOW != !indicatorActiveLow );
  return *this;
}

/* Optionally override the default trigger() method
 * Control how your machine processes triggers
 */

Atm_esp8266_wifi& Atm_esp8266_wifi::trigger( int event ) {
  Machine::trigger( event );
  return *this;
}

/* Optionally override the default state() method
 * Control what the machine returns when another process requests its state
 */

int Atm_esp8266_wifi::state( void ) {
  return current == ACTIVE ? 1 : 0;
}

/* Nothing customizable below this line                          
 ************************************************************************************************
*/

/* Public event methods
 *
 */

Atm_esp8266_wifi& Atm_esp8266_wifi::start() {
  trigger( EVT_START );
  return *this;
}

Atm_esp8266_wifi& Atm_esp8266_wifi::stop() {
  trigger( EVT_STOP );
  return *this;
}

Atm_esp8266_wifi& Atm_esp8266_wifi::toggle() {
  trigger( EVT_TOGGLE );
  return *this;
}

/*
 * onChange() push connector variants ( slots 2, autostore 0, broadcast 0 )
 */

Atm_esp8266_wifi& Atm_esp8266_wifi::onChange( Machine& machine, int event ) {
  onPush( connectors, ON_CHANGE, 0, 2, 1, machine, event );
  return *this;
}

Atm_esp8266_wifi& Atm_esp8266_wifi::onChange( atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_CHANGE, 0, 2, 1, callback, idx );
  return *this;
}

Atm_esp8266_wifi& Atm_esp8266_wifi::onChange( int sub, Machine& machine, int event ) {
  onPush( connectors, ON_CHANGE, sub, 2, 0, machine, event );
  return *this;
}

Atm_esp8266_wifi& Atm_esp8266_wifi::onChange( int sub, atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_CHANGE, sub, 2, 0, callback, idx );
  return *this;
}

/* State trace method
 * Sets the symbol table and the default logging method for serial monitoring
 */

Atm_esp8266_wifi& Atm_esp8266_wifi::trace( Stream & stream ) {
  Machine::setTrace( &stream, atm_serial_debug::trace,
    "ESP8266_WIFI\0EVT_START\0EVT_STOP\0EVT_TOGGLE\0EVT_TIMER\0EVT_CONNECT\0EVT_DISCONNECT\0EVT_PACKET\0ELSE\0IDLE\0START\0WAIT\0CHECK\0ACTIVE\0WAITPKT\0PKTRCVD\0DISCONN" );
  return *this;
}




