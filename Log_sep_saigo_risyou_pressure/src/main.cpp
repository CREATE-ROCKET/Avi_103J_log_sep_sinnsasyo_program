#include <Arduino.h>
#include <LPS25HB.h>

#define SCK 17
#define MISO 16
#define MOSI 18
#define LPSCS 7
#define led_pin 11
#define ICMCS 8
#define LPS25HB_WHO_AM_I 0xBD // LPS25HBの固有ID (10進数で189)

SPICREATE::SPICreate SPIC1;
LPS lps;

u_int8_t LPS_temp[3] = {0, 0, 0};
volatile u_int8_t LPS25_data[3] = {0, 0, 0};
spi_host_device_t host_in = SPI3_HOST;

int lcount = 1; // ここが0だと0～5の6回分のデータの加算を40m×6=0.12sごとに行うことになってしまう
volatile int bx = 0;
volatile int bh = 0;
int bc = 0;
const int tikatika_syuuki = 500;
volatile int tikatika_count = 0;
volatile bool tikatika_flag = false;
volatile bool risyou_flag = false;
bool tikatika_ison = false;
int pcount = 0;
int frmax = 200;
volatile int u = 0;
volatile int v = 0;
volatile int vv = 0;
volatile bool suuti = false;
volatile int risyou_count = 0;
bool risyou_flag_local;
int risyou_count_local;

hw_timer_t *timer = NULL;

IRAM_ATTR void counter()
{ // 1msで呼ばれる
  suuti = true;
  if (pcount % 40 == 1)
  {

    bx += ((LPS25_data[0] + LPS25_data[1] * 256 + LPS25_data[2] * 65536) * 200) / 4096; // ヘクトパスカル*200の値が格納されています
    v = bh;
    if (lcount == 20) // 40→5に変更
    {

      u = bh - bx / 20;                   // 変えた
      if ((bh - bx / 20) >= 2 && bh != 0) // ここを変える テストのときは4 本番は20⇐今回は0.1hPa=20だから合ってる 前回と今回が逆になっていたので調整
      {
        bc++;
        if (bc == 5)
        {
          tikatika_flag = true;
          risyou_flag = true;
          risyou_count += 1;
        }
      }
      else
      {
        bc = 0;
      }
      bh = bx / 20;
      bx = 0;
      lcount = 0;
    }
    lcount++;
  }

  // 以下LEDちかちか

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
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
  pinMode(ICMCS, OUTPUT);
  digitalWrite(ICMCS, HIGH);

  // digitalWrite(led_pin, LOW);
}

void loop()
{

  uint8_t who_am_i = lps.WhoAmI();
  // WhoAmI値が正しいかどうかで処理を分岐
  if (who_am_i == LPS25HB_WHO_AM_I)
  {
    // --- 成功した場合：気圧データを取得して表示 ---
    Serial.print("センサー接続OK。(WhoAmI=0x");
    Serial.print(who_am_i, HEX); // WhoAmIの値を16進数で表示
    Serial.println(")");
    if (suuti)
    {
      noInterrupts();
      vv = v;
      suuti = false;
      interrupts();
      // Serial.print(millis());    //時刻を表示
      // Serial.print(",");
      Serial.print(millis());
      Serial.print(",");
      Serial.println(vv);
    }
    // put your main code here, to run repeatedly:
    noInterrupts();
    risyou_flag_local = risyou_flag;
    risyou_count_local = risyou_count;
    if (risyou_flag_local)
    {
      risyou_flag = false;
    }
    interrupts();
    if (risyou_flag_local && risyou_count_local == 1)
    {
      Serial.println("risyou");
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

    lps.Get(LPS_temp);
    noInterrupts();
    LPS25_data[0] = LPS_temp[0];
    LPS25_data[1] = LPS_temp[1];
    LPS25_data[2] = LPS_temp[2];
    interrupts();

    // Serial.println(tikatika_flag);
    // Serial.print("dif");
    // Serial.println(u);
  }
  else
  {
    //--- WhoAmIが正しくない場合：エラーメッセージを表示 ---
    Serial.print("センサー接続失敗。 (WhoAmI=0x");
    Serial.print(who_am_i, HEX);
    Serial.println(")");
  }
  delay(40); // LPS25のデータはとっていないとき0となるのではなく前回の値が残り続けるので1msごとに取得する必要はなく40msごとで十分
}
