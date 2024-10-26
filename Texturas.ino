#include <ESP8266WiFi.h> // biblioteca para usar as funções de Wifi do módulo ESP8266
#include <Wire.h>         // biblioteca de comunicação I2C
#include "SSD1306Wire.h"
 
/*
 * Definições de alguns endereços mais comuns do MPU6050
 * os registros podem ser facilmente encontrados no mapa de registros do MPU6050
 */
const int MPU_ADDR =      0x68; // definição do endereço do sensor MPU6050 (0x68)
const int WHO_AM_I =      0x75; // registro de identificação do dispositivo
const int PWR_MGMT_1 =    0x6B; // registro de configuração do gerenciamento de energia
const int GYRO_CONFIG =   0x1B; // registro de configuração do giroscópio
const int ACCEL_CONFIG =  0x1C; // registro de configuração do acelerômetro
const int ACCEL_XOUT =    0x3B; // registro de leitura do eixo X do acelerômetro
 
const int sda_pin = D5; // definição do pino I2C SDA
const int scl_pin = D6; // definição do pino I2C SCL

const int motor_pin = D8; //entrada de controle dos atuadores

// Armazenamento dos dados "crus" do acelerômetro
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

struct Texture {
  float thigh; 
  float tlow; 
  float duty_cycle; 
};

unsigned long start_time = 0;
String simulating;

String message;

Texture harsh_roughness_texture;
Texture fine_roughness_texture;
Texture smooth_texture;
Texture drip_texture;
Texture soft_texture;
Texture simulatedTexture;

/*const char* ssid="BRIDA_5G";
const char* password="";
WiFiServer server(80);*/ 

/*
  Avaliar a versão do ESP8266 para saber se o range vai de 0 a 255 ou 0 a 1023

  Para 255 representando um duty cicle de 100%
  harsh_roughness_texture possui 32% de duty cicle, ou seja, 82
  fine_roughness_texture possui 34% de duty cicle, ou seja, 87
  smooth_texture possui 67% de duty cicle, ou seja, 171
  drip_texture possui 17% de duty cicle, ou seja, 43 (43,3)
  soft_texture possui 16,7% de duty cicle, ou seja, 43 (42,5)

*/

// Inicializa o display Oled
SSD1306Wire  display(0x3c, sda_pin, scl_pin);
 
// Configura a I2C com os pinos desejados.
void initI2C() {
  Wire.begin(sda_pin, scl_pin);
}

// Escreve um dado valor em um dado registro
void writeRegMPU(int reg, int val){      
  Wire.beginTransmission(MPU_ADDR);     // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                      // escreve o valor no registro
  Wire.endTransmission(true);           // termina a transmissão
}

// Leitura do registro indicado
uint8_t readRegMPU(uint8_t reg){
  uint8_t data;
  Wire.beginTransmission(MPU_ADDR);     // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
  Wire.endTransmission(false);          // termina transmissão mas continua com I2C aberto (envia STOP e START)
  Wire.requestFrom(MPU_ADDR, 1);        // configura para receber 1 byte do registro escolhido acima
  data = Wire.read();                   // lê o byte e guarda em 'data'
  return data;                          //retorna 'data'
}

// Procura pelo sensor no endereço indicado (0x68)
void findMPU(int mpu_addr){
  Wire.beginTransmission(MPU_ADDR);
  int data = Wire.endTransmission(true);
 
  if(data == 0){
    Serial.print("Dispositivo encontrado no endereço: 0x");
    Serial.println(MPU_ADDR, HEX);
  }
  else{
    Serial.println("Dispositivo não encontrado!");
  }
}

// Verifica se o sensor responde e se está ativo
void checkMPU(int mpu_addr){
  findMPU(MPU_ADDR);
  int data = readRegMPU(WHO_AM_I); // Register 117 – Who Am I - 0x75
  if(data == 104) {
    Serial.println("MPU6050 Dispositivo respondeu OK! (104)");
    data = readRegMPU(PWR_MGMT_1); // Register 107 – Power Management 1-0x6B

    if(data == 64) Serial.println("MPU6050 em modo SLEEP! (64)");
    else Serial.println("MPU6050 em modo ACTIVE!"); 
  }
  else Serial.println("Verifique dispositivo - MPU6050 NÃO disponível!");
}

