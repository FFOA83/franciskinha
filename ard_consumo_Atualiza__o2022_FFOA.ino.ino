//#include <DS1307.h>                   //Biblioteca do relógio clck
#include <DS3231.h>                 //Biblioteca do relógio clck
#include <Wire.h>                 // Biblioteca para comunicação I2C/TWI SPA: Linha de daods e SCL: Linha de clock
#include <SD.h>                //Biblioteca do Cartão de memória SD
#include <Adafruit_ADS1015.h> //Biblioteca para incluir o módulo conversor de sinal analógico para digital
#include "RTClib.h"         
#include <EEPROM.h>         // Biblioteca para gravação dos dados em memória
#include "EmonLib.h"   //Biblioteca para TP 
//Teste
EnergyMonitor emon1; 
Adafruit_ADS1115 ads(0x48);
const float SetPoint = 50;      //50A/1V
const float multiplicador = 0.095f;
const int offset = 512;
int addr = 0;     //endereço na eeprom para armazenar dado.
RTC_DS3231 rtc;
File dataFile;
DateTime now;
const int chipSelect = 53;
int tempo_inicial = millis();
float consumo;
char daysOfTheWeek[7][12]= {"domingo","segunda","terça","quarta","quinta","sexta","sábado"}; 

///////////SETUP//////////

void setup() {
  pinMode(4, OUTPUT);
  emon1.voltage(1, 234.26, 1.7);
  ads.setGain(GAIN_TWO);        // ±2.048V  1 bit = 0.0848mV
  ads.begin(); 
  //rtc.halt(false);
  rtc.begin();
  //rtc.setDOW(FRIDAY);    
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  else{
   Serial.println("DataLogger OK!");
   delay(2000);
  }
  Serial.begin(9600);
  SD.begin();
  Wire.begin();
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
   }
   else{
    Serial.println("Cartao SD OK!");
    delay(3000);
   }
 Serial.println("card initialized.");
  DateTime now = rtc.now();
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.print("Start logging on: ");
  dataFile.print(now.year(),DEC);
  dataFile.print('/');
  dataFile.print(now.month(),DEC);
  dataFile.print('/');
  dataFile.print(now.day(),DEC);
  dataFile.println(" ");
  dataFile.println("Data            Hora            Tensão        Corrente      Consumo(kwh)");
  dataFile.close();
  Serial.print(tempo_inicial);
  Serial.print("Start logging on: ");
  Serial.print(now.year(),DEC);
  Serial.print('/');
  Serial.print(now.month(),DEC);
  Serial.print('/');
  Serial.print(now.day(),DEC);
  Serial.println(" ");
  Serial.println("Data            Hora            Tensão        Corrente      Consumo(kwh)");
}

//LOOP//////////LOOP////////

void loop(){ 
  int tempoI = millis();
  float tempo_final = millis() - tempo_inicial; 
  float  Volta = ads.readADC_Differential_0_1();
  float CorrenteRMS = getCorrente();
  float TensaoRMS = getTensao();
  float PotenciaAtiva = 220.00 * CorrenteRMS;
  //float i = getCorrente();
  //float ultimo_consumo = EEPROM.get(addr, f);
  consumo = ((CorrenteRMS/1000) * (tempo_final/3600000)) + consumo;
 // byte hiByte = highByte(consumo);
 // byte loByte = lowByte(consumo);
  //EEPROM.put(addr, consumo);
 // EEPROM.write(addr + 1, loByte);
  DateTime now = rtc.now();
 //long int transTime = tempoI;
  //(now.unixtime())
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile){
     dataFile.print(now.day(),DEC);
     dataFile.print('/');
     dataFile.print(now.month(),DEC);
     dataFile.print('/');
     dataFile.print(now.year(),DEC);
     dataFile.print("        ");
     dataFile.print(now.hour(),DEC);
     dataFile.print(":");
     dataFile.print(now.minute(),DEC);
     dataFile.print(":");
     dataFile.print(now.second(),DEC);
     dataFile.print("        ");
     dataFile.print(TensaoRMS);
     dataFile.print("        ");
     dataFile.print(CorrenteRMS);
     dataFile.print("         ");
     dataFile.println(consumo, 4);   
     dataFile.close();
     Serial.print(now.day(),DEC);
     Serial.print('/');
     Serial.print(now.month(),DEC);
     Serial.print('/');
     Serial.print(now.year(),DEC);
     Serial.print("        ");
     Serial.print(now.hour(),DEC);
     Serial.print(":");
     Serial.print(now.minute(),DEC);
     Serial.print(":");
     Serial.print(now.second(),DEC);
     Serial.print("        ");
     Serial.print(TensaoRMS);
     Serial.print("        ");
     Serial.print(CorrenteRMS); 
     Serial.print("           ");
     Serial.print(consumo, 2);  
     Serial.print("            ");
     Serial.print(now.unixtime());
     Serial.print("           ");
     Serial.println(tempo_final/1000);      
  }
  

  digitalWrite(4, HIGH);
  
  Serial.print(TensaoRMS, 2);
  Serial.print("    ");
  Serial.println(CorrenteRMS, 2);
}
float getCorrente(){
 float Volt_diferencial;
 float corrente;
 float soma1 = 0;
 long tempoI = millis();
 int counterI = 0;
 while (millis() - tempoI < 100)
   {
     Volt_diferencial = ads.readADC_Differential_0_1() * multiplicador;
     corrente = Volt_diferencial * SetPoint;
     corrente /= 1000.0;
     soma1 += sq(corrente);
     counterI = counterI + 1;
  }  
 corrente = sqrt(soma1 / counterI);
 return(corrente);
}

float getTensao(){
  emon1.calcVI(4, 100);  
  float tensao = emon1.Vrms;
  return (tensao);
  delay(3000);
}
