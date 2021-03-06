/*
 Name:		cloud_led2.ino
 Created:	2020/9/27 14:00:07
 Author:	joe
*/

//ws2812库
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
//ssd3306库
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//获取天气库
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>                 //使用JSON-v5版的库

#define BLYNK_PRINT Serial
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include "DHT.h"
#define DHTPIN 14     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float h, t;


char auth[] = "BidLe1JUPW3aGswQDs5j08pREtmXmYMo";

static const char ntpServerName[] = "time.nist.gov";
const int timeZone = 8; //BeiJing in China
int tmp;//温度
String  shimiao;
String  yueri;

WiFiUDP Udp;
WiFiClient client;
const char ssid[] = "lang";  //  your network SSID (name)

const char pass[] = "57954626";       // your network password
#define PIN 12 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);
int index_wave[591] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4,4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,16, 16, 17, 18, 18, 19, 20, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 30, 31, 32, 33, 34, 36, 37, 38, 40, 41, 43, 45, 46, 48, 50, 52,54, 56, 58, 60, 62, 65, 67, 70, 72, 75, 78, 81, 84, 87, 90, 94, 97, 101, 105, 109, 113, 117, 122, 126, 131, 136, 141, 146, 152, 158,164, 170, 176, 183, 190, 197, 205, 213, 221, 229, 238, 247, 256, 256, 247, 238, 229, 221, 213, 205, 197, 190, 183, 176, 170, 164, 158,152, 146, 141, 136, 131, 126, 122, 117, 113, 109, 105, 101, 97, 94, 90, 87, 84, 81, 78, 75, 72, 70, 67, 65, 62, 60, 58, 56, 54, 52, 50,48, 46, 45, 43, 41, 40, 38, 37, 36, 34, 33, 32, 31, 30, 28, 27, 26, 25, 25, 24, 23, 22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 15, 14,14, 13, 13, 12, 12, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3,3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };      //创建一个数组，用来储存每次延时的亮度

unsigned int localPort = 8888;  // local port to listen for UDP packets

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // ssd3306-i2c通讯

time_t getNtpTime();

void digitalClockDisplay();

void printDigits(int digits);

void sendNTPpacket(IPAddress& address);

/*  请求的Json数据格式如下：
 * {
 *    "results": [
 *        {
 *            "location": {
 *                "id": "WX4FBXXFKE4F",
 *                "name": "北京",
 *                "country": "CN",
 *                "path": "北京,北京,中国",
 *                "timezone": "Asia/Shanghai",
 *                "timezone_offset": "+08:00"
 *            },
 *            "now": {
 *                "text": "多云",
 *                "code": "4",
 *                "temperature": "23"
 *            },
 *            "last_update": "2019-10-13T09:51:00+08:00"
 *        }
 *    ]
 *}
 */



void setup()
{
    Serial.begin(115200);
    strip.begin();
    strip.setBrightness(75);
    strip.show(); // Initialize all pixels to 'off'
    dht.begin();

    u8g2.begin();

    wifi_start_connect();
    client.setTimeout(5000);//设置服务器连接超时时间


    Udp.begin(localPort);

    Serial.print("Local port: ");

    Serial.println(Udp.localPort());

    Serial.println("waiting for sync");

    setSyncProvider(getNtpTime);

    setSyncInterval(300);
}
time_t prevDisplay = 0; // when the digital clock was displayed

