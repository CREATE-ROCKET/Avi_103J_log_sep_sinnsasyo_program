#include <Arduino.h>
#include <CANCREATE.h>
#include <ICM20602.h>

#define CAN_RX 47
#define CAN_TX 48
#define led_pin 11

#define SCK 17
#define MISO 16
#define MOSI 18
#define ICMCS 8
#define SPIFREQ 1200000

CAN_CREATE CAN(true);


ICM icm42688;
SPICREATE::SPICreate SPIC;

hw_timer_t *timer = NULL;

const int tikatika_syuuki = 500;
int tikatika_count = 0;
bool tikatika_flag = false;
bool tikatika_ison = false;

u_int64_t heikina = 0;
int kasokudoc = 0;

int16_t ICM_data[6];
uint8_t ICM_rawdata[6];

int pcount = 0;

bool risyou = false;

IRAM_ATTR void counter()
{ // 1msで呼ばれる
  heikina += ((ICM_data[0]) * (ICM_data[0]) + (ICM_data[1]) * (ICM_data[1]) + (ICM_data[2]) * (ICM_data[2])) * 16.0 * 16.0 / 32768.0 / 32768.0;
  if (pcount % 20 == 1)
  {
    if (heikina / 20 > 1)
    {
      kasokudoc++;
      if (kasokudoc > 2) // ここを変える テストのときは2 本番は50
      {
        tikatika_flag = true;
        Serial.print("risyou");
        // risyoudetect();//離床検知
      }
    }
    else
    {
      kasokudoc = 0;
    }
    heikina = 0;
  }
  // 以下LEDちかちか
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
  tikatika_count++;
  pcount++;
}
int degree = 0;
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  //while (!Serial);

  Serial.println("CAN Reciever");
  CAN.setPins(CAN_RX, CAN_TX);

  if (!CAN.begin(100E3))
  {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  }
  // icm
  SPIC.begin(SPI2_HOST, SCK, MISO, MOSI);
  icm42688.begin(&SPIC, ICMCS, SPIFREQ);
  // タイマー割り込み
  timer = timerBegin(0, getApbFrequency() / 1000000, true);
  timerAttachInterrupt(timer, &counter, false);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);

  // LED
  pinMode(led_pin, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  icm42688.Get(ICM_data, ICM_rawdata);

  if (CAN.available())
  {
    char cmd = (char)CAN.read();
    Serial.print(cmd);
    CAN.sendPacket(0x13, cmd); // 受信確認用

    if (cmd == 'o')
    { // サーボを回す。（開ける
      //degree = 90;
      tikatika_flag = false;
    }
    else if (cmd == 'c')
    { // サーボを元に戻す（閉める
      //degree = 0;
      tikatika_flag = true;
    }
    // myservo.write(degree);
  }
  delay(1);
}