// Inicializa o sensor
void initMPU(){
  setSleepOff();
  setGyroScale();
  setAccelScale();
}

// Configura o sleep bit  
void setSleepOff(){
  writeRegMPU(PWR_MGMT_1, 0); // escreve 0 no registro de gerenciamento de energia(0x68), colocando o sensor em o modo ACTIVE
}
 
/* função para configurar as escalas do giroscópio
   registro da escala do giroscópio: 0x1B[4:3]
   0 é 250°/s */

void setGyroScale(){
  writeRegMPU(GYRO_CONFIG, 0);
}
 
/* função para configurar as escalas do acelerômetro
   registro da escala do acelerômetro: 0x1C[4:3]
   0 é 250°/s
*/
void setAccelScale(){
  writeRegMPU(ACCEL_CONFIG, 0);
}

/* Leitura dos dados 'crus'(raw data) do sensor.
   São 14 bytes no total sendo eles 2 bytes para cada eixo e
   2 bytes para temperatura:
 
  0x3B 59 ACCEL_XOUT[15:8]
  0x3C 60 ACCEL_XOUT[7:0]
  0x3D 61 ACCEL_YOUT[15:8]
  0x3E 62 ACCEL_YOUT[7:0]
  0x3F 63 ACCEL_ZOUT[15:8]
  0x40 64 ACCEL_ZOUT[7:0]
 
  0x41 65 TEMP_OUT[15:8]
  0x42 66 TEMP_OUT[7:0]
 
  0x43 67 GYRO_XOUT[15:8]
  0x44 68 GYRO_XOUT[7:0]
  0x45 69 GYRO_YOUT[15:8]
  0x46 70 GYRO_YOUT[7:0]
  0x47 71 GYRO_ZOUT[15:8]
  0x48 72 GYRO_ZOUT[7:0]
    
*/
void readRawMPU(Texture simTex)
{  
  Wire.beginTransmission(MPU_ADDR);       // inicia comunicação com endereço do MPU6050
  Wire.write(ACCEL_XOUT);                 // envia o registro com o qual se deseja trabalhar, começando com registro 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);            // termina transmissão mas continua com I2C aberto (envia STOP e START)
  Wire.requestFrom(MPU_ADDR, 14);         // configura para receber 14 bytes começando do registro escolhido acima (0x3B)
 
  AcX = Wire.read() << 8;                 // lê primeiro o byte mais significativo
  AcX |= Wire.read();                     // depois lê o bit menos significativo
  AcY = Wire.read() << 8;
  AcY |= Wire.read();
  AcZ = Wire.read() << 8;
  AcZ |= Wire.read();
 
  Tmp = Wire.read() << 8;
  Tmp |= Wire.read();
 
  GyX = Wire.read() << 8;
  GyX |= Wire.read();
  GyY = Wire.read() << 8;
  GyY |= Wire.read();
  GyZ = Wire.read() << 8;
  GyZ |= Wire.read(); 
 
  // Verificar deslocamento horizontal no eixo Z
  float displacementZ = GyZ / 131.0;

  //start_time = (millis()/1000);
  //Serial.println(start_time);

  // simulação de texturas, tirar daqui e colocar no loop?
  if (displacementZ > 20 || displacementZ < -20) {
    analogWrite(motor_pin, 255);
    delay(simTex.thigh);
    analogWrite(motor_pin, 0);
    delay(simTex.tlow);
  } 
  else {
    analogWrite(motor_pin, 0);
  }

  Serial.print("AcX = "); Serial.print(AcX / 16384.0);
  Serial.print(" | AcY = "); Serial.print(AcY / 16384.0);
  Serial.print(" | AcZ = "); Serial.print(AcZ / 16384.0);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);
  Serial.print(" | GyX = "); Serial.print(GyX / 131.0);
  Serial.print(" | GyY = "); Serial.print(GyY / 131.0);
  Serial.print(" | GyZ = "); Serial.println(GyZ / 131.0);

  //Apaga o display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);

  display.drawString(63, 1, "NodeMCU do Sergio");
  display.drawString(63, 18, "AcX: " + String(AcX / 16384.0) + "  GyX: " + String(GyX / 131.0));
  display.drawString(63, 28, "AcY: " + String(AcY / 16384.0) + "  GyY: " + String(GyY / 131.0));
  display.drawString(63, 38, "AcZ: " + String(AcZ / 16384.0) + "  GyZ: " + String(GyZ / 131.0));
  display.drawString(63, 48, "Temperatura: " + String((Tmp + 12421) / 340.0) + " °C");
  display.display();
}
 
