#include <Automaton.h>
#include <Atm_esp8266.h>

// Control a led on pin D5 with this simple webserver

Atm_esp8266_httpd_simple server( 80 );
Atm_led led;

char ap_ssid[] = "MySSID";
char ap_password[] = "MyPASSWORD";

void setup() {
  Serial.begin( 9600 );
  Serial.println( "Connecting to WIFI" );

  // The led to be controlled

  led.begin( D5 );
 
  // The Wifi machine manages the wifi connection
  
  wifi.begin( ap_ssid, ap_password ) 
    .onChange( true, [] ( int idx, int v, int up  ) {
      Serial.print( "Connected to Wifi, browse to http://");
      Serial.println( wifi.ip() );
      server.start();
    })
    .onChange( false, [] ( int idx, int v, int up  ) {
      Serial.println( "Lost Wifi connection");      
    })
    .led( LED_BUILTIN, true ) // Esp8266 built in led shows wifi status
    .start();

  // The Http server machine handles incoming requests

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
