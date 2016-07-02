#include <Automaton.h>
#include <Atm_esp8266.h>

Atm_esp8266_https_simple server( 80 );
Atm_led led;

void setup() {

  Serial.begin( 9600 );
  Serial.println( "Connecting to Wifi" );
  WiFi.begin( "MySSID", "MyWpaPassword" );
  while ( WiFi.status() != WL_CONNECTED ) {
    delay( 500 ); 
    Serial.println( "Wait..." );
  }
  Serial.print( "Connected to Wifi, address: ");
  Serial.println( WiFi.localIP() );

  led.begin( D4 );
  
  server.begin()
    .onRequest( "/on", led, led.EVT_ON )
    .onRequest( "/off", led, led.EVT_OFF )
    .onRequest( "/blink", led, led.EVT_START )
    .onRequest( "/", [] ( int idx, int v, int up ) {
      server.send( 
        "<!DOCTYPE html><html><body>"
        "<a href='on'>On</a><br>" 
        "<a href='off'>Off</a><br>" 
        "<a href='blink'>Blink</a><br>" 
        "</body></html>"
      );
    });
}

void loop() {
  automaton.run();
}