void loop()
{
    Blynk.run();

    if (client.connect("api.seniverse.com", 80) == 1)              //连接服务器并判断是否连接成功，若成功就发送GET 请求数据下发
    {                                           //换成你自己在心知天气申请的私钥//改成你所在城市的拼音
        client.print("GET /v3/weather/now.json?key=S7bz8bp0khYowfDK6&location=xining&language=zh-Hans&unit=c HTTP/1.1\r\n"); //心知天气的URL格式
        client.print("Host:api.seniverse.com\r\n");
        client.print("Accept-Language:zh-cn\r\n");
        client.print("Connection:close\r\n\r\n"); //向心知天气的服务器发送请求。


        String status_code = client.readStringUntil('\r');        //读取GET数据，服务器返回的状态码，若成功则返回状态码200
        Serial.println(status_code);

        if (client.find("\r\n\r\n") == 1)                            //跳过返回的数据头，直接读取后面的JSON数据，
        {
            String json_from_server = client.readStringUntil('\n');  //读取返回的JSON数据
            Serial.println(json_from_server);
            parseUserData(json_from_server);                      //将读取的JSON数据，传送到JSON解析函数中进行显示。
        }
    }
    else
    {
        Serial.println("connection failed this time");
        delay(5000);                                            //请求失败等5秒
    }

    client.stop();                                            //关闭HTTP客户端，采用HTTP短链接，数据请求完毕后要客户端要主动断开https://blog.csdn.net/huangjin0507/article/details/52396580




    //显示时间函数
    if (timeStatus() != timeNotSet)
    {

        if (now() != prevDisplay)
        { //update the display only if time has changed

            prevDisplay = now();

            digitalClockDisplay();

        }

    }
    readdht();

    dislay();


    delay(3000);
}








void dislay() {

    u8g2.clearBuffer();


    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, 15);
    u8g2.print(tmp);

    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(17, 15);
    u8g2.print("C");


    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(27, 15);
    u8g2.print("/");
    u8g2.setCursor(32, 15);
    u8g2.print(t);
    u8g2.setCursor(60, 15);
    u8g2.print("C");
    u8g2.setCursor(75, 15);
    u8g2.print(h);
    u8g2.setCursor(97, 15);
    u8g2.print("%");


    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 64, "XN");
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.drawGlyph(110, 64, 0x2665);


    u8g2.setFont(u8g2_font_helvB24_tf);
    u8g2.setCursor(27, 50);
    u8g2.print(shimiao);

    u8g2.setFont(u8g2_font_helvB12_tf);
    u8g2.setCursor(54, 64);
    u8g2.print(yueri);



    u8g2.sendBuffer();


}


void readdht() {
    h = dht.readHumidity();

    t = dht.readTemperature();
}

//连接WIFI
void wifi_start_connect()
{
    WiFi.mode(WIFI_STA);

    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (cnt++ >= 10) {
            WiFi.beginSmartConfig();
            while (1) {
                delay(1000);
                if (WiFi.smartConfigDone()) {
                    Serial.println();
                    Serial.println("SmartConfig: Success");
                    break;
                }
               
                
                Serial.print("|");
            }
        }
    }

    WiFi.printDiag(Serial);

    Blynk.config(auth);
}

void parseUserData(String content)  // Json数据解析并串口打印.可参考https://www.bilibili.com/video/av65322772
{
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 210;
    DynamicJsonBuffer jsonBuffer(capacity);

    JsonObject& root = jsonBuffer.parseObject(content);

    JsonObject& results_0 = root["results"][0];

    JsonObject& results_0_location = results_0["location"];
    const char* results_0_location_id = results_0_location["id"];
    const char* results_0_location_name = results_0_location["name"];
    const char* results_0_location_country = results_0_location["country"];
    const char* results_0_location_path = results_0_location["path"];
    const char* results_0_location_timezone = results_0_location["timezone"];
    const char* results_0_location_timezone_offset = results_0_location["timezone_offset"];

    JsonObject& results_0_now = results_0["now"];
    const char* results_0_now_text = results_0_now["text"];
    const char* results_0_now_code = results_0_now["code"];
    const char* results_0_now_temperature = results_0_now["temperature"];
    const char* results_0_last_update = results_0["last_update"];

    Serial.println(results_0_location_name);                       //通过串口打印出需要的信息
   /* Serial.println(results_0_now_text);
    Serial.println(results_0_now_code);
    Serial.println(results_0_now_temperature);
    Serial.println(results_0_last_update);
    Serial.print("\r\n");*/


    tmp = results_0_now["temperature"].as<int>();

    /* Serial.print("temp===");
     Serial.print(tmp);*/

     //colorWipe(strip.Color(139, 69, 19), 50); //  	 	马鞍棕色



    if (tmp <= 15)
    {
        colorWipe3(strip.Color(0, 139, 139), 1); // 


    }
    else
    {
        colorWipe3(strip.Color(218, 165, 32), 1); //  	 	

    }
    // u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.drawGlyph(83, 64, 0x2665);
    u8g2.sendBuffer();
    //ceshi11




}


