#include <ESP8266mDNS.h>
#include "Ticker.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <Wire.h>                                                       // required by BME280 library
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
#include "BH1750.h"
#include "eepromapi.h"
#include "INDEXHTM.h"
#include "IPHTM.h"
#include "STYLECSS.h"
#include "HELPHTM.h"

#define LED_PIN	2
#define RESET_PIN 0
// D1 en D2, (GPIO5 and GPIO4)  are SCL resp SDA  and used by the BME280
// #define FORCE_DEEPSLEEP //comment out when you like to use battery level dependent sleep
#define SLEEP_PIN 13
#define debugSerial Serial

struct SensorVol{
  float temp;
  float pres;
  float lumi;
  float humi;
} sensorNow;

int resetTick = 0;
int nmTick = 0, timeSensor = 0;
int timeNarod = 5;
bool bmeStatus = false, lightStatus = false, onSensor = false, onNarod = false;

const char* upgradeIndex = R"=====(<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>)=====";

ADC_MODE(ADC_VCC); //use getVcc

Adafruit_BME280 bme;
BH1750 lightMeter;
EepromClass eepromapi;
Ticker blinker, sensor, stopAll;
ESP8266WebServer HTTP(80);

void sensorTik();
void tickBlink();
void smart_res();
void startServer();
void sensor_json();
void mem_set();
void narodmonSend();
void sensor_xml();
void read_sensor();

void hold(const unsigned int &ms) {
	unsigned long m = millis();
	while (millis() - m < ms) {
		yield();
	}
}

void AP_mode_default(){
	blinker.attach(3, tickBlink);
	String nameDev = WiFi.macAddress();
	nameDev.replace(":", "");
	nameDev = "WEATHER_" + nameDev;
	char AP_ssid_default[32]; 
	nameDev.toCharArray(AP_ssid_default, nameDev.length() + 1);
	char AP_pass_default[32] = "12345678"; 
	WiFi.mode(WIFI_OFF);
	WiFi.softAP(AP_ssid_default, AP_pass_default);
	digitalWrite(LED_PIN, HIGH);
}

void STATION_mode(IOTconfig customVar){
	blinker.attach(0.5, tickBlink);
	WiFi.mode(WIFI_OFF);
	WiFi.mode(WIFI_STA);
	WiFi.begin(customVar.STA_ssid, customVar.STA_pass);
	hold(100);
	while (WiFi.status() != WL_CONNECTED){
		debugSerial.print(".");
		if(WiFi.status() == WL_NO_SSID_AVAIL){}
		if(WiFi.status() ==  WL_CONNECT_FAILED){}
	}

	if (customVar.dhc == 0xaa55) {
		IPAddress gateway = WiFi.gatewayIP();
		IPAddress subnet = WiFi.subnetMask();
		WiFi.config(customVar.statIp, gateway, subnet);
	}
	if (MDNS.begin("iot_home", WiFi.localIP())) {
		debugSerial.println("MDNS responder started");
	}
	blinker.detach();
	digitalWrite(LED_PIN, HIGH);
}

void scanwifi_json() {
	int n = WiFi.scanNetworks();
	String outStr = "{\"scan\":[";
	// '{"sacn":[]}'
	if (n > 0){
		// '{"sacn":[{"sidd":"lvm1976","rssi":"66666666"},{"sidd":"roll","rssi":"12345678"}]}' 
		for (int i = 0; i < n; i++){
			outStr += "{\"ssid\":\"";
			outStr += WiFi.SSID(i);
			outStr += "\", ";
			outStr += "\"mac\":\"";
			outStr += WiFi.BSSIDstr(i);
			outStr += "\", ";
			outStr += "\"channel\":";
			outStr += WiFi.channel(i);
			outStr += ", ";
			outStr += "\"pass\":";
			outStr += WiFi.encryptionType(i) != ENC_TYPE_NONE ? "true" : "false";
			outStr += ", ";
			outStr += "\"hidden\":";
			outStr += WiFi.isHidden(i)? "true" : "false";
			outStr += ", ";
			outStr += "\"rssi\":";
			outStr += WiFi.RSSI(i);
			outStr += "}";
			if ((i + 1) != n) outStr += ", ";
		}
	}
	outStr += "]}";
	HTTP.send(200,"application/json",outStr);
	WiFi.scanDelete();
}

