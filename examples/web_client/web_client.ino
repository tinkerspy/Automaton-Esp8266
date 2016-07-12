#include <Automaton.h>
#include <Atm_esp8266.h>

Atm_esp8266_httpc_simple client1, client2;
Atm_esp8266_wifi wifi;
Atm_button button1, button2;

// Two buttons on D2 & D7 send out /on and /off web requests
// Could be used as a remote control for the esp_blink example

void setup() {

  wifi.begin( "MySSID", "MyPASSWORD" ) 
    .led( LED_BUILTIN, true ) // Esp8266 built in led shows wifi status
    .start();  

  client1.begin( "172.22.22.220" )
    .get( "/on" );        
  client2.begin( "172.22.22.220" )
    .get( "/off" ); // 23 positions in a 1 night stand
        
  button1.begin( D2 )
    .onPress( client1, client1.EVT_START );
  button2.begin( D7 )
    .onPress( client2, client2.EVT_START );

}

void loop() {
  automaton.run();
}