void digitalClockDisplay()

{

    // digital clock display of the time

    //Serial.print(hour());

    //printDigits(minute());

    //printDigits(second());

    //Serial.print(" ");

    //Serial.print(day());

    //Serial.print(".");

    //Serial.print(month());

    //Serial.print(".");

    //Serial.print(year());

    //Serial.println();

    shimiao = String(hour()) + ":" + String(minute());
    yueri = String(month()) + "-" + String(day());


    /* Serial.print("yueri==");
     Serial.print(yueri);*/


}







/*-------- NTP code ----------*/


void printDigits(int digits)

{

    // utility for digital clock display: prints preceding colon and leading 0

    Serial.print(":");

    if (digits < 10)

        Serial.print('0');

    Serial.print(digits);

}
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets



time_t getNtpTime()

{

    IPAddress ntpServerIP; // NTP server's ip address



    while (Udp.parsePacket() > 0); // discard any previously received packets

    Serial.println("Transmit NTP Request");

    // get a random server from the pool

    WiFi.hostByName(ntpServerName, ntpServerIP);

    Serial.print(ntpServerName);

    Serial.print(": ");

    Serial.println(ntpServerIP);

    sendNTPpacket(ntpServerIP);

    uint32_t beginWait = millis();

    while (millis() - beginWait < 1500) {

        int size = Udp.parsePacket();

        if (size >= NTP_PACKET_SIZE) {

            Serial.println("Receive NTP Response");

            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer

            unsigned long secsSince1900;

            // convert four bytes starting at location 40 to a long integer

            secsSince1900 = (unsigned long)packetBuffer[40] << 24;

            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;

            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;

            secsSince1900 |= (unsigned long)packetBuffer[43];

            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;

        }

    }

    Serial.println("No NTP Response :-(");

    return 0; // return 0 if unable to get the time

}



// send an NTP request to the time server at the given address

void sendNTPpacket(IPAddress& address)

{

    // set all bytes in the buffer to 0

    memset(packetBuffer, 0, NTP_PACKET_SIZE);

    // Initialize values needed to form NTP request

    // (see URL above for details on the packets)

    packetBuffer[0] = 0b11100011;   // LI, Version, Mode

    packetBuffer[1] = 0;     // Stratum, or type of clock

    packetBuffer[2] = 6;     // Polling Interval

    packetBuffer[3] = 0xEC;  // Peer Clock Precision

    // 8 bytes of zero for Root Delay & Root Dispersion

    packetBuffer[12] = 49;

    packetBuffer[13] = 0x4E;

    packetBuffer[14] = 49;

    packetBuffer[15] = 52;

    // all NTP fields have been given values, now

    // you can send a packet requesting a timestamp:

    Udp.beginPacket(address, 123); //NTP requests are to port 123

    Udp.write(packetBuffer, NTP_PACKET_SIZE);

    Udp.endPacket();

}

//2812显示

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

void colorWipe1(uint32_t c, uint8_t wait) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}


void colorWipe3(uint32_t c, uint8_t wait) {

    for (int b = 0; b < 591; b++)
    {
        strip.setBrightness(index_wave[b]);
        for (uint16_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
            strip.show();
            delay(wait);
        }
    }





}