#include <Automaton.h>
#include <Atm_esp8266.h>

Atm_esp8266_httpc_simple client;
Atm_esp8266_wifi wifi;
Atm_button button1, button2;

void setup() {

  Serial.begin( 115200 );
  Serial.println( "Start" );

  wifi.begin( "MySSID", "MyPASSWORD" ) 
    .onChange( true, [] ( int idx, int v, int up  ) {
      Serial.println( "Connected to Wifi");
    })
    .onChange( false, [] ( int idx, int v, int up  ) {
      Serial.println( "Lost Wifi connection");      
    })
    .led( LED_BUILTIN, true ) // Esp8266 built in led shows wifi status
    .start();  

  client.begin( "172.22.22.220" )
    .onFinish( [] ( int idx, int v, int up ) {
      Serial.print( "Result: " );
      Serial.println( client.responseCode() );
      Serial.print( "[" );
      Serial.print( client.headers() );
      Serial.println( "]" );
      Serial.print( "[" );
      Serial.print( client.content() );
      Serial.println( "]" );
    });
        
  button1.begin( D2 )
    .onPress( [] ( int idx, int v, int up ) {
      client
        .get( "/on" )
        .start();
    });
  button2.begin( D7 )
    .onPress( [] ( int idx, int v, int up ) {
      client
        .get( "/off" )
        .start();
    });

}

void loop() {
  automaton.run();
}

