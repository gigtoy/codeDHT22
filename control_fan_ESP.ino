#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"

#include <MCP3208.h>    //Connect MCP3208
#include <SPI.h>
MCP3208 adc(D8);        //CS Port connect to ESP Digital Pin

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

#define DHTPIN D4       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define FAN_PIN D2      // FAN RELAY
#define Buzz_PIN D3     // Buzz Relay
BlynkTimer timer;

char auth[] = "WFEg_fIV42IcCDZf-xWtydsZejKRNzzQ";
char ssid[] = "Henvisut2012";    //wifi name
char pass[] = "line6387";       //wifi password

int Val = 0;
int i = 0;
int pressAuto;
float avtemp = 0;
float avhumi = 0;
float sumH[20];
float sumT[20];
float limitHigh;
float realT;
float realH;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

void processSensor(){
  for (i=0; i<=19; i++){
    delay(250);
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
  Blynk.virtualWrite(V6, realT);
  Blynk.virtualWrite(V5, realH);
  
//set average temp&humidity to zero
  avtemp = 0;
  avhumi = 0;
}

void condition(){// Compare Threshold value from Blynk and DHT Temperature value.
  if ((pressAuto == 1)||(pressAuto == 2)||(pressAuto == 3)){
    if (realT < limitHigh)
      {
      digitalWrite(FAN_PIN, HIGH); //turn off
      digitalWrite(Buzz_PIN, HIGH);
      Serial.print(F("auto1"));
      Serial.println();
      }
    else{
      digitalWrite(FAN_PIN, LOW);
      digitalWrite(Buzz_PIN, HIGH);//turn on
      Serial.print(F("auto2"));
      Serial.println();
      }
  }
  if (pressAuto == 0){
    if (realT > Val){
      digitalWrite(FAN_PIN, HIGH); //turn off
      digitalWrite(Buzz_PIN, HIGH);
      Serial.print(F("m1"));
      Serial.println();
     }
    else{
      digitalWrite(FAN_PIN, LOW);
      digitalWrite(Buzz_PIN, HIGH);//turn on
      Serial.print(F("m2"));
      Serial.println();
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
  }
  if (pressAuto == 2){
    lcd.setCursor(13,0);
    lcd.print("[NATTO]");
  }
  if (pressAuto == 3){
    lcd.setCursor(12,0);
    lcd.print("[TEMPEH]");
  }
  if (pressAuto == 0){
    lcd.setCursor(12,0);
    lcd.print("[MANUAL]");
  }
}

void ChangeTemp(){
  delay(170);
  if(adc.analogRead(0)>=2000){
    Val = Val+1;
    Serial.print(F("Theshold Temp: "));
    Serial.print(Val);
    Serial.println();
  }
  if(adc.analogRead(1)>=2000){
    Val = Val-1;
    Serial.print(F("Theshold Temp: "));
    Serial.print(Val);
    Serial.println();
  }
}

void KeyBotton(){ //Key Botton 4 PIN Select mode
  delay(170);
  if(adc.analogRead(2) < 300){
    pressAuto = 1;
    limitHigh = 30;
    Serial.print(F("Bread Botton"));
    Serial.println();
  }
  else if(adc.analogRead(3) < 300){
    pressAuto = 3;
    limitHigh = 30;
    Serial.print(F("Tempeh Botton"));
    Serial.println();
  }
  else if(adc.analogRead(4) < 300){
    pressAuto = 2;
    limitHigh = 30; 
    Serial.print(F("Natto Botton"));
    Serial.println();
  }
  else if(adc.analogRead(5) < 300){
    pressAuto = 0;
    Serial.print(F("Manual Botton"));
    Serial.println();
  }
}

BLYNK_WRITE(V0) { //Segment switch choose what to do
  switch (param.asInt())
  {
    case 1: { //Bread
        pressAuto = 1;
        limitHigh = 30; 
        Serial.print(F("Blynk High is:  "));
        Serial.print(limitHigh);
        Serial.println();
        break;
      }
    case 2: { //Natto
        pressAuto = 2;
        limitHigh = 30; 
        Serial.print(F("Blynk High is:  "));
        Serial.print(limitHigh);
        Serial.println();
        break;
      }
    case 3: { //Tempeh
        pressAuto = 3;
        limitHigh = 30; 
        Serial.print(F("Blynk High is:  "));
        Serial.print(limitHigh);
        Serial.println();
        break;
      }
    case 4: { //Manual
        pressAuto = 0;
        Serial.print(F("Blynk Manual: "));
        Serial.print(pressAuto);
        Serial.println();
        break;
      }
    }
}

BLYNK_WRITE(V3){
    Val = param.asInt(); // assigning incoming value from pin V3 to a variable
    Serial.print(" The Threshhold value is: ");
    Serial.println(Val);
    Serial.println();
}

BLYNK_CONNECTED(){
  Blynk.syncAll();
}

void setup() {
  Serial.begin(115200);
//setup mcp3208
  adc.begin();
  
//setup lcd(Debug console)
  dht.begin();
  lcd.init();  
  lcd.backlight();
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.setCursor(0,2);
  lcd.print("Humid: ");
  
//setup  pin degital output.
  pinMode(FAN_PIN, OUTPUT);
  pinMode(Buzz_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(Buzz_PIN, LOW);
  Blynk.begin( auth, ssid , pass );
  timer.setInterval(10L, KeyBotton);
  timer.setInterval(10L, ChangeTemp);
  timer.setInterval(1000L, updateLCD);
  timer.setInterval(1000L, condition);
  timer.setInterval(10000L, processSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}
