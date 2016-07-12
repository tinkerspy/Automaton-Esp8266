#include "Atm_esp8266_httpd_simple.hpp"

/* Add optional parameters for the state machine to begin()
 * Add extra initialization code
 */

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::begin() {
  // clang-format off
  const static state_t state_table[] PROGMEM = {
    /*               ON_ENTER    ON_LOOP  ON_EXIT  EVT_INCOMING  EVT_CMD  EVT_START    ELSE */
    /*    IDLE */    ENT_IDLE,        -1,      -1,           -1,      -1,    ACTIVE,     -1,
    /*  ACTIVE */  ENT_ACTIVE, LP_ACTIVE,      -1,      REQUEST,      -1,        -1,     -1,
    /* REQUEST */ ENT_REQUEST,        -1,      -1,           -1, COMMAND,        -1, ACTIVE,
    /* COMMAND */ ENT_COMMAND,        -1,      -1,           -1,      -1,        -1, ACTIVE,
  };
  // clang-format on
  Machine::begin( state_table, ELSE );
  return *this;          
}

/* Add C++ code for each internally handled event (input) 
 * The code must return 1 to trigger the event
 */

int Atm_esp8266_httpd_simple::event( int id ) {
  switch ( id ) {
    case EVT_INCOMING:
      return incoming_request[0] > 0;
    case EVT_CMD:
      return 1;
    case EVT_START:
      return 0;
  }
  return 0;
}

/* Add C++ code for each action
 * This generates the 'output' for the state machine
 *
 * Available connectors:
 *   push( connectors, ON_COMMAND, 0, <v>, <up> );
 */

void Atm_esp8266_httpd_simple::action( int id ) {
  uint8_t matched = 0;
  switch ( id ) {
    case ENT_IDLE:
      return;
    case LP_ACTIVE:
      server->handleClient();
      return;
    case ENT_ACTIVE:
      server->onNotFound( [ this ] () {   
        server->uri().toCharArray( incoming_request, 80 );
      });
      server->begin();
      return;
    case ENT_REQUEST:
      return;
    case ENT_COMMAND:
      for ( int i = 0; i < MAX_HANDLERS; i++ ) {
        if ( cmd_connectors[i].mode() > 0 && cmd_strings[i].equals( incoming_request ) ) {
          matched++;
          no_output = 1;
          cmd_connectors[i].push();
        }
      }
      if ( !matched && connectors[ON_COMMAND].mode() > 0 ) { // No default output here! add?  
        no_output = 1;
        push( connectors, ON_COMMAND, 0, commands ? lookup( incoming_request, commands ) : 0, 0 );
        matched++;
      }
      if ( matched && no_output ) { // Send default output
        send( reply_string && reply_string[0] > 0 ? reply_string : "OK" );
      }
      incoming_request[0] = '\0';
      server->client().stopAll();
      return;
  }
}

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::list( const char* cmds ) {
  commands = cmds;
  return *this;
}

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::send( int result, String content_type, String content ) {
  server->send( result, content_type, content );
  no_output = 0;
  return *this;
}

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::send( String content ) {
  server->send( 200, "text/html", content );
  no_output = 0;
  return *this;
}

String Atm_esp8266_httpd_simple::uri( void ) {
  return incoming_request;
}

int Atm_esp8266_httpd_simple::args( void ) {
  return server->args();
}

String Atm_esp8266_httpd_simple::argName( int id ) {
  return server->argName( id );
}

String Atm_esp8266_httpd_simple::arg( int id ) {
  return server->arg( id );
}

String Atm_esp8266_httpd_simple::arg( String name ) {
   return server->arg( name );
}

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::reply( const char * s ) {
  reply_string = s;
  return *this;
}

int Atm_esp8266_httpd_simple::lookup( char * cmd, const char* cmdlist ) {
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

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::onRequest( String cmd, Machine& machine, int event ) {
  for ( int i = 0; i < MAX_HANDLERS; i++ ) {
    if ( cmd_connectors[i].mode() == 0 ) {
      cmd_connectors[i].set( &machine, event );
      cmd_strings[i] = cmd;
      return *this;
    }
  }
  return *this;
}

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::onRequest( String cmd, atm_cb_push_t callback, int idx /* = 0 */ ) {
  for ( int i = 0; i < MAX_HANDLERS; i++ ) {
    if ( cmd_connectors[i].mode() == 0 ) {
      cmd_connectors[i].set( callback, idx );
      cmd_strings[i] = cmd;
      return *this;
    }
  }
  return *this;
}

/* Optionally override the default trigger() method
 * Control how your machine processes triggers
 */

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::trigger( int event ) {
  Machine::trigger( event );
  return *this;
}

/* Optionally override the default state() method
 * Control what the machine returns when another process requests its state
 */

int Atm_esp8266_httpd_simple::state( void ) {
  return Machine::state();
}

/* Nothing customizable below this line                          
 ************************************************************************************************
*/

/* Public event methods
 *
 */

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::cmd() {
  trigger( EVT_CMD );
  return *this;
}

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::start() {
  trigger( EVT_START );
  return *this;
}

/*
 * onRequest() push connector variants ( slots 1, autostore 0, broadcast 0 )
 */

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::onRequest( Machine& machine, int event ) {
  onPush( connectors, ON_COMMAND, 0, 1, 1, machine, event );
  return *this;
}

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::onRequest( atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_COMMAND, 0, 1, 1, callback, idx );
  return *this;
}

/* State trace method
 * Sets the symbol table and the default logging method for serial monitoring
 */

Atm_esp8266_httpd_simple& Atm_esp8266_httpd_simple::trace( Stream & stream ) {
  Machine::setTrace( &stream, atm_serial_debug::trace,
    "ESP8266_HTTPD_SIMPLE\0EVT_INCOMING\0EVT_CMD\0EVT_START\0ELSE\0IDLE\0ACTIVE\0REQUEST\0COMMAND" );
  return *this;
}




