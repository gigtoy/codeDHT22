#define BLYNK_TEMPLATE_ID "TMPLTyQ7jLQK"
#define BLYNK_DEVICE_NAME "New start Device"
#define BLYNK_AUTH_TOKEN "JY5R2RySLLo8xPXv6aaNbpe5WM4tJrGm"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
#include <SimpleTimer.h>
#include "Adafruit_MCP23017.h"
#include <LiquidCrystal_I2C.h>

#define DHTPIN D4       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Gigtoy1";
char pass[] = "kenpeepo12";

SimpleTimer timer;
Adafruit_MCP23017 mcp;
LiquidCrystal_I2C lcd(0x27,20,4);
DHT dht(DHTPIN, DHTTYPE);

int Val = 0;
int i = 0;
int pressAuto;
float avtemp = 0;
float avhumi = 0;
float sumH[20];
float sumT[20];
float realT;
float limitHigh;
float realH,HumiHigh;

void processSensor(){
  for (i=0; i<=19; i++){
    delay(100);
    sumH[i] = dht.readHumidity();
    sumT[i] = dht.readTemperature();
    if (isnan(sumH[i]) || isnan(sumT[i])){
    sumH[i] = 0;
    sumT[i] = 0;
    }
    avtemp = avtemp + sumT[i]; 
    avhumi = avhumi + sumH[i];
  }
  realT = avtemp/20;
  realH = avhumi/20;
  
  Serial.print(F("Temperature: "));
  Serial.print(realT);
  Serial.print(F("°C "));
  Serial.println();
  Serial.print(F("Humidity: "));
  Serial.print(realH);
  Serial.print(F("%"));
  Serial.println();
  Serial.println("***********************");
  Serial.println();
  
//show Temp&HUmidity in Blynk app
  Blynk.virtualWrite(V2, realT);
  Blynk.virtualWrite(V3, realH);
  
//set average temp&humidity to zero
  avtemp = 0;
  avhumi = 0;
}

void condition(){// Compare Threshold value from Blynk and DHT Temperature value.
  if ((pressAuto == 1)||(pressAuto == 2)){
    if (realT < limitHigh && realH < HumiHigh){ //Temperature
      mcp.digitalWrite(6, HIGH);
      mcp.digitalWrite(7, LOW);
      mcp.digitalWrite(8, HIGH);
      mcp.digitalWrite(9, LOW);
      Serial.println(F("LowTemp & LowHumidity"));
      }
    else if (realT < limitHigh && realH > HumiHigh+5){
      mcp.digitalWrite(6, LOW);
      mcp.digitalWrite(7, LOW);
      mcp.digitalWrite(8, HIGH);
      mcp.digitalWrite(9, HIGH);
      Serial.println(F("LowTemp & HighHumidity"));
      }
    if (realT > limitHigh+0.5 && realH < HumiHigh){ //Humidity
      mcp.digitalWrite(6, LOW);
      mcp.digitalWrite(7, HIGH);
      mcp.digitalWrite(8, LOW);
      mcp.digitalWrite(9, LOW);
      Serial.println(F("HighTemp & LowHumidity"));
    }
    else if (realT > limitHigh+0.5 && realH > HumiHigh+5){
      mcp.digitalWrite(6, LOW);
      mcp.digitalWrite(7, HIGH);
      mcp.digitalWrite(8, HIGH);
      mcp.digitalWrite(9, HIGH);
      Serial.println(F("HighTemp & HighHumidity"));
    }
  }
  if (pressAuto == 0){
    if (realT < Val && realH < HumiHigh){ //Temperature
      mcp.digitalWrite(6, HIGH);
      mcp.digitalWrite(7, LOW);
      mcp.digitalWrite(8, HIGH);
      mcp.digitalWrite(9, LOW);
      Serial.println(F("LowTemp & LowHumidity"));
      }
    else if (realT < Val && realH > HumiHigh+5){
      mcp.digitalWrite(6, LOW);
      mcp.digitalWrite(7, LOW);
      mcp.digitalWrite(8, HIGH);
      mcp.digitalWrite(9, HIGH);
      Serial.println(F("LowTemp & HighHumidity"));
      }
    if (realT > Val+0.5 && realH < HumiHigh){ //Humidity
      mcp.digitalWrite(6, LOW);
      mcp.digitalWrite(7, HIGH);
      mcp.digitalWrite(8, LOW);
      mcp.digitalWrite(9, LOW);
      Serial.println(F("HighTemp & LowHumidity"));
    }
    else if (realT > Val+0.5 && realH > HumiHigh+5){
      mcp.digitalWrite(6, LOW);
      mcp.digitalWrite(7, HIGH);
      mcp.digitalWrite(8, HIGH);
      mcp.digitalWrite(9, HIGH);
      Serial.println(F("HighTemp & HighHumidity"));
    }
  }
}

