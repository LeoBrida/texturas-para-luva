#include <Wire.h>
#include <ESP8266WiFi.h>
#include "SSD1306Wire.h"
//#include <MPU6050.h>
//#include <helper_3dmath.h>

/*
Links úteis:
https://randomnerdtutorials.com/esp8266-pwm-arduino-ide/
https://arduino-esp8266.readthedocs.io/en/latest/reference.html
https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
https://docs.arduino.cc/learn/microcontrollers/analog-output/
https://www.eevblog.com/forum/microcontrollers/esp8266-analogwritefreq(32000)-gives-30-1-khz-instead-of-32khz-why/
https://electronoobs.com/eng_arduino_tut140.php
https://github.com/khoih-prog/ESP8266TimerInterrupt
https://lurchi.wordpress.com/2016/06/29/esp8266-pwm-revisited-and-reimplemented/
*/
 
struct Texture {
  float thigh; //milisegundos
  float tlow; //milisegundos
  float duty_cycle; //%
};

int pwmPin = D8; 
unsigned long start_time = 0;

const char* ssid = "DIONE";
const char* password = "flora123";
WiFiServer server(80);

String message;

Texture harsh_roughness_texture;
Texture fine_roughness_texture;
Texture smooth_texture;
Texture drip_texture;
Texture soft_texture;


/*
  Avaliar a versão do ESP8266 para saber se o range vai de 0 a 255 ou 0 a 1023

  Para 255 representando um duty cicle de 100%
  harsh_roughness_texture possui 32% de duty cicle, ou seja, 82
  fine_roughness_texture possui 34% de duty cicle, ou seja, 87
  smooth_texture possui 67% de duty cicle, ou seja, 171
  drip_texture possui 17% de duty cicle, ou seja, 43 (43,3)
  soft_texture possui 16,7% de duty cicle, ou seja, 43 (42,5)

*/


void setup() {

  Serial.begin(9600);
  pinMode(pwmPin, OUTPUT);
  //analogWriteRange(255);
  harsh_roughness_texture = { 45, 94, 82}; //Rugosidade Grossa
  fine_roughness_texture = { 22, 42, 87}; //Rugosidade Fina
  smooth_texture = { 2, 1, 171}; //Lisura
  drip_texture = {100, 500, 43}; //Gotejamento
  soft_texture = {1, 5, 42}; //Suavidade

  //Configurando o módulo Wifi
  Serial.println("Conectando a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a rede: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  /*if (Serial.available()) {  // Verifica se há dados disponíveis na serial
    message = Serial.readString();  // Lê a string enviada pelo monitor serial
    Serial.print("Comando: ");
    Serial.println(message);  // Exibe a string no monitor serial
    controller(message);
  }*/

  WiFiClient client;
  client = server.available();

  if (client){
    String request = client.readStringUntil('\r');
    request.trim();
    Serial.println(request);
    wifiController(request);
  }
}


void startSimulation(Texture texture) {

  start_time = millis();
  Serial.println(start_time);

  /*while (true){
  message = Serial.readString();  // Lê a string enviada pelo monitor serial
  if (message.indexOf("stop") != -1){
    Serial.print("Comando: ");
    Serial.println(message);
    stopSimulation();
    break;
  }*/ 

  analogWrite(pwmPin, 255);
  delay(texture.thigh);
  analogWrite(pwmPin, 0);
  delay(texture.tlow);

  /*digitalWrite(pwmPin, HIGH);
  delay(texture.thigh);
  digitalWrite(pwmPin, LOW);
  delay(texture.tlow);*/
}

void stopSimulation() {
  analogWrite(pwmPin, 0);
}

void wifiController(String request){
  if (request.indexOf("harsh_roughness_texture")!= -1){
      startSimulation(harsh_roughness_texture);
      Serial.println("Reproduzindo Rugosidade Grossa");
    }
    else if (request.indexOf("fine_roughness_texture") != -1){
      startSimulation(fine_roughness_texture);
      Serial.println("Reproduzindo Rugosidade Fina");
    }
    else if (request.indexOf("smooth_texture") != -1){
      startSimulation(smooth_texture);
      Serial.println("Reproduzindo Lisura");
    }
    else if (request.indexOf("drip_texture") != -1){
      startSimulation(drip_texture);
      Serial.println("Reproduzindo Gotejamento");
    }
    else if (request.indexOf("soft_texture") != -1){
      startSimulation(soft_texture);
      Serial.println("Reproduzindo Suavidade");
    }
    else if (request.indexOf("stop") != -1){
      stopSimulation();
      Serial.println("Reprodução finalizada");
    }
}

