#include "Atm_esp8266_httpc_simple.hpp"

/* Add optional parameters for the state machine to begin()
 * Add extra initialization code
 */

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::begin( const char * host, int port /* = 80 */ ) {
  // clang-format off
  const static state_t state_table[] PROGMEM = {
    /*               ON_ENTER    ON_LOOP  ON_EXIT  EVT_LOST  EVT_AVAILABLE  EVT_TIMER  EVT_START  ELSE */
    /*    IDLE */          -1, ATM_SLEEP,      -1,       -1,            -1,        -1,   CONNECT,   -1,
    /* CONNECT */ ENT_CONNECT,        -1,      -1,  TIMEOUT,            -1,        -1,        -1, SEND,
    /*    SEND */    ENT_SEND,        -1,      -1,       -1,            -1,        -1,        -1, WAIT,
    /*    WAIT */          -1,        -1,      -1,     DONE,          READ,   TIMEOUT,        -1,   -1,
    /*    READ */    ENT_READ,        -1,      -1,     DONE,            -1,        -1,        -1, WAIT,
    /*    DONE */    ENT_DONE,        -1,      -1,       -1,            -1,        -1,        -1, IDLE,
    /* TIMEOUT */ ENT_TIMEOUT,        -1,      -1,       -1,            -1,        -1,        -1, IDLE,
  };
  // clang-format on
  Machine::begin( state_table, ELSE );
  client_host = host;
  client_port = port;
  timeout.set( 60000 );
  max_response_size = 1024;
  return *this;          
}

/* Add C++ code for each internally handled event (input) 
 * The code must return 1 to trigger the event
 */

int Atm_esp8266_httpc_simple::event( int id ) {
  switch ( id ) {
    case EVT_LOST:
      return !client.connected();
    case EVT_AVAILABLE:
      return client.available();
    case EVT_TIMER:
      return timeout.expired( this );
  }
  return 0;
}

/* Add C++ code for each action
 * This generates the 'output' for the state machine
 *
 * Available connectors:
 *   push( connectors, ON_FINISH, 0, <v>, <up> );
 */

void Atm_esp8266_httpc_simple::action( int id ) {
  String request = "";
  switch ( id ) {
    case ENT_CONNECT:
      client.connect( client_host, client_port );
      return;
    case ENT_SEND:
      push( connectors, ON_START, 0, sub_event, 0 );
      response_data = "";
      if ( post_flag ) {
        request.concat( "POST " );
        request.concat( request_path );
        request.concat( " HTTP/1.1\r\n" );
        request.concat( "Host: " );
        request.concat( client_host );
        request.concat( "\r\nCache-Control: no-cache\r\n");
        request.concat( "Content-Type: application/x-www-form-urlencoded\r\n" );
        request.concat( "Content-Length: " );
        request.concat( request_data.length() );
        request.concat( "\r\n\r\n" );
        request.concat( request_data );           
        request.concat( "\r\n" );
      } else {        
        request.concat( "GET " );
        request.concat( request_path );
        request.concat( "?" );
        request.concat( request_data );
        request.concat( " HTTP/1.1\r\nHost: " );
        request.concat( client_host );
        request.concat( "\r\nCache-Control: no-cache\r\n\r\n");
      }
      client.print( request );
      return;
    case ENT_READ:
      while ( client.available() ) {
        if ( response_data.length() < max_response_size ) {
          response_data += (char) client.read();
        }
      }
      return;
    case ENT_DONE:
      push( connectors, ON_FINISH, 0, sub_event, 0 );
      return;
    case ENT_TIMEOUT:
      response_data = "HTTP/1.1 400 Fail\r\n";
      return;
  }
}

/* Optionally override the default trigger() method
 * Control how your machine processes triggers
 */

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::trigger( int event ) {
  if ( event >= EVT_START ) {
    sub_event = event - EVT_START;
    Machine::trigger( EVT_START );
  }
  return *this;
}

/* Optionally override the default state() method
 * Control what the machine returns when another process requests its state
 */

int Atm_esp8266_httpc_simple::state( void ) {
  return Machine::state();
}

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::get( String path, String data, uint16_t maxResponseSize /* = 1024 */ ) {
  request_path = path;
  request_data = data;
  max_response_size = maxResponseSize;
  post_flag = false;
  return *this;
}

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::post( String path, String data, uint16_t maxResponseSize /* = 1024 */  ) {
  request_path = path;
  request_data = data;
  max_response_size = maxResponseSize;
  post_flag = true;
  return *this;
}

String Atm_esp8266_httpc_simple::content() {
  return response_data.substring( response_data.indexOf( "\r\n\r\n" ) + 4 );
}

String Atm_esp8266_httpc_simple::headers() {
  return response_data.substring( 0, response_data.indexOf( "\r\n\r\n" ) );
}

int Atm_esp8266_httpc_simple::responseCode() {
  return response_data.substring( response_data.indexOf( " " ) + 1 ).toInt();
}

bool Atm_esp8266_httpc_simple::is_success() {
  int r = responseCode();
  return r >= 200 && r < 300;
}

bool Atm_esp8266_httpc_simple::is_redirect() {
  int r = responseCode();
  return r >= 300 && r < 400;
}

bool Atm_esp8266_httpc_simple::is_error() {
  int r = responseCode();
  return r >= 400 && r < 500;
}


/* Nothing customizable below this line                          
 ************************************************************************************************
*/

/* Public event methods
 *
 */

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::start( int subCode /* = 0 */ ) {
  trigger( EVT_START + subCode );
  return *this;
}

/*
 * onFinish() push connector variants ( slots 1, autostore 0, broadcast 0 )
 */

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::onFinish( Machine& machine, int event ) {
  onPush( connectors, ON_FINISH, 0, 1, 1, machine, event );
  return *this;
}

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::onFinish( atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_FINISH, 0, 1, 1, callback, idx );
  return *this;
}

/*
 * onStart() push connector variants ( slots 1, autostore 0, broadcast 0 )
 */

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::onStart( Machine& machine, int event ) {
  onPush( connectors, ON_START, 0, 1, 1, machine, event );
  return *this;
}

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::onStart( atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_START, 0, 1, 1, callback, idx );
  return *this;
}

/* State trace method
 * Sets the symbol table and the default logging method for serial monitoring
 */

Atm_esp8266_httpc_simple& Atm_esp8266_httpc_simple::trace( Stream & stream ) {
  Machine::setTrace( &stream, atm_serial_debug::trace,
    "ESP8266_HTTPC_SIMPLE\0EVT_LOST\0EVT_AVAILABLE\0EVT_TIMER\0EVT_START\0ELSE\0IDLE\0CONNECT\0SEND\0WAIT\0READ\0DONE\0TIMEOUT" );
  return *this;
}

