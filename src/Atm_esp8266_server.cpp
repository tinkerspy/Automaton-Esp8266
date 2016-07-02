#include "Atm_esp8266_server.hpp"

/* Add optional parameters for the state machine to begin()
 * Add extra initialization code
 */

Atm_esp8266_server& Atm_esp8266_server::begin() {
  // clang-format off
  const static state_t state_table[] PROGMEM = {
    /*               ON_ENTER  ON_LOOP  ON_EXIT  EVT_INCOMING  EVT_CMD  ELSE */
    /*    IDLE */    ENT_IDLE, LP_IDLE,      -1,      REQUEST,      -1,   -1,
    /* REQUEST */ ENT_REQUEST,      -1,      -1,           -1, COMMAND, IDLE,
    /* COMMAND */ ENT_COMMAND,      -1,      -1,           -1,      -1, IDLE,
  };
  // clang-format on
  Machine::begin( state_table, ELSE );
  server->onNotFound( [ this ] () {   
    server->uri().toCharArray( incoming_request, 80 );
  });
  server->begin();
  return *this;          
}

/* Add C++ code for each internally handled event (input) 
 * The code must return 1 to trigger the event
 */

int Atm_esp8266_server::event( int id ) {
  switch ( id ) {
    case EVT_INCOMING:
      return incoming_request[0] > 0;
    case EVT_CMD:
      return 1;
  }
  return 0;
}

/* Add C++ code for each action
 * This generates the 'output' for the state machine
 *
 * Available connectors:
 *   push( connectors, ON_COMMAND, 0, <v>, <up> );
 */

void Atm_esp8266_server::action( int id ) {
  switch ( id ) {
    case ENT_IDLE:
      return;
    case LP_IDLE:
      server->handleClient();
      return;
    case ENT_REQUEST:
      return;
    case ENT_COMMAND:
      push( connectors, ON_COMMAND, 0, lookup( incoming_request, commands ), 0 );
      incoming_request[0] = '\0';
      return;
  }
}

Atm_esp8266_server& Atm_esp8266_server::list( const char* cmds ) {
  commands = cmds;
  return *this;
}

Atm_esp8266_server& Atm_esp8266_server::send( int result, String content_type, String content ) {
  server->send( result, content_type, content );
  return *this;
}

String Atm_esp8266_server::uri( void ) {
  return incoming_request;
}

int Atm_esp8266_server::args( void ) {
  return server->args();
}

String Atm_esp8266_server::argName( int id ) {
  return server->argName( id );
}

String Atm_esp8266_server::arg( int id ) {
  return server->arg( id );
}

String Atm_esp8266_server::arg( String name ) {
   return server->arg( name );
}

int Atm_esp8266_server::lookup( char * cmd, const char* cmdlist ) {
  int cnt = 0;
  char* a = cmd;
  while ( cmdlist[0] != '\0' ) {
    while ( cmdlist[0] != '\0' && toupper( cmdlist[0] ) == toupper( a[0] ) ) {
      cmdlist++;
      a++;
    }
    if ( a[0] == '\0' && ( cmdlist[0] == ' ' || cmdlist[0] == '\0' ) ) return cnt;
    while ( cmdlist[0] != ' ' && cmdlist[0] != '\0' ) cmdlist++;
    cmdlist++;
    a = cmd;
    cnt++;
  }
  return -1;
}

/* Optionally override the default trigger() method
 * Control how your machine processes triggers
 */

Atm_esp8266_server& Atm_esp8266_server::trigger( int event ) {
  Machine::trigger( event );
  return *this;
}

/* Optionally override the default state() method
 * Control what the machine returns when another process requests its state
 */

int Atm_esp8266_server::state( void ) {
  return Machine::state();
}

/* Nothing customizable below this line                          
 ************************************************************************************************
*/

/* Public event methods
 *
 */

Atm_esp8266_server& Atm_esp8266_server::cmd() {
  trigger( EVT_CMD );
  return *this;
}

/*
 * onCommand() push connector variants ( slots 1, autostore 0, broadcast 0 )
 */

Atm_esp8266_server& Atm_esp8266_server::onCommand( Machine& machine, int event ) {
  onPush( connectors, ON_COMMAND, 0, 1, 1, machine, event );
  return *this;
}

Atm_esp8266_server& Atm_esp8266_server::onCommand( atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_COMMAND, 0, 1, 1, callback, idx );
  return *this;
}

/* State trace method
 * Sets the symbol table and the default logging method for serial monitoring
 */

Atm_esp8266_server& Atm_esp8266_server::trace( Stream & stream ) {
  Machine::setTrace( &stream, atm_serial_debug::trace,
    "ESP8266_SERVER\0EVT_INCOMING\0EVT_CMD\0ELSE\0IDLE\0REQUEST\0COMMAND" );
  return *this;
}



