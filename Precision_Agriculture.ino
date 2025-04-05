#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define buzzerPin 15 
#define DHT_PIN 4
#define DHT_TYPE DHT11  
#define SOIL_MOIS 32
#define LDR_PIN 35
#define RAIN_PIN 18
#define AIR_GAS 39
#define RED_ONE 25
#define GREEN_ONE 26
#define BLUE_ONE 27
#define RED_TWO 14
#define GREEN_TWO 12
#define BLUE_TWO 13
#define RED_THREE 23
#define GREEN_THREE 33
#define BLUE_THREE 5
#define servoPin 16

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE); 
Servo irrigationMotor;

unsigned long previousMillis = 0;  
int displayState = 0;             

void setup() {
  Serial.begin(9600);
  dht.begin();  
  // Pin Configurations
  pinMode(RED_ONE, OUTPUT);
  pinMode(GREEN_ONE, OUTPUT);
  pinMode(BLUE_ONE, OUTPUT); 
  pinMode(RED_TWO, OUTPUT);
  pinMode(GREEN_TWO, OUTPUT);
  pinMode(BLUE_TWO, OUTPUT);
  pinMode(BLUE_THREE, OUTPUT); 
  pinMode(RED_THREE, OUTPUT);
  pinMode(GREEN_THREE, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(SOIL_MOIS, INPUT);
  pinMode(LDR_PIN,INPUT);
  pinMode(RAIN_PIN,INPUT);
  pinMode(AIR_GAS,INPUT);
  irrigationMotor.attach(servoPin);
  
  // LCD Setup 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Precision");
  lcd.setCursor(0,1);
  lcd.print("Agriculture");
  delay(3000);
  lcd.clear();
}

void loop() {

  // mois_perc & Auto-irrigation with Rain detection
  int mois_analog=analogRead(SOIL_MOIS);
  int mois_perc= (1-(mois_analog/4095))*100;

  int rainVal=digitalRead(RAIN_PIN);
  String irrigationStatus="Not Irrigating";
  int ldr=digitalRead(LDR_PIN);
  String lightSensing="ACTIVE";
  if (ldr==HIGH){
    lightSensing="NO";
  } else if (ldr==LOW){
    lightSensing="YES";
  }

  if (ldr==HIGH || rainVal==LOW){
    irrigationStatus="Not Irrigating";
    irrigationMotor.write(0);
    digitalWrite(RED_ONE,HIGH);
    digitalWrite(BLUE_ONE,LOW);
    digitalWrite(GREEN_ONE,LOW);
    delay(700);
    digitalWrite(RED_ONE,LOW);
    digitalWrite(BLUE_ONE,LOW);
    digitalWrite(GREEN_ONE,HIGH);
    delay(700);
  }
  if (ldr==LOW && rainVal==HIGH && mois_perc <=30){
    irrigationStatus="Irrigating...";
    digitalWrite(RED_ONE,HIGH);
    digitalWrite(BLUE_ONE,LOW);
    digitalWrite(GREEN_ONE,LOW);
    digitalWrite(buzzerPin,HIGH);
    irrigationMotor.write(0);
    delay(500);
    irrigationMotor.write(180);
    delay(500);
  } else if (ldr==LOW && rainVal==HIGH && mois_perc>30 && mois_perc<60){
    irrigationStatus="Not Irrigating";
    digitalWrite(RED_ONE,LOW);
    digitalWrite(BLUE_ONE,HIGH);
    digitalWrite(GREEN_ONE,LOW);
    digitalWrite(buzzerPin,LOW);
  } else if (ldr==LOW && rainVal==HIGH && mois_perc>=60){
    irrigationStatus="Not Irrigating";
    digitalWrite(RED_ONE,LOW);
    digitalWrite(BLUE_ONE,LOW);
    digitalWrite(GREEN_ONE,HIGH); 
    digitalWrite(buzzerPin,LOW);
  }

  // Air Quality & Gas Detection
  int airAnalog = analogRead(AIR_GAS);
  float airPercentage = (airAnalog / 4095.0) * 100;
  float airPercentageInv = 100-airPercentage;
  Serial.println(airPercentageInv);

  if (airAnalog <= 2000) {
    digitalWrite(RED_TWO, LOW);
    digitalWrite(BLUE_TWO, LOW);
    digitalWrite(GREEN_TWO, HIGH);
    digitalWrite(buzzerPin,LOW);
  } else if (airAnalog > 2000 && airAnalog < 3000) {
    digitalWrite(RED_TWO, LOW);
    digitalWrite(BLUE_TWO, HIGH);
    digitalWrite(GREEN_TWO, LOW);  
    digitalWrite(buzzerPin,LOW); 
  } else {
    digitalWrite(RED_TWO, HIGH);
    digitalWrite(BLUE_TWO, LOW);
    digitalWrite(GREEN_TWO, LOW);
    digitalWrite(buzzerPin,HIGH);
  }

  // Temperature & Humidity
  float temperature = dht.readTemperature();
  float roundedTemp=round(temperature*10)/(10.0);
  int humidity = dht.readHumidity(); 

  if (humidity<70 && humidity>=40){
    digitalWrite(GREEN_THREE,HIGH);
    digitalWrite(BLUE_THREE, LOW);
    digitalWrite(RED_THREE, LOW);
    digitalWrite(buzzerPin,LOW);
  } else if (humidity>=30 && humidity<40)){
    digitalWrite(GREEN_THREE,LOW);
    digitalWrite(BLUE_THREE, HIGH);
    digitalWrite(RED_THREE, LOW);
    digitalWrite(buzzerPin,LOW);
  } else if (humidity<30){
    digitalWrite(GREEN_THREE,LOW);
    digitalWrite(BLUE_THREE, LOW);
    digitalWrite(RED_THREE, HIGH); 
    digitalWrite(buzzerPin,HIGH);
  } else if (humidity>=70 && humidity<85)){
    digitalWrite(GREEN_THREE,LOW);
    digitalWrite(BLUE_THREE, HIGH);
    digitalWrite(RED_THREE, LOW);
    digitalWrite(buzzerPin,LOW);
  } else if (humidity>=85){
    digitalWrite(GREEN_THREE,LOW);
    digitalWrite(BLUE_THREE, LOW);
    digitalWrite(RED_THREE, HIGH); 
    digitalWrite(buzzerPin,HIGH);

  // LCD Updates
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 3000) {  
    previousMillis = currentMillis;
    lcd.clear();
    lcd.setCursor(0, 0);

    if (displayState==0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AIR");
      lcd.setCursor(0,1);
      lcd.print(airPercentageInv);
      lcd.print("%");

      lcd.setCursor(4,0);
      lcd.print("|");
      lcd.setCursor(4,1);
      lcd.print("|");

      lcd.setCursor(5, 0);
      lcd.print("TEMP");
      lcd.setCursor(5,1);
      lcd.print(roundedTemp,1);

      lcd.setCursor(10,0);
      lcd.print("|");
      lcd.setCursor(10,1);
      lcd.print("|");

      lcd.setCursor(11, 0);
      lcd.print("HUMID");
      lcd.setCursor(11,1);
      lcd.print(humidity);
      displayState = 1; 
  
    } else if (displayState==1){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MOISTURE");
      lcd.setCursor(0,1);
      lcd.print(mois_perc);
      lcd.print("%");

      lcd.setCursor(9,0);
      lcd.print("|");
      lcd.setCursor(9,1);
      lcd.print("|");

      lcd.setCursor(11, 0);
      lcd.print("LIGHT");
      lcd.setCursor(11,1);
      lcd.print(lightSensing);
      displayState = 2;  

    } else if (displayState==2){
      lcd.print("WATER MOTOR :");
      lcd.setCursor(0,1);
      lcd.print(irrigationStatus);
      displayState=0;
    }
  }
}
