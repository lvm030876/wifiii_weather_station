#ifndef config_h
#define config_h
#include "Arduino.h"
#include <ESP8266mDNS.h>
#include "Ticker.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include "WiFiManager.h"
#include <Wire.h>                                                       // required by BME280 library
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include "eepromclass.h"
#include "index.h"
#include "ipstat.h"


// #define PROGMEM   ICACHE_RODATA_ATTR
#define LED_PIN 2
#define RESET_PIN 0
// #define ALARM_1 5	//датчик руху
// #define ALARM_2 4	//RS пульт	4
// #define SEN_PIN 1	//радіомодуль	1

#define SLEEP_PIN 13 // нужно определится
// D1 en D2, (GPIO5 and GPIO4)  are SCL resp SDA  and used by the BME280
//#define FORCE_DEEPSLEEP //comment out when you like to use battery level dependent sleep
#define debugSerial Serial

struct SensorVol{
  float temp;
  float pres;
  float lumi;
  float humi;
} sensorNow;

void sensorTik();
void tickBlink();
void smart_res();
void startServer();
void sensor_json();
void mem_set();
void narodmonSend();
void sensor_xml();
void read_sensor();

int resetTick = 0;
int nmTick = 0, timeSensor = 0;
int timeNarod = 5;
bool bmeStatus = false, lightStatus = false, onSensor = false, onNarod = false;

#endif