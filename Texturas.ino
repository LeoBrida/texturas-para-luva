#include <Wire.h>
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
  String name;
  float thigh; //milisegundos
  float tlow; //milisegundos
  float frequency; //%
};

int pwmPin = D8; 
float temp;

String message;

Texture harsh_roughness_texture;
Texture fine_roughness_texture;
Texture smooth_texture;
Texture drip_texture;
Texture soft_texture;


void setup() {

  Serial.begin(115200);
  pinMode(pwmPin, OUTPUT);
  //analogWriteRange(255);
  harsh_roughness_texture = {"Harsh", 45, 94, 7.2}; //Rugosidade Grossa
  fine_roughness_texture = {"Fine", 22, 42, 15.6}; //Rugosidade Fina
  smooth_texture = {"Smooth", 2, 1, 333}; //Lisura
  drip_texture = {"Drip",100, 500, 1.7}; //Gotejamento
  soft_texture = {"Soft",1, 5, 166.7}; //Suavidade

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

  /*else if (message.indexOf("stop") != -1) {
    stopSimulation();
  }*/

}





