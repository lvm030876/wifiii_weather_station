#include "config.h"
ADC_MODE(ADC_VCC); //use getVcc ss


Adafruit_BME280 bme;
BH1750 lightMeter(0x23);
EepromClass eepromapi;
Ticker blinker, sensor, stopAll;
ESP8266WebServer HTTP(80);

void setup() {
	debugSerial.begin(115200);
	debugSerial.println("\n start");
	pinMode(LED_PIN, OUTPUT);
	blinker.attach(0.6, tickBlink);
	pinMode(SLEEP_PIN, INPUT_PULLUP);
	pinMode(RESET_PIN, INPUT_PULLUP);
	eepromapi.eeprom_init();

	bmeStatus = bme.begin(); //bme.begin(address)  0x76 or 0x77
	if (!bmeStatus) debugSerial.println("no BME280 detected");
	if (bmeStatus) {
		debugSerial.println("BME280 Advanced begin");
		bme.setSampling(Adafruit_BME280::MODE_NORMAL,
						Adafruit_BME280::SAMPLING_X1, // temperature
						Adafruit_BME280::SAMPLING_X1, // pressure
						Adafruit_BME280::SAMPLING_X1, // humidity
						Adafruit_BME280::FILTER_OFF   );
	}
	lightStatus = lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
	if (!lightStatus) debugSerial.println("no BH1750 detected");
	if (lightStatus) debugSerial.println("BH1750 Advanced begin");
	delay(1000);
	lightMeter.readLightLevel();

	sensor.attach(0.1, sensorTik);
	WiFiManager wifiManager;
	wifiManager.setTimeout(180);
	wifiManager.setAPCallback([](WiFiManager *myWiFiManager){blinker.attach(0.5, tickBlink);});
	if (!wifiManager.autoConnect("iot_home")) {
		debugSerial.println("failed to connect and hit timeout");
		ESP.reset();
	}
	delay(3000);
	blinker.detach();
	digitalWrite(LED_PIN, HIGH);
	IOTconfig customVar = eepromapi.eeprom_get();
	timeNarod = customVar.timeNarod;
	if (digitalRead(SLEEP_PIN) == 0) {
		read_sensor();
		narodmonSend();
		int tt = (timeNarod != 0)?timeNarod:5;
		debugSerial.println("Засыпаем на " + String(tt) + " минут");
		ESP.deepSleep(tt * 60000000);          // спать на 5 минут пины D16 и  RST должны быть соеденены между собой
	}
	if (customVar.dhc == 0xaa55) {
		IPAddress gateway = WiFi.gatewayIP();
		IPAddress subnet = WiFi.subnetMask();
		WiFi.config(customVar.statIp, gateway, subnet);
	}
	if (MDNS.begin("iot_home", WiFi.localIP())) {
		debugSerial.println("MDNS responder started");
	}
	startServer();
}

void loop() {
	if (digitalRead(SLEEP_PIN) == 0) ESP.reset();
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
	delay(5000);
	ESP.reset();
}

void startServer() {
    HTTP.on("/", HTTP_GET, [](){HTTP.send(200, "text/html", homeIndex);});
    HTTP.on("/style.css", HTTP_GET, [](){HTTP.send(200, "text/css", style);});
	HTTP.on("/sensor.json", HTTP_GET, sensor_json);
	HTTP.on("/sensor.xml", HTTP_GET, sensor_xml);
    HTTP.on("/ip.htm", HTTP_GET, [](){HTTP.send(200, "text/html", ipIndex);});
	HTTP.on("/mem", HTTP_GET, mem_set);
	HTTP.on("/default", HTTP_GET, smart_res);
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
	HTTP.send(200,"application/json","{\"status\":\"ok\"}");
	IOTconfig customVar = eepromapi.eeprom_get();
	if (HTTP.arg("ipStat") != "") {
		flag = true;
		IPAddress ip;
		ip.fromString(HTTP.arg("ipStat"));
		IPAddress gateway = WiFi.gatewayIP();
		IPAddress subnet = WiFi.subnetMask();
		WiFi.config(ip, gateway, subnet);
		delay(1000);
		customVar.statIp = ip;
		customVar.dhc = 0xaa55;
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
	if (client.connect("narodmon.ru", 8283)) {
		debugSerial.println("Отправляю данные.");
		client.print(buf);
	}
	delay(1000);
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
