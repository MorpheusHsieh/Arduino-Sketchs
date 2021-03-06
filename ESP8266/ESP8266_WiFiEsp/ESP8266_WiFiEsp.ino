#include "DHT.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include "SoftwareSerial.h"
#include <WiFiEsp.h>        // 只能在傳輸速率 9600 時執行
#include <PubSubClient.h>

/*
 *  LCD Pins            Arduino Pin 
 *  ----------------------------------
 *  GND                 GND
 *  VCC                 5V
 *  SDA                 SDA
 *  SCL                 SCL
 *
 *  DHT Pins            Arduino Pin 
 *  ----------------------------------
 *  1 (VDD, 3~5.5V)     3.3V
 *  2 (Data)            D5
 *  3 (NC)
 *  4 (GND)             GND
 *
 *  ESP8266 Pin         Arduino Pin
 *  ----------------------------------
 *  1 (GND)             GND
 *  2 (GPIO2)           NC
 *  3 (GPIO0)           NC
 *  4 (RX)              D3
 *  5 (VCC)             3.3V (External Power)
 *  6 (RST)             NC
 *  7 (CH_PD)           3.3V
 *  8 (TX)              D4
 * 
 */

/* ---  Serial setup  --- */
#define BAUD_SERIAL       9600        // 序列埠傳輸速率設定為 9600 bps
#define DELAY_MS          1000        // 系統預設延遲時間

/* ---  LCD setup  */
LiquidCrystal_I2C LCD1602(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

/* ---  DHT  --- */
#define DHTPIN  5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/* ---  ESP8266 setup  --- */
/*
 * Be aware that 115200 will not work with software serial.
 * With Arduino Uno must set the a lower baud rate, ex. 9600
 */
#define BAUD_ESP8266      115200
#define WiFi_SSID         "xxxxxxxx"
#define WiFi_PSWD         "xxxxxxxx"

#define RXPIN     3                  // ESP8266 TX -> MCU D3
#define TXPIN     4                  // ESP8266 RX -> MCU D4
SoftwareSerial ESP8266(RXPIN, TXPIN);

/* ---  MQTT setup --- */
const char* MQTT_SERVER = "m16.cloudmqtt.com";
const int   MQTT_PORT   = 11344;
const char* MQTT_TOKEN  = "";

const char* clientID = "80:7d:3a:72:5d:5f"; // MAC address as Client ID
const char* topic = "Arduino/DHT11";        // MQTT topic


void setup() 
{
  Serial.begin(BAUD_SERIAL);
  Serial.println("Arduino...OK!");
  
  lcd_Setting();
  Serial.println("LCD...OK");

  dht.begin();
  Serial.println("DHT...OK!");

  wifi_Setting();
}

void loop()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) { 
    Serial.println("Failed to read from DHT sensor!");
    return;
  } 
  printSerial(h, t); printLCD(h, t);
  
  delay(DELAY_MS);
}

void lcd_Setting()
{
  // LCD initializing, default state is open backlight.
  LCD1602.begin(16, 2);      
  LCD1602.clear();

  // Flash three times
  for(int i = 0; i < 3; i++) {
    LCD1602.backlight();    // open backlight
    delay(250);
    LCD1602.noBacklight();  // close backlight
    delay(250);
  }
  LCD1602.backlight();

  // 輸出初始化文字
  LCD1602.setCursor(0, 0); // 設定游標位置在第一行行首
  LCD1602.print("LCD...OK!");
}

void wifi_Setting()
{
  int WiFi_Status = WL_IDLE_STATUS;
  
  ESP8266.begin(BAUD_ESP8266);
    
  // ESP module is initializing...
  WiFi.init(&ESP8266);

  Serial.print("Connect to ");
  Serial.print(WiFi_SSID); Serial.println(" ...");

  int i=0;
  do{
    WiFi_Status = WiFi.begin(WiFi_SSID, WiFi_PSWD);
    Serial.print(++i); Serial.print(" ");
    delay(500);
  } while (WiFi_Status != WL_CONNECTED);
  Serial.println();
    
  Serial.println("WiFi connection established!");
  Serial.print("IP Address: ");       
  Serial.println(WiFi.localIP());
}


void printSerial(float h, float t)
{
  String outstr = "Humi = " + String(h) + " %"
                +",\tTemp = " + String(t) + "°C ";
  Serial.println(outstr);
}

void printLCD(float h, float t)
{
  String row1 = "Humi: " + String(h) + " %";

  // '°' = (char)223
  String unit = String((char)223) + "C";
  String row2 = "Temp: " + String(t) + unit;

  LCD1602.clear();
  LCD1602.setCursor(0, 0); LCD1602.print(row1);
  LCD1602.setCursor(0, 1); LCD1602.print(row2);
}
