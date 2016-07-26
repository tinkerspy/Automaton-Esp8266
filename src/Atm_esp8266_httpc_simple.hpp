#pragma once

#include <Automaton.h>
#include <Atm_esp8266.h>

class Atm_esp8266_httpc_simple: public Machine {

 public:
  enum { IDLE, CONNECT, SEND, WAIT, READ, DONE, TIMEOUT }; // STATES
  enum { EVT_LOST, EVT_AVAILABLE, EVT_TIMER, EVT_START, ELSE }; // EVENTS
  Atm_esp8266_httpc_simple( void ) : Machine() {};
  Atm_esp8266_httpc_simple& begin( const char * host, int port = 80 );
  Atm_esp8266_httpc_simple& trace( Stream & stream );
  Atm_esp8266_httpc_simple& trigger( int event );
  int state( void );
  Atm_esp8266_httpc_simple& onFinish( Machine& machine, int event = 0 );
  Atm_esp8266_httpc_simple& onFinish( atm_cb_push_t callback, int idx = 0 );
  Atm_esp8266_httpc_simple& onStart( Machine& machine, int event = 0 );
  Atm_esp8266_httpc_simple& onStart( atm_cb_push_t callback, int idx = 0 );
  Atm_esp8266_httpc_simple& start( int subCode = 0 );
  Atm_esp8266_httpc_simple& get( String path, String data = "", uint16_t maxResponseSize = 1024 );
  Atm_esp8266_httpc_simple& post( String path, String data = "", uint16_t maxResponseSize = 1024 );
  String content();
  String headers();
  int responseCode();
  bool is_success();
  bool is_redirect();
  bool is_error();

 private:
  enum { ENT_CONNECT, ENT_SEND, ENT_READ, ENT_DONE, ENT_TIMEOUT }; // ACTIONS
  enum { ON_FINISH, ON_START, CONN_MAX }; // CONNECTORS
  atm_connector connectors[CONN_MAX];
  String request_data;
  String request_path;
  String response_data;
  const char * client_host;
  int client_port;
  int sub_event;
  uint16_t max_response_size;
  WiFiClient client;
  atm_timer_millis timeout;
  bool post_flag;
  int event( int id ); 
  void action( int id ); 
};

/* 
Automaton::ATML::begin - Automaton Markup Language

<?xml version="1.0" encoding="UTF-8"?>
<machines>
  <machine name="Atm_esp8266_httpc_simple">
    <states>
      <IDLE index="0" sleep="1">
        <EVT_START>CONNECT</EVT_START>
      </IDLE>
      <CONNECT index="1" on_enter="ENT_CONNECT">
        <EVT_LOST>TIMEOUT</EVT_LOST>
        <ELSE>SEND</ELSE>
      </CONNECT>
      <SEND index="2" on_enter="ENT_SEND">
        <ELSE>WAIT</ELSE>
      </SEND>
      <WAIT index="3">
        <EVT_LOST>DONE</EVT_LOST>
        <EVT_AVAILABLE>READ</EVT_AVAILABLE>
        <EVT_TIMER>TIMEOUT</EVT_TIMER>
      </WAIT>
      <READ index="4" on_enter="ENT_READ">
        <EVT_LOST>DONE</EVT_LOST>
        <ELSE>WAIT</ELSE>
      </READ>
      <DONE index="5" on_enter="ENT_DONE">
        <ELSE>IDLE</ELSE>
      </DONE>
      <TIMEOUT index="6" on_enter="ENT_TIMEOUT">
        <ELSE>IDLE</ELSE>
      </TIMEOUT>
    </states>
    <events>
      <EVT_LOST index="0" access="PRIVATE"/>
      <EVT_AVAILABLE index="1" access="PRIVATE"/>
      <EVT_TIMER index="2" access="PRIVATE"/>
      <EVT_START index="3" access="PUBLIC"/>
    </events>
    <connectors>
      <FINISH autostore="0" broadcast="0" dir="PUSH" slots="1"/>
      <START autostore="0" broadcast="0" dir="PUSH" slots="1"/>
    </connectors>
    <methods>
    </methods>
  </machine>
</machines>

Automaton::ATML::end 
*/
