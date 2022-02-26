#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
#include <SimpleTimer.h>
#include "Adafruit_MCP23017.h"
#include <LiquidCrystal_I2C.h>

#define DHTPIN D4       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

SimpleTimer timer;
Adafruit_MCP23017 mcp;
LiquidCrystal_I2C lcd(0x27,20,4);

char auth[] = "WFEg_fIV42IcCDZf-xWtydsZejKRNzzQ";
char ssid[] = "Gigtoy1";           //wifi name*********************************************
char pass[] = "kenpeepo12";        //wifi password******************************************Don't forget to change Username and password.

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
  Blynk.virtualWrite(V6, realT);
  Blynk.virtualWrite(V5, realH);
  
//set average temp&humidity to zero
  avtemp = 0;
  avhumi = 0;
}

void condition(){// Compare Threshold value from Blynk and DHT Temperature value.
  if (realT > 50){
    mcp.digitalWrite(8,  HIGH);
    mcp.digitalWrite(9,  HIGH);
    mcp.digitalWrite(10, LOW);
    mcp.digitalWrite(11, HIGH);
  }
  if ((pressAuto == 1)||(pressAuto == 2)){
    if (realT < limitHigh){ //Cold
      mcp.digitalWrite(8,  HIGH);
      mcp.digitalWrite(9,  LOW);
      mcp.digitalWrite(10, HIGH);
      mcp.digitalWrite(11, HIGH);
      Serial.print(F("auto1"));
      Serial.println();
      }
    else{
      mcp.digitalWrite(8,  LOW);
      mcp.digitalWrite(9,  HIGH);
      mcp.digitalWrite(10, HIGH);
      mcp.digitalWrite(11, LOW);
      Serial.print(F("auto2"));
      Serial.println();
      }
  }
  if (pressAuto == 0){
    if (realT > Val){  //Hot
      mcp.digitalWrite(8,  LOW);
      mcp.digitalWrite(9,  HIGH);
      mcp.digitalWrite(10, HIGH);
      mcp.digitalWrite(11, LOW);
      Serial.print(F("m1"));
      Serial.println();
     }
    else{
      mcp.digitalWrite(8,  HIGH);
      mcp.digitalWrite(9,  LOW);
      mcp.digitalWrite(10, HIGH);
      mcp.digitalWrite(11, HIGH);
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
    lcd.setCursor(15,3);
    lcd.print(limitHigh);
  }
  if (pressAuto == 2){
    lcd.setCursor(13,0);
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
  delay(150);
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
  delay(150);
  if(mcp.digitalRead(0) == 0){
    Blynk.begin(auth, ssid , pass);
    Serial.print(F("Blynk connected"));
    Serial.println();
  }
  else if(mcp.digitalRead(1) == 0){
    pressAuto = 1;
    limitHigh = 28;
    Serial.print(F("Bread Botton"));
    Serial.println();
  }
  else if(mcp.digitalRead(2) == 0){
    pressAuto = 2;
    limitHigh = 37; 
    Serial.print(F("Tempeh Botton"));
    Serial.println();
  }
  else if(mcp.digitalRead(3) == 0){
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
        limitHigh = 28;
        Serial.print(F("Blynk Bread"));
        Serial.print(limitHigh);
        Serial.println();
        break;
      }
    case 2: { //Tempeh
        pressAuto = 2;
        limitHigh = 37; 
        Serial.print(F("Blynk Tempeh"));
        Serial.print(limitHigh);
        Serial.println();
        break;
      }
    case 3: { //Manual
        pressAuto = 0;
        Serial.print(F("Blynk Manual"));
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
//setup mcp23017
  mcp.begin();
  mcp.pinMode(0, INPUT); //key1
  mcp.pinMode(1, INPUT); //key2
  mcp.pinMode(2, INPUT); //key3
  mcp.pinMode(3, INPUT); //key4
  mcp.pinMode(4, INPUT); //Plus  key
  mcp.pinMode(5, INPUT); //Minus key
  
  mcp.pinMode(8, OUTPUT);
  mcp.pinMode(9, OUTPUT);
  mcp.pinMode(10,OUTPUT);//relay
  mcp.pinMode(11,OUTPUT);
  mcp.digitalWrite(8,  HIGH); //fan
  mcp.digitalWrite(9,  HIGH); //light
  mcp.digitalWrite(10, HIGH); //buzzer
  mcp.digitalWrite(11, HIGH); //steam
  
//setup lcd(Debug console)
  dht.begin();
  lcd.init();  
  lcd.backlight();
  lcd.setCursor(0,1);
  lcd.print("Temp : ");
  lcd.setCursor(0,2);
  lcd.print("Humid: ");
  lcd.setCursor(0,3);
  lcd.print("Starting Temp: ");
 
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