void setup() {
	debugSerial.begin(115200);
	debugSerial.println("\n start");
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);
	pinMode(SLEEP_PIN, INPUT_PULLUP);
	pinMode(RESET_PIN, INPUT_PULLUP);
	Wire.begin(5, 4);
	eepromapi.eeprom_init();
	bmeStatus = bme.begin(0x76); //bme.begin(address)  0x76 or 0x77
	if (!bmeStatus) debugSerial.println("no BME280 detected");
	if (bmeStatus) {
		debugSerial.println("BME280 Advanced begin");
		bme.setSampling(Adafruit_BME280::MODE_NORMAL,
						Adafruit_BME280::SAMPLING_X1, // temperature
						Adafruit_BME280::SAMPLING_X1, // pressure
						Adafruit_BME280::SAMPLING_X1, // humidity
						Adafruit_BME280::FILTER_OFF   );
	}
	lightStatus = lightMeter.begin();
	if (!lightStatus) debugSerial.println("no BH1750 detected");
	if (lightStatus) debugSerial.println("BH1750 Advanced begin");
	hold(1000);
	sensor.attach(0.1, sensorTik);
	IOTconfig customVar = eepromapi.eeprom_get();
	if (customVar.wifimode == 0) AP_mode_default();
	else STATION_mode(customVar);
	timeNarod = customVar.timeNarod;
	#ifdef FORCE_DEEPSLEEP
		if (digitalRead(SLEEP_PIN) == 0) {
			read_sensor();
			narodmonSend();
			int tt = (timeNarod != 0)?timeNarod:5;
			debugSerial.println("Засыпаем на " + String(tt) + " минут");
			ESP.deepSleep(tt * 60000000);          // спать на 5 минут пины D16 и  RST должны быть соеденены между собой
		}
	#endif
	startServer();
}

void loop() {
	#ifdef FORCE_DEEPSLEEP
		if (digitalRead(SLEEP_PIN) == 0) ESP.reset();
	#endif
	if (onSensor) read_sensor();
	if (onNarod) narodmonSend();
	HTTP.handleClient();
	MDNS.update();
}

void sensorTik() {
	if (digitalRead(RESET_PIN) == 0) {
		if (resetTick++ > 100) smart_res();
	} else resetTick = 0;
	if (timeNarod != 0)
		if (nmTick++ > (timeNarod * 600)) 
			onNarod = true;
	if (timeSensor++ > 5) onSensor = true;
}

void tickBlink() {
	int state = digitalRead(LED_PIN);
	digitalWrite(LED_PIN, !state);
}

void smart_res() {
	debugSerial.println("Reset devise");
	WiFi.disconnect(true);
	eepromapi.eeprom_clr();
	hold(5000);
	ESP.reset();
}

void startServer() {
    HTTP.on("/", HTTP_GET, [](){HTTP.send(200, "text/html", homeIndex);});
    HTTP.on("/style.css", HTTP_GET, [](){HTTP.send(200, "text/css", style);});
	HTTP.on("/sensor.json", HTTP_GET, sensor_json);
	HTTP.on("/sensor.xml", HTTP_GET, sensor_xml);
    HTTP.on("/ip.htm", HTTP_GET, [](){HTTP.send(200, "text/html", ipIndex);});
	HTTP.on("/scanwifi.json", HTTP_GET, scanwifi_json);
    HTTP.on("/help", HTTP_GET, [](){HTTP.send(200, "text/html", helpIndex);});
	HTTP.on("/mem", HTTP_GET, mem_set);
	HTTP.on("/default", HTTP_GET, [](){
		HTTP.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
		hold(5000);
		smart_res();
		});
	HTTP.on("/reboot", HTTP_GET, [](){
		HTTP.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
		hold(5000);
		ESP.restart();
		hold(500);
		});
    HTTP.on("/upgrade", HTTP_GET, [](){HTTP.send(200, "text/html", upgradeIndex);});
    HTTP.on("/update", HTTP_POST, [](){
		HTTP.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
		hold(5000);
		ESP.restart();
		hold(500);
    },[](){
		HTTPUpload& upload = HTTP.upload();
		if(upload.status == UPLOAD_FILE_START){
			debugSerial.setDebugOutput(true);
			debugSerial.printf("Update: %s\n", upload.filename.c_str());
			uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
			if(!Update.begin(maxSketchSpace)){
				Update.printError(debugSerial);
			}
		} else if(upload.status == UPLOAD_FILE_WRITE){
			if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
				Update.printError(debugSerial);
			}
		} else if(upload.status == UPLOAD_FILE_END){
			if(Update.end(true)){
				debugSerial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
			} else {
				Update.printError(debugSerial);
			}
			debugSerial.setDebugOutput(false);
		}
		yield();
    });
	HTTP.begin();
	debugSerial.println("http server started");
}

