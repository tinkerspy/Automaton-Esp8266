#pragma once

#include <Automaton.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

class Atm_esp8266_server: public Machine {

 public:
  enum { IDLE, REQUEST, COMMAND }; // STATES
  enum { EVT_INCOMING, EVT_CMD, ELSE }; // EVENTS
  Atm_esp8266_server( int port ) : Machine() {  static ESP8266WebServer esp_server( port ); server = &esp_server; };
  Atm_esp8266_server& begin( void );
  Atm_esp8266_server& trace( Stream & stream );
  Atm_esp8266_server& trigger( int event );
  int state( void );
  Atm_esp8266_server& onCommand( Machine& machine, int event = 0 );
  Atm_esp8266_server& onCommand( atm_cb_push_t callback, int idx = 0 );
  Atm_esp8266_server& cmd( void );
  Atm_esp8266_server& list( const char* cmds );
  Atm_esp8266_server& send( int result, String content_type, String content );
  String uri( void );
  int args( void );
  String argName( int id );
  String arg( int id );
  String arg( String name );

  int lookup( char * cmd, const char* cmdlist ); // Move to private!!!

 private:
  enum { ENT_IDLE, LP_IDLE, ENT_REQUEST, ENT_COMMAND }; // ACTIONS
  enum { ON_COMMAND, CONN_MAX }; // CONNECTORS
  atm_connector connectors[CONN_MAX];
  const char* commands;
  ESP8266WebServer * server;
  char incoming_request[80];
  int event( int id ); 
  void action( int id ); 

};

/*
Automaton::ATML::begin - Automaton Markup Language

<?xml version="1.0" encoding="UTF-8"?>
<machines>
  <machine name="Atm_esp8266_server">
    <states>
      <IDLE index="0" on_enter="ENT_IDLE" on_loop="LP_IDLE">
        <EVT_INCOMING>REQUEST</EVT_INCOMING>
      </IDLE>
      <REQUEST index="1" on_enter="ENT_REQUEST">
        <EVT_CMD>COMMAND</EVT_CMD>
        <ELSE>IDLE</ELSE>
      </REQUEST>
      <COMMAND index="2" on_enter="ENT_COMMAND">
        <ELSE>IDLE</ELSE>
      </COMMAND>
    </states>
    <events>
      <EVT_INCOMING index="0" access="PRIVATE"/>
      <EVT_CMD index="1" access="MIXED"/>
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

