//レビュー　小数点が捨てられている
#include <Arduino.h>
#include <CANCREATE.h>
#include <ICM42688.h>
#include <SPICREATE.h>

#define SCK 17
#define MISO 16
#define MOSI 18
#define ICMCS 8
#define SPIFREQ 1200000
#define CAN_RX 47
#define CAN_TX 48

#define led_pin 11

ICM icm42688;
SPICREATE::SPICreate SPIC;
CAN_CREATE CAN(true);

hw_timer_t *timer = NULL;

int heikina = 0;
int kasokudoc = 0;
const int tikatika_syuuki = 500;
volatile int tikatika_count = 0;
volatile bool tikatika_flag = false;
bool tikatika_ison = false;
int icm_count = 0;
char Data = 'a';

int16_t ICM_temp[6];
volatile int16_t ICM_data[6];

spi_host_device_t host_in = SPI2_HOST;

int pcount = 0;

IRAM_ATTR void counter()
{ // 1msで呼ばれる
  
  if (pcount % 1 == 0) // 本番は60→1
  {
    heikina += ((ICM_data[0]) * (ICM_data[0]) + (ICM_data[1]) * (ICM_data[1]) + (ICM_data[2]) * (ICM_data[2])) * 16.0 * 16.0 / 32768.0 / 32768.0;
    if (icm_count % 20==1)
    { 
      if (heikina / 20 > 1) // 本番は4
      {
        kasokudoc++;
        if (kasokudoc > 2) // ここを変える テストのときは2 本番は50
        {
          tikatika_flag = true;

          // risyoudetect();//離床検知
        }

      }
      else
      {
        kasokudoc = 0;
      }
      heikina = 0;
      
    }
    icm_count++;
  }

  // 以下LEDちかちか

  tikatika_count++;
  pcount++;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPIC.begin(SPI2_HOST, SCK, MISO, MOSI);
  icm42688.begin(&SPIC, ICMCS, SPIFREQ);

  Serial.println("CAN Sender");
  // 100 kbpsでCANを動作させる
  if (CAN.begin(100E3, CAN_RX, CAN_TX, 10))
  {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  }

  // タイマー割り込み
  timer = timerBegin(0, getApbFrequency() / 1000000, true);
  timerAttachInterrupt(timer, &counter, false);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);

  // LED
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
}

void loop()
{
  if (tikatika_flag)
  {
    Serial.print("risyou");
  }
  if (tikatika_count >= tikatika_syuuki)
  {
    tikatika_count = 0;
    if (tikatika_flag)
    {
      if (tikatika_ison)
      {
        tikatika_ison = false;
        digitalWrite(led_pin, LOW);
      }
      else
      {
        tikatika_ison = true;
        digitalWrite(led_pin, HIGH);
      }
    }
    else
    {
      digitalWrite(led_pin, HIGH);
    }
  }
  // put your main code here, to run repeatedly:
  icm42688.Get(ICM_temp);
  noInterrupts();
  ICM_data[0] = ICM_temp[0];
  ICM_data[1] = ICM_temp[1];
  ICM_data[2] = ICM_temp[2];
  interrupts();

  if (CAN.available()) // CAN受信用
  {
    // CANを受信していたら実行される
    if (CAN.read(&Data))
    { // エラーの場合の処理
      Serial.println("failed to get CAN data");
    }
    else
      Serial.printf("Can received!!!: %c\n", Data);
  }
  if (Data == 's'){
    tikatika_flag = false;
  }

  // Serial.print(ICM_data[0] * 16.0 / 32768.0);
  // Serial.print(",");
  // Serial.print(ICM_data[1] * 16.0 / 32768.0);
  // c
  // Serial.println(ICM_data[2] * 16.0 / 32768.0); // 単位はg⇒デフォで1になるはず
  //Serial.println( ((ICM_data[0]) * (ICM_data[0]) + (ICM_data[1]) * (ICM_data[1]) + (ICM_data[2]) * (ICM_data[2])) * 16.0 * 16.0 / 32768.0 / 32768.0);
  //Serial.println(tikatika_flag);
  Serial.println(heikina);
  //Serial.print(",");
  //Serial.print(pcount);
  //Serial.print(",");
  //Serial.print(icm_count);
  //Serial.print(",\n");

  delay(60);
}
