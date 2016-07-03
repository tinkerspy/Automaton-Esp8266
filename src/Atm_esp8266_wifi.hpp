#pragma once

#include <Automaton.h>

class Atm_esp8266_wifi: public Machine {

 public:
  enum { IDLE, START, WAIT, CHECK, ACTIVE }; // STATES
  enum { EVT_START, EVT_STOP, EVT_TOGGLE, EVT_TIMER, EVT_CONNECT, EVT_DISCONNECT, ELSE }; // EVENTS
  Atm_esp8266_wifi( void ) : Machine() {};
  Atm_esp8266_wifi& begin(  String ssid, String password );
  Atm_esp8266_wifi& trace( Stream & stream );
  Atm_esp8266_wifi& trigger( int event );
  int state( void );
  Atm_esp8266_wifi& onChange( Machine& machine, int event = 0 );
  Atm_esp8266_wifi& onChange( atm_cb_push_t callback, int idx = 0 );
  Atm_esp8266_wifi& onChange( int sub, Machine& machine, int event = 0 );
  Atm_esp8266_wifi& onChange( int sub, atm_cb_push_t callback, int idx = 0 );
  Atm_esp8266_wifi& start( void );
  Atm_esp8266_wifi& stop( void );
  Atm_esp8266_wifi& toggle( void );

 private:
  enum { ENT_START }; // ACTIONS
  enum { ON_CHANGE, CONN_MAX = 2 }; // CONNECTORS
  atm_connector connectors[CONN_MAX];
  int event( int id ); 
  void action( int id ); 

};

/*
Automaton::ATML::begin - Automaton Markup Language

<?xml version="1.0" encoding="UTF-8"?>
<machines>
  <machine name="Atm_esp8266_wifi">
    <states>
      <IDLE index="0">
        <EVT_START>START</EVT_START>
        <EVT_TOGGLE>START</EVT_TOGGLE>
      </IDLE>
      <START index="1" on_enter="ENT_START">
        <ELSE>WAIT</ELSE>
      </START>
      <WAIT index="2">
        <EVT_TIMER>CHECK</EVT_TIMER>
      </WAIT>
      <CHECK index="3">
        <EVT_CONNECT>ACTIVE</EVT_CONNECT>
        <ELSE>WAIT</ELSE>
      </CHECK>
      <ACTIVE index="4">
        <EVT_DISCONNECT>WAIT</EVT_DISCONNECT>
      </ACTIVE>
    </states>
    <events>
      <EVT_START index="0" access="PUBLIC"/>
      <EVT_STOP index="1" access="PUBLIC"/>
      <EVT_TOGGLE index="2" access="PUBLIC"/>
      <EVT_TIMER index="3" access="PRIVATE"/>
      <EVT_CONNECT index="4" access="PRIVATE"/>
      <EVT_DISCONNECT index="5" access="PRIVATE"/>
    </events>
    <connectors>
      <CHANGE autostore="0" broadcast="0" dir="PUSH" slots="2"/>
    </connectors>
    <methods>
    </methods>
  </machine>
</machines>

Automaton::ATML::end
*/

