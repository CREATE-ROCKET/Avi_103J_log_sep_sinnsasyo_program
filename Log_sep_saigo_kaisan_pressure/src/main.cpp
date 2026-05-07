#include <Arduino.h>
#include <LPS25HB.h>
// ESP32Servo.hは消した(今回のサーボでいらない)
// Servo myservo;⇐これも要らない

#define SCK 17
#define MISO 16
#define MOSI 18
#define LPSCS 7
//(#define ServoPin 12は消した)
#define MIN1 9
#define MIN2 10 // 追加した
#define led_pin 11
#define ICMCS 8
#define LPS25HB_WHO_AM_I 0xBD // LPS25HBの固有ID (10進数で189)

SPICREATE::SPICreate SPIC1;
LPS lps;

hw_timer_t *timer = NULL;

volatile u_int8_t LPS25_data[3] = {0, 0, 0};

const int tikatika_syuuki = 500;
int tikatika_count = 0;
volatile bool tikatika_flag = false; // これがtrueならLED点滅する
volatile bool kaisan_flag = false;
bool tikatika_ison = false;          // これがONでないと点滅は始まらない。なぜあるのかはわからない

int lcount = 1; // ここは1に変更
volatile int kaisan_count = 0;
volatile int bx = 0;
volatile int bh = 0;
volatile int br = 0;
volatile int bkc = 0;
volatile int v = 0;
int pcount = 0;
volatile int u = 0;

bool kaisan_flag_local;
int kaisan_count_local;
int br_local;

const int frmax = 200;

spi_host_device_t host_in = SPI3_HOST;

IRAM_ATTR void counter()
{
  // 1msで呼ばれる
  if (pcount % 20 == 0)
  {
    bx += ((LPS25_data[0] + LPS25_data[1] * 256 + LPS25_data[2] * 65536) * 200) / 4096; // ヘクトパスカル*200の値が格納されています
    br = ((LPS25_data[0] + LPS25_data[1] * 256 + LPS25_data[2] * 65536) * 200) / 4096;  //生データ
    if (lcount == 10) // 5(10にした)回の平均(40になっていたので変更)
    {
      v = bx / 10;
      u = bx / 10 - bh;

      if (bx / 10 > bh && bh != 0) // 前回と今回が逆になってた
      {
        bkc++;
        if (bkc == 5) // ここを変える テストのときは4 本番は9
        {
          kaisan_flag = true; // 開傘条件クリア
          tikatika_flag = true;
          kaisan_count += 1;
        }
      }
      else
      {
        bkc = 0;
      }
      bh = bx / 10;
      bx = 0;
      lcount = 0;
    }
    lcount++;
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
        digitalWrite(MIN1, HIGH);
        digitalWrite(MIN2, HIGH);
      }
      else
      {
        tikatika_ison = true;
        digitalWrite(led_pin, HIGH);
        digitalWrite(MIN1, LOW);
        digitalWrite(MIN2, HIGH);
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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPIC1.begin(SPI3_HOST, SCK, MISO, MOSI);
  lps.begin(&SPIC1, LPSCS, 6000000);

  // タイマー割り込み
  timer = timerBegin(0, getApbFrequency() / 1000000, true);
  timerAttachInterrupt(timer, &counter, false);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);
  // MOTORドライバ
  pinMode(MIN1, OUTPUT);
  pinMode(MIN2, OUTPUT);
  digitalWrite(MIN1, LOW);
  digitalWrite(MIN2, LOW);
  // LED
  pinMode(led_pin, OUTPUT);

  pinMode(ICMCS, OUTPUT);
  digitalWrite(ICMCS, HIGH);
}

void loop()
{
  uint8_t who_am_i = lps.WhoAmI();
  // WhoAmI値が正しいかどうかで処理を分岐
  if (who_am_i == LPS25HB_WHO_AM_I)
  {
    // --- 成功した場合：気圧データを取得して表示 ---
    // Serial.print("センサー接続OK。(WhoAmI=0x");
    // Serial.print(who_am_i, HEX); // WhoAmIの値を16進数で表示
    // Serial.println(")");
    uint8_t tmp[3];
    lps.Get(tmp);
    noInterrupts();
    LPS25_data[0] = tmp[0];
    LPS25_data[1] = tmp[1];
    LPS25_data[2] = tmp[2];
    interrupts();
    
    noInterrupts();
    kaisan_flag_local = kaisan_flag;
    kaisan_count_local = kaisan_count;
    br_local = br;
    if (kaisan_flag)
      {
        kaisan_flag = false;
      }
    interrupts();
    if (kaisan_flag_local && kaisan_count_local == 1)
      {
        Serial.println("kaisan");
      }
      Serial.print(millis());
      Serial.print(",");
      Serial.println(br_local);

    // Serial.print("dif");
    // Serial.println(u);
    
  }else{
  // --- WhoAmIが正しくない場合：エラーメッセージを表示 ---
  // Serial.print("センサー接続失敗。 (WhoAmI=0x");
  // Serial.print(who_am_i, HEX);
  // Serial.println(")");
  }
  delay(20);
}