// Configura e seta o display
void telainicial(){

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);

  display.setFont(ArialMT_Plain_10);
  display.drawString(63, 1, "ESP8266 do Sergio");
  display.drawString(63, 26, "Iniciando módulos...");
  display.drawString(63, 45, "Display Oled - OK");
  display.display();
}

void setup() {
  Serial.begin(115200); // verificar
  display.init();
  display.flipScreenVertically();
  pinMode(motor_pin, OUTPUT);
  analogWriteRange(255);

  Serial.println("nIniciando configuração do MPU6050n");
  initI2C();
  initMPU();
  checkMPU(MPU_ADDR);

  harsh_roughness_texture = { 45, 94, 82}; //Rugosidade Grossa
  fine_roughness_texture = { 22, 42, 87}; //Rugosidade Fina
  smooth_texture = { 2, 1, 171}; //Lisura
  drip_texture = {100, 500, 43}; //Gotejamento
  soft_texture = {1, 5, 42}; //Suavidade

//Configurando o módulo Wifi
  /*Serial.println("Conectando a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a rede!");
  Serial.println(WiFi.localIP());*/

  telainicial();
  delay(1000);

}

void loop() {
  if (Serial.available()) {  // Verifica se há dados disponíveis na serial
    message = Serial.readString();  
    Serial.print("Comando: ");
    Serial.println(message);  
    simulatedTexture = controller(message);
  }

  readRawMPU(simulatedTexture);
//Seleção por WiFi
  /*WiFiClient client;
  client = server.available();

  if (client){
    String request = client.readStringUntil('\r');
    Serial.println(request);

    requested = wifiController(request);
    readRawMPU(requested);

  }*/ 
}

void startSimulation(Texture texture) {

  start_time = (millis()/1000);
  Serial.println(start_time);

  while (true){
  analogWrite(motor_pin, 255);
  delay(texture.thigh); // add "random"
  analogWrite(motor_pin, 0);
  delay(texture.tlow);
  }
}

void stopSimulation() {
  analogWrite(motor_pin, 0);
}

/*void wifiController(){
  if (request.indexOf("harsh_roughness_texture" != -1)){
      startSimulation(harsh_roughness_texture);
      Serial.println("Reproduzindo Rugosidade Grossa");
    }
    else if (request.indexOf("fine_roughness_texture" != -1)){
      startSimulation(fine_roughness_texture);
      Serial.println("Reproduzindo Rugosidade Fina");
    }
    else if (request.indexOf("smooth_texture" != -1)){
      startSimulation(smooth_texture);
      Serial.println("Reproduzindo Lisura");
    }
    else if (request.indexOf("drip_texture" != -1)){
      startSimulation(drip_texture);
      Serial.println("Reproduzindo Gotejamento");
    }
    else if (request.indexOf("soft_texture" != -1)){
      startSimulation(soft_texture);
      Serial.println("Reproduzindo Suavidade");
    }
    else if (request.indexOf("stop" != -1)){
      stopSimulation();
      Serial.println("Reprodução finalizada");
    }
}*/

Texture controller(String message){

  if (message.indexOf("harsh") != -1) {
    simulatedTexture = harsh_roughness_texture;
  }

  else if (message.indexOf("fine") != -1) {
    //startSimulation(fine_roughness_texture);
    simulatedTexture = fine_roughness_texture;
  }

  else if (message.indexOf("smooth") != -1) {
    simulatedTexture = smooth_texture;
  }

  else if (message.indexOf("drip") != -1) {
    simulatedTexture = drip_texture;
  }

  else if (message.indexOf("soft") != -1) {
    simulatedTexture = soft_texture;
  }

  else if (message.indexOf("stop") != -1) {
    stopSimulation();
  }
  return simulatedTexture;
}





