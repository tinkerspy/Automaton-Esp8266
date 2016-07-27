#pragma once

#include <Automaton.h>

#define ATM_WIFI_MAX_NODES 16
#define ATM_WIFI_MIRROR_PORT 55056

union mirror_packet_t {
  char b[6];
  struct {
    uint32_t address;
    int16_t value;
  };
};

class Atm_esp8266_wifi: public Machine {

 public:
  enum { IDLE, START, WAIT, CHECK, ACTIVE, DISCONN }; // STATES
  enum { EVT_START, EVT_STOP, EVT_TOGGLE, EVT_TIMER, EVT_CONNECT, EVT_DISCONNECT, ELSE }; // EVENTS
  Atm_esp8266_wifi( void ) : Machine() {};
  Atm_esp8266_wifi& begin( const char ssid[], const char password[] );
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
  IPAddress ip( void );
  IPAddress netmask( void );
  IPAddress broadcastAddress( void );
  Atm_esp8266_wifi& led( int led, bool activeLow = false );
  uint16_t reg( const char name[] );
  uint16_t reg( uint32_t );
  Atm_esp8266_wifi & transmit( uint16_t mirrorId, int v );

 private:
  enum { ENT_START, ENT_ACTIVE, ENT_DISCONN }; // ACTIONS
  enum { ON_CHANGE, CONN_MAX = 2 }; // CONNECTORS
  atm_connector connectors[CONN_MAX];
  atm_connector mirror_connectors[ATM_WIFI_MAX_NODES];
  uint32_t mirror_nodes[ATM_WIFI_MAX_NODES];
  atm_timer_millis timer;
  int8_t indicator;
  bool indicatorActiveLow;
  WiFiUDP udp;
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
      <ACTIVE index="4" on_enter="ENT_ACTIVE">
        <EVT_DISCONNECT>DISCONN</EVT_DISCONNECT>
      </ACTIVE>
      <DISCONN index="5" on_enter="ENT_DISCONN">
        <ELSE>WAIT</ELSE>
      </DISCONN>
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

