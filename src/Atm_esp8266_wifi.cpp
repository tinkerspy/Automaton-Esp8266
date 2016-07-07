#include "Atm_esp8266.h"

/* Add optional parameters for the state machine to begin()
 * Add extra initialization code
 */

Atm_esp8266_wifi& Atm_esp8266_wifi::begin( const char ssid[], const char password[] ) {
  // clang-format off
  const static state_t state_table[] PROGMEM = {
    /*               ON_ENTER  ON_LOOP  ON_EXIT  EVT_START  EVT_STOP  EVT_TOGGLE  EVT_TIMER  EVT_CONNECT  EVT_DISCONNECT  ELSE */
    /*    IDLE */          -1,      -1,      -1,     START,       -1,      START,        -1,          -1,             -1,   -1,
    /*   START */   ENT_START,      -1,      -1,        -1,       -1,         -1,        -1,          -1,             -1, WAIT,
    /*    WAIT */          -1,      -1,      -1,        -1,       -1,         -1,     CHECK,          -1,             -1,   -1,
    /*   CHECK */          -1,      -1,      -1,        -1,       -1,         -1,        -1,      ACTIVE,             -1, WAIT,
    /*  ACTIVE */  ENT_ACTIVE,      -1,      -1,        -1,       -1,         -1,        -1,          -1,        DISCONN,   -1,
    /* DISCONN */ ENT_DISCONN,      -1,      -1,        -1,       -1,         -1,        -1,          -1,             -1, WAIT,
  };
  // clang-format on
  Machine::begin( state_table, ELSE );
  WiFi.begin( ssid, password );
  timer.set( 500 );
  indicator = -1;
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
  switch ( id ) {
    case ENT_START:
      return;
    case ENT_ACTIVE:
      push( connectors, ON_CHANGE, true, 1, 0 );
      if ( indicator > -1 ) digitalWrite( indicator, !HIGH != !indicatorActiveLow );
      return;
    case ENT_DISCONN:
      push( connectors, ON_CHANGE, false, 0, 0 );
      if ( indicator > -1 ) digitalWrite( indicator, !LOW != !indicatorActiveLow );
      return;
  }
}

IPAddress Atm_esp8266_wifi::ip( void ) {
  return WiFi.localIP();
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
    "ESP8266_WIFI\0EVT_START\0EVT_STOP\0EVT_TOGGLE\0EVT_TIMER\0EVT_CONNECT\0EVT_DISCONNECT\0ELSE\0IDLE\0START\0WAIT\0CHECK\0ACTIVE\0DISCONN" );
  return *this;
}




