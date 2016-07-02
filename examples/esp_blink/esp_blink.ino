#include <Automaton.h>
#include <Atm_esp8266.h>

Atm_esp8266_server server( 80 );
Atm_led led;

const char cmdlist[] = "/ /on /off";
enum { CMD_ROOT, CMD_ON, CMD_OFF };

void setup() {

  Serial.begin( 115200 );
  
  Serial.println( "Connecting to Wifi" );
  WiFi.begin( "MySSID", "MyPassword" );
  while ( WiFi.status() != WL_CONNECTED ) {
    delay( 500 ); 
    Serial.println( "Wait..." );
  }
  Serial.print( "Connected to Wifi, address: ");
  Serial.println( WiFi.localIP() );

  led.begin( D4 );
  
  server.begin()
    .list( cmdlist )
    .onCommand( [] ( int idx, int v, int up ) {
      switch ( v ) {
        case CMD_ROOT:        
          return;
        case CMD_ON:        
          led.start();
          server.send( 200, "text/plain", "LED blinking " );
          return;
        case CMD_OFF:
          led.off();
          server.send( 200, "text/plain", "LED off");
          return;
      }      
    });    
}

void loop() {
  automaton.run();
}

