#include <Automaton.h>
#include <Atm_esp8266.h>

// Minimal Hello World webserver

Atm_esp8266_httpd_simple server( 80 );

void setup() {
  wifi.begin( "MySSID", "MyPASSWORD" )
    .onChange( true, server, server.EVT_START )
    .start();

  server.begin()
    .onRequest( "/", [] ( int idx, int v, int up ) {
      server.send( 
        "<!DOCTYPE html><html><body>"
        "Hello world!"
        "</body></html>"
      );
    });
}

void loop() {
  automaton.run();
}