void sensor_xml() {
	String relayXml = "<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n<sensor>\n\t<mac>"
		+ WiFi.macAddress()
		+ "</mac>\n\t<btem>" + String(sensorNow.temp)
		+ "</btem>\n\t<pres>" + String(sensorNow.pres)
		+ "</pres>\n\t<humi>" + String(sensorNow.humi)
		+ "</humi>\n\t<lumi>" + String(sensorNow.lumi)
		+ "</lumi>\n\t<RSSI>" + WiFi.RSSI()
		+ "</RSSI>\n</sensor>";	
	HTTP.send(200,"text/xml",relayXml);
}

void sensor_json() {
	double battV = ESP.getVcc() / 1023.000;
	String relayJson = "";
	relayJson +=  "{\"sensor\": {\"mac\":\"";
	relayJson += WiFi.macAddress();
	relayJson += "\",\"batt\":\"" + String(battV);
	relayJson += "\",\"temp\":\"" + String(sensorNow.temp);
	relayJson += "\",\"pressure\":\"" + String(sensorNow.pres);
	relayJson += "\",\"humidity\":\"" + String(sensorNow.humi);
	relayJson += "\",\"light\":\"" + String(sensorNow.lumi);
	relayJson += "\",\"RSSI\":\"" + String(WiFi.RSSI());
	relayJson += "\",\"narodTime\":\"" + String(timeNarod);
	relayJson += "\"}}";	
	HTTP.send(200,"application/json",relayJson);
}

void mem_set() {
	boolean flag = false;
	boolean rst = false;
	HTTP.send(200,"application/json","{\"status\":\"ok\"}");
	IOTconfig customVar = eepromapi.eeprom_get();
	if (HTTP.arg("ipStat") != "") {
		flag = true;
		IPAddress ip;
		ip.fromString(HTTP.arg("ipStat"));
		IPAddress gateway = WiFi.gatewayIP();
		IPAddress subnet = WiFi.subnetMask();
		WiFi.config(ip, gateway, subnet);
		hold(1000);
		customVar.statIp = ip;
		customVar.dhc = 0xaa55;
	}
	if (HTTP.arg("ssid") != "") {
		flag = true;
		String str = HTTP.arg("ssid");
    	str.toCharArray(customVar.STA_ssid, str.length() + 1);
		customVar.wifimode = 1;
		rst = true;
	}
	if (HTTP.arg("pass") != "") {
		flag = true;
		String str = HTTP.arg("pass");
    	str.toCharArray(customVar.STA_pass, str.length() + 1);
		customVar.wifimode = 1;
		rst = true;
	}
	if (HTTP.arg("narodTime") != "") {
		int t = HTTP.arg("narodTime").toInt();
		customVar.timeNarod = ((t >= 5) && (t <= 60))?t:0;
		flag = true;
	}
	if (flag) {
		eepromapi.eeprom_set(customVar);
		timeNarod = customVar.timeNarod;
	}
	if (rst) {
		hold(5000);
		ESP.restart();
		hold(500);
	}
}

void narodmonSend () {
	tickBlink();
	WiFiClient client;
	double battV = ESP.getVcc() / 1023.000;
	String buf;
	String sleepMode = (digitalRead(SLEEP_PIN) == 1)?"0":"1";
	buf = "#" + WiFi.macAddress() + "#ESP"  +  ESP.getChipId()  + "\n";
	buf += "#rssi#" + String(WiFi.RSSI()) + "\n";
	buf += "#vbat#" + String(battV) + "\n";
	buf += "#sleepmode#" + sleepMode + "\n";
	buf += "#temp#" + String(sensorNow.temp) + "\n";
	buf += "#pres#" + String(sensorNow.pres) + "\n";
	buf += "#humi#" + String(sensorNow.humi) + "\n";
	buf += "#light#" + String(sensorNow.lumi) + "\n";
	buf += "##\n";
	debugSerial.println(buf);
	debugSerial.println("Соеденение с сервером narodmon.ru...."); 
	// if (client.connect("185.245.187.13", 8283))  {
	if (client.connect("narodmon.com", 8283)) {
		debugSerial.println("Отправляю данные.");
		client.print(buf);
	}
	hold(1000);
	tickBlink();
	onNarod = false;
	nmTick = 0;
}    

void read_sensor(){
	sensorNow.temp = 0;
	sensorNow.pres = 0;
	sensorNow.humi = 0;
	sensorNow.lumi = 0;
	if (bmeStatus){
		sensorNow.temp = bme.readTemperature();
		sensorNow.pres = bme.readPressure() / 133.32F;
		sensorNow.humi = bme.readHumidity();
	}
	if (lightStatus) 
		sensorNow.lumi = lightMeter.readLightLevel();
	timeSensor = 0;
	onSensor = false;
}