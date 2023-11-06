#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Definir el nombre y la contraseña de la red WiFi
const char* ssid = "FAMILIA FLORES";
const char* password = "*56ZP78#";

// Variables para el control de tiempo del request del mensaje
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// Definir el tipo de hardware y el numero de dispositivos
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 3

// Definir los pines de conexion
#define CLK_PIN   D5 // or SCK
#define DATA_PIN  D7 // or MOSI
#define CS_PIN    SS // or SS

// SPI hardware interface
MD_MAX72XX cartel = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// mensaje a mostrar
String mensaje = "Cargando...";
String proximo_mensaje;

void setup() {
  // inicializar el objeto mx
  cartel.begin();

  // Establecer intencidad a un valor de 5
  cartel.control(MD_MAX72XX::INTENSITY, 10);

  // Desactivar auto-actualizacion
  cartel.control( MD_MAX72XX::UPDATE, false );

  WiFi.begin(ssid, password);

  Serial.print("Conectando a la red WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado exitosamente!");
  // inicializar puerto Serie a 9600 baudios
  Serial.begin(9600);

}
void loop() {
  mensaje = getMessage();
  slide_text( 150 );
  actualizar_mensaje();
}

String getMessage(){
  String newmessage = mensaje;
  if((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      // Crear cliente HTTPS
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      // Permite que el cliente acepte certificados SSL invalidos
      client->setInsecure();
      HTTPClient http;
      //Se crea un objeto WiFiClient
      http.begin(*client,"https://arc.h130.dev/api/message/text"); // Se utiliza el objeto WiFiClient como primer argumento
      int httpCode = http.GET();
      Serial.print("Codigo HTTP: ");
      Serial.println(httpCode);

      if (httpCode > 0) {
        // Obtener mensaje de respuesta
        newmessage = http.getString();
        Serial.print(newmessage);
      }
      http.end();
    }
  }
  return newmessage;
}

void actualizar_mensaje(){

  while( Serial.available() ){

    char c = Serial.read();

    if( c == '\n' ){
      mensaje = proximo_mensaje;
      proximo_mensaje = "";
      break;
    }
    else
      proximo_mensaje += c;
    
  }
}

void slide_text(int ms_delay){
  int col = 0;
  int last_pos;
  bool completo = false;
  
  cartel.clear();

  while( completo == false ){
    last_pos = printText(col, mensaje);
    delay(ms_delay);
    col++;
    if( last_pos > (int)cartel.getColumnCount() )
      completo = true;
  }
}

int printText(int pos, const String text){
  int w;
  
  for( int i = 0; i < text.length(); i++ ){
    // imprimir letra

    w = cartel.setChar( pos, text[i] );
    // la proxima letra empieza donde termina esta
    pos = pos - w; 
    // Se deja una columna entre letras.
    cartel.setColumn(pos, B00000000);
    
    pos = pos - 1;
    
    if( pos < 0 )
      break;
      
  }
  cartel.update();
  
  return pos;
}