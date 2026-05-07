#include <Arduino.h>
#include <ICM42688.h>
#include <LPS25HB.h>
//(#include <ESP32Servo.h>)

//(Servo myservo;)

SPICREATE::SPICreate SPIC1;
LPS lps;


#define led_pin 11  
#define MIN1 9
#define MIN2 10
volatile u_int32_t ms_count = 0;
unsigned long long rot_time = 0;

// SPICREATE::SPICreate SPIC1;
// LPS lps;

u_int8_t LPS25_data[3] = {0, 0, 0};
u_int8_t last_modified_LPS25_data[3] = {0, 0, 0};
int kaisan_count = 0;

hw_timer_t *timer = NULL;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // SPIC1.begin(VSPI, SCK1, MISO1, MOSI1);
  // lps.begin(&SPIC1, LPSCS, 6000000);
//  タイマー割り込み

  pinMode(led_pin, OUTPUT);

  pinMode(MIN1,OUTPUT);
  pinMode(MIN2,OUTPUT);

  digitalWrite(led_pin, HIGH); //光ると離床
  digitalWrite(MIN1,LOW);
  digitalWrite(MIN2,LOW);
  rot_time = millis();
}

void loop() {
  if ((10000 < (millis()-rot_time)) && ((millis()-rot_time) < 13000)) // ここを変える
  {

  // put your main code here, to run repeatedly:
    digitalWrite(MIN1,HIGH);
    digitalWrite(MIN2,LOW);
    delay(3000);  //何秒回す？？？
    digitalWrite(MIN1,LOW);
    digitalWrite(MIN2,LOW);
  }
  //if(kaisan) {
  //  Serial.print("rotate");
  //  myservo.write(90);
  //  delay(1000);
  //}

  // Serial.print("rotate");
  // myservo.write(90);
  // delay(1000);
  // myservo.write(0);
  // delay(1000);
  // delay(1);
}