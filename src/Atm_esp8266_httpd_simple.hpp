#pragma once

#include <Automaton.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define MAX_HANDLERS 16

class Atm_esp8266_httpd_simple: public Machine {

 public:
  enum { IDLE, ACTIVE, REQUEST, COMMAND }; // STATES
  enum { EVT_INCOMING, EVT_CMD, EVT_START, ELSE }; // EVENTS
  Atm_esp8266_httpd_simple( int port ) : Machine() { static ESP8266WebServer esp_server( port ); server = &esp_server; };
  Atm_esp8266_httpd_simple& begin( void );
  Atm_esp8266_httpd_simple& trace( Stream & stream );
  Atm_esp8266_httpd_simple& trigger( int event );
  int state( void );
  Atm_esp8266_httpd_simple& onRequest( Machine& machine, int event = 0 );
  Atm_esp8266_httpd_simple& onRequest( atm_cb_push_t callback = 0, int idx = 0 );
  Atm_esp8266_httpd_simple& onRequest( String cmd, Machine& machine, int event = 0 );
  Atm_esp8266_httpd_simple& onRequest( String cmd, atm_cb_push_t callback = 0, int idx = 0 );
  Atm_esp8266_httpd_simple& start( void );
  Atm_esp8266_httpd_simple& list( const char* cmds );
  Atm_esp8266_httpd_simple& send( int result, String content_type, String content );
  Atm_esp8266_httpd_simple& send( String content );
  String uri( void );
  int args( void );
  String argName( int id );
  String arg( int id );
  String arg( String name );
  Atm_esp8266_httpd_simple& reply( const char * s );
  Atm_esp8266_httpd_simple& cmd( void );

 private:
  enum { ENT_IDLE, ENT_ACTIVE, LP_ACTIVE, ENT_REQUEST, ENT_COMMAND }; // ACTIONS
  enum { ON_COMMAND, CONN_MAX }; // CONNECTORS
  atm_connector connectors[CONN_MAX];
  atm_connector cmd_connectors[MAX_HANDLERS];
  String cmd_strings[MAX_HANDLERS];
  const char* reply_string;
  const char* commands;
  ESP8266WebServer * server;
  char incoming_request[80];
  uint8_t no_output;
  int lookup( char * cmd, const char* cmdlist ); 
  int event( int id ); 
  void action( int id ); 

};

/*
Automaton::ATML::begin - Automaton Markup Language

<?xml version="1.0" encoding="UTF-8"?>
<machines>
  <machine name="Atm_esp8266_httpd_simple">
    <states>
      <IDLE index="0" on_enter="ENT_IDLE" on_loop="LP_IDLE">
        <EVT_START>ACTIVE</EVT_START>
      </IDLE>
      <ACTIVE index="1" on_enter="ENT_ACTIVE">
        <EVT_INCOMING>REQUEST</EVT_INCOMING>
      </ACTIVE>
      <REQUEST index="2" on_enter="ENT_REQUEST">
        <EVT_CMD>COMMAND</EVT_CMD>
        <ELSE>IDLE</ELSE>
      </REQUEST>
      <COMMAND index="3" on_enter="ENT_COMMAND">
        <ELSE>IDLE</ELSE>
      </COMMAND>
    </states>
    <events>
      <EVT_INCOMING index="0" access="PRIVATE"/>
      <EVT_CMD index="1" access="MIXED"/>
      <EVT_START index="2" access="MIXED"/>
    </events>
    <connectors>
      <COMMAND autostore="0" broadcast="0" dir="PUSH" slots="1"/>
    </connectors>
    <methods>
    </methods>
  </machine>
</machines>

Automaton::ATML::end
*/