void updateLCD(){   //update data from sensor and show on LCD.
  lcd.setCursor(7,1);
  lcd.print(realT);
  lcd.setCursor(13,1);
  lcd.print("C");
  lcd.setCursor(7,2);
  lcd.print(realH);
  lcd.setCursor(13,2);
  lcd.print("%");
  if (pressAuto == 1){
    lcd.setCursor(13,0);
    lcd.print("[BREAD]");
    lcd.setCursor(15,3);
    lcd.print(limitHigh);
  }
  if (pressAuto == 2){
    lcd.setCursor(12,0);
    lcd.print("[TEMPEH]");
    lcd.setCursor(15,3);
    lcd.print(limitHigh);
  }
  if (pressAuto == 0){
    lcd.setCursor(12,0);
    lcd.print("[MANUAL]");
    lcd.setCursor(15,3);
    lcd.print(Val);
  }
}

void ChangeTemp(){
  delay(140);
  if(mcp.digitalRead(4)== 1){
    Val = Val+1;
    Serial.print(F("Threshold Temp: "));
    Serial.print(Val);
    Serial.println();
  }
  if(mcp.digitalRead(5)==1){
    Val = Val-1;
    Serial.print(F("Threshold Temp: "));
    Serial.print(Val);
    Serial.println();
  }
}

void KeyBotton(){ //Key Botton 4 PIN Select mode
  delay(140);
  if(mcp.digitalRead(0) == 0){
    Blynk.begin(auth, ssid, pass);
    Serial.print(F("Blynk connected"));
    Serial.println();
  }
  else if(mcp.digitalRead(1) == 0){
    pressAuto = 1;
    limitHigh = 28;
    HumiHigh  = 50;
    Serial.print(F("Bread Botton"));
    Serial.println();
  }
  else if(mcp.digitalRead(2) == 0){
    pressAuto = 2;
    limitHigh = 32;
    HumiHigh  = 50;
    Serial.print(F("Tempeh Botton"));
    Serial.println();
  }
  else if(mcp.digitalRead(3) == 0){
    pressAuto = 0;
    HumiHigh  = 50;
    Serial.print(F("Manual Botton"));
    Serial.println();
  }
}

BLYNK_WRITE(V0) { 
  int control = param.asInt();
  if(control == 1){
    pressAuto = 1;
    limitHigh = 28;
    HumiHigh  = 50;
    Serial.print(F("Bread Botton"));
    Serial.println();
  }
}

BLYNK_WRITE(V1) { 
  int value = param.asInt();
  if(value == 1){
    pressAuto = 2;
    HumiHigh  = 50;
    Serial.print(F("Tempeh Botton"));
    Serial.println();
  }
}

BLYNK_WRITE(V4) { 
  int manual = param.asInt();
  if(manual == 1){
    pressAuto = 0;
    HumiHigh  = 50; 
    Serial.print(F("Manual Botton"));
    Serial.println();
  }
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
}

void setup() {
  Serial.begin(115200);
//setup mcp23017
  mcp.begin();
  mcp.pinMode(0, INPUT); //key1
  mcp.pinMode(1, INPUT); //key2
  mcp.pinMode(2, INPUT); //key3
  mcp.pinMode(3, INPUT); //key4
  mcp.pinMode(4, INPUT); //Plus  key
  mcp.pinMode(5, INPUT); //Minus key
  
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);
  mcp.pinMode(8,OUTPUT);//relay
  mcp.pinMode(9,OUTPUT);
  mcp.digitalWrite(6, HIGH); //fan1
  mcp.digitalWrite(7, HIGH); //light
  mcp.digitalWrite(8, HIGH); //fan2
  mcp.digitalWrite(9, HIGH); //steam
  
//setup lcd(Debug console)
  dht.begin();
  lcd.init();  
  lcd.backlight();
  lcd.setCursor(0,1);
  lcd.print("Temp : ");
  lcd.setCursor(0,2);
  lcd.print("Humid: ");
  lcd.setCursor(0,3);
  lcd.print("Setting Temp: ");
 
//setup  pin degital output.
  timer.setInterval(10,    KeyBotton);
  timer.setInterval(10,    ChangeTemp);
  timer.setInterval(1000,  updateLCD);
  timer.setInterval(1000,  condition);
  timer.setInterval(10000, processSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}
