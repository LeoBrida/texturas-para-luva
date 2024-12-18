#include <Wire.h>
#include "SSD1306Wire.h"

 
struct Texture {
  String name;
  float thigh; //milisegundos
  float tlow; //milisegundos
  float frequency; //%
};

int pwmPin = D8; 
float temp;

const int sda_pin = D5; // definição do pino I2C SDA
const int scl_pin = D6; // definição do pino I2C SCL
SSD1306Wire  display(0x3c, sda_pin, scl_pin); // Inicializa o display Oled

String message;

Texture harsh_roughness_texture;
Texture fine_roughness_texture;
Texture smooth_texture;
Texture drip_texture;
Texture soft_texture;

void initI2C() {
  Wire.begin(sda_pin, scl_pin);
}

void telainicial(){

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);

  display.setFont(ArialMT_Plain_10);
  display.drawString(63, 1, "ESP8266");
  display.drawString(63, 26, "Iniciando módulos...");
  display.drawString(63, 45, "Display Oled - OK");
  display.display();
}

void setup() {

  Serial.begin(115200);
  pinMode(pwmPin, OUTPUT);
  initI2C();
  display.init();

  harsh_roughness_texture = {"Harsh", 45, 94, 7.2}; //Rugosidade Grossa
  fine_roughness_texture = {"Fine", 22, 42, 15.6}; //Rugosidade Fina
  smooth_texture = {"Smooth", 2, 1, 333}; //Lisura
  drip_texture = {"Drip",100, 500, 1.7}; //Gotejamento
  soft_texture = {"Soft",1, 5, 166.7}; //Suavidade
  
  telainicial();
  delay(1000);

}

void loop() {
  if (Serial.available()) {  // Verifica se há dados disponíveis na serial
    message = Serial.readString();  // Lê a string enviada pelo monitor serial
    Serial.print("Comando: ");
    Serial.println(message);  // Exibe a string no monitor serial
    controller(message);
  }
}

float time(Texture texture){
  int seconds = 1;
  temp = (texture.frequency)*seconds;
  return temp;
}

void startSimulation(Texture texture) {

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(63, 18, "Reprodução de texturas");
  display.display();

  time(texture);
  Serial.print("Tempo de inicio:");
  Serial.println(millis()/1000);

  while (temp > 0){ 
  analogWrite(pwmPin, 255);
  delay(texture.thigh);
  analogWrite(pwmPin, 0);
  delay(texture.tlow);
  temp--; 
  }
  
  Serial.print("Tempo de fim:");
  Serial.println(millis()/1000);
  Serial.println("Reprodução de "+ texture.name + " finalizada");
}

void stopSimulation() {
  analogWrite(pwmPin, 0);
}

void controller(String message){

  if (message.indexOf("harsh") != -1) {
    startSimulation(harsh_roughness_texture);
  }

  else if (message.indexOf("fine") != -1) {
    startSimulation(fine_roughness_texture);
  }

  else if (message.indexOf("smooth") != -1) {
    startSimulation(smooth_texture); 
  }

  else if (message.indexOf("drip") != -1) {
    startSimulation(drip_texture);
  }

  else if (message.indexOf("soft") != -1) {
    startSimulation(soft_texture);
  }

}





