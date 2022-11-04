/*
 Toelichting:

 Om LCD aan te sluiten moet GND naar GND, VCC naar 5V, SDA naar A4 en SCL naar A5
 (https://circuitsgeek.com/guides-and-how-to/how-to-connect-pcf8574-i2c-lcd-with-arduino/)

 Te installeren Arduino library is LiquidCrystal_PCF8574

 SW8 is de knop (verbonden met digitale pin 8) waarmee de meting wordt gereset.

 Analog pin 0 is de input pin voor het hartsignaal.
*/
// creates a "virtual" serial port/UART
// connect BT module TX to D10
// connect BT module RX to D11
// connect BT Vcc to 5V, GND to GND

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

#include <SoftwareSerial.h> // remove the inverted commas after you copy the code to the IDE




SoftwareSerial BT(10, 11); 

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

float max_v = -100;
float min_v = 100;
float range_finding_delay = 1.5;
float lcd_delay = 1000;
float lcd_timer = 0;
float lcd_start = 0;
int up = 0;
int peaks = 0;
unsigned long start_time;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
//  Serial.begin(9600);
  BT.begin(9600);
  // Send test message to other device
  BT.println("Hello from Arduino");
  start_time = millis()/1000.0;
  lcd_start = millis();
  pinMode(8,INPUT_PULLUP);

  int error;
//    Serial.begin(115200);
  Serial.println("LCD...");

  // See http://playground.arduino.cc/Main/I2cScanner
  Wire.begin();
  Wire.beginTransmission(0x27); //Your LCD Address
  error = Wire.endTransmission(); 
  Serial.print("Error: ");
  Serial.print(error);
  if (error == 0) {

    Serial.println(": LCD found.");

  } else {
    Serial.println(": LCD not found.");
  } // if

  lcd.begin(16, 2); // initialize the lcd
}
// the loop routine runs over and over again forever:
void loop() {
  float current_time = millis()/1000.0;
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  max_v = max(max_v/1.001,voltage);
  min_v = min(min_v*1.001,voltage);
  float r_max = max_v - min_v;
  float r_voltage = voltage - min_v;
  float u_thresh = r_max*0.6;
  float l_thresh = r_max*0.2;
  bool new_peak = false;

  float time_delta = current_time - start_time;
  if (time_delta>range_finding_delay)
  {
  
  if (up!=1 && r_voltage >= u_thresh){
    up = 1;
    new_peak = true;
    peaks += 1;
  }
  else if (up!=-1 && r_voltage <= l_thresh){
    up = -1;
    new_peak = true;
    peaks += 1;
  }

  float freq = peaks/(2*time_delta);
  int button_state = digitalRead(8);
  if (button_state == LOW) {
    start_time = millis()/1000.0;
    peaks = 0;
    lcd_start = start_time;
  }
  if (new_peak && time_delta > 10.0) {
    Serial.println(freq * 60.0);
//    start_time = millis()/1000.0;
//    peaks = 0;
//    lcd_start = millis();
  }
  lcd_timer = millis();
  if (lcd_timer - lcd_start > lcd_delay) {
    lcd_start = millis();
    
    lcd.setBacklight(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Uw hartslag is:");
    lcd.setCursor(0, 1);
    lcd.print(String(round(freq*60.0)) + String(" bpm "));
  }
  BT.print(time_delta);
  BT.print(',');
  BT.println(voltage);
//  Serial.println(up);
//  Serial.println(voltage);
//  Serial.println(r_voltage);
//  Serial.print(min_v);
//  Serial.print(',');
//  Serial.println(max_v);
//  Serial.print(l_thresh);
//  Serial.print(',');
//  Serial.println(u_thresh);
//  Serial.println(peaks);
//  Serial.println(' ');
  }
  delay(10);
}
