#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include <Adafruit_BMP085_U.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MCP23008.h>

#include <UTFT_DLB.h>
#include <UTFTUi.h>

#include <WundergroundClient.h>
#include <TimeClient.h>

#include "Settings.h"
#include "WebServer.h"



//#include <Ticker.h>										may be needed

//  Fonts
extern uint8_t DejaVuSans18plus[];
extern uint8_t DejaVuSans10[];
extern uint8_t DejaVuSans42[];
extern uint8_t Sans12[];
extern uint8_t meteocons42[];
extern uint8_t meteocons21[];

// Screen conctructor 
// UTFT_DLB TFT(    screen ,MISO,SCK,CS,MOSI,DC)
   UTFT_DLB TFT(ILI9341_S5P, D7, D5, D3, D6, D4);

//   ADS7846 ads;

//  BMP constructor 
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// WIFI settings
//const char* WIFI_SSID = "A72_common";						// A72_common
//const char* WIFI_PWD = "0peHBPT72";						// 0peHBPT72

// WiFi Server Settings
static const char WIFI_CONNECT_TIMEOUT = 10;  // in seconds

TimeClient timeClient(0);

//Adafruit_MCP23008 mcp;

// Set to false, if you prefere imperial/inches, Fahrenheit
WundergroundClient wunderground(true);

//Ticker ticker;

String currTemp = "";
String prevTemp = "";
String currPressure = "";
String prevPressure = "";
String currAltitude = "";
String prevAltitude = "";
String currHumidity = "";
String prevHumidity = "";
char currMinutes;
char prevMinutes = NULL;
char currSecond;
char prevSecond = NULL;
char currSecond2;
char prevSecond2 = NULL;

unsigned long currTime = 0;
unsigned long prevInfoUpdate = 0;
unsigned long lastUpdate = 0;

unsigned long CheckSensorsInterval =	 60000 * 10;
unsigned long updateInterval = 60000 * 60;

int buttonState = 0;
int lastButtonState = 0;
int lastDebounceTime = 0;

bool frameDrawn = false;

// UI settings
bool frame1(UTFT_DLB *TFT, UTFTUiState* state, int x, int y);
bool frame2(UTFT_DLB *TFT, UTFTUiState* state, int x, int y);

UTFTUi ui(&TFT);

FrameCallback frames[] = { mainFrame, infoFrame };

int numOfFrames = 2;

// functions headers
bool updateData(unsigned long updateInterval = 0);
bool connectWifi();
bool drawLocation(int x, int y, bool alignCenter = false);
bool drawWeather(int x, int y, bool alignCenter = false);
bool drawTime(int x, int y, bool forceUpdate = false, bool alignCenter = false);
bool drawDate(int x, int y, bool alignCenter = false);
bool drawForecast(int x, int y, bool alignCenter = false);
bool drawForecastDetails(int x, int y, int dayIndex);
bool drawInfoOverlay(int x, int y);
bool DrawCurrentInfo(int x, int y, unsigned long interval, bool alignCenter);
String getTemp();
String getPressure();
String getAltitude();


void setup()
{
	//core initlization
	Serial.begin(115200);
	Serial.printf("\n\n\n\n");
//	pinMode(D8, INPUT_PULLUP);

	//periphrals initialization
	Settings.begin();
	bmp.begin();
//	mcp.begin();

	// screen initilization and settings
	ui.InitLCD();
	TFT.clrScr();
	ui.setTimePerTransition(500);
	ui.setFrames(frames, numOfFrames);
	ui.disableAutoTransition();
	ui.setTimePerFrame(500);         


	//wifi and webserver initilization
	if (connectWifi()) {
		timeClient.updateOffset(Settings.utcOffset + 1);				//utc offset  broken theat's why +1
		updateData();
	}
	WebServer.begin();
}

void loop()
{
	
	currTime = millis();
	WebServer.loop();
	if (WiFi.status() == WL_CONNECTED) {
		ui.update();


//		//button
//		//int reading = digitalRead(D8);
//		//if (currTime - lastDebounceTime > 100) {
//			//if (reading == 1) {
//				ui.nextFrame();
//				frameDrawn = false;
//				lastDebounceTime = currTime;
//				Serial.println("Changing Frame");
//			}
////		}
	}
}

bool mainFrame(UTFT_DLB *dispaly, UTFTUiState* state, int x, int y) {
	//check if everything is drawn
	if (!frameDrawn) {
		drawWeather(20, 90, true);
		drawForecast(34, 152, true);
		drawInfoOverlay(230, 30);
		DrawCurrentInfo(230, 30,0, true);
		drawTime(20, 10, true, true);
		drawDate(20, 10, true);
		drawLocation(0, 210, true);
		frameDrawn = true;
	}

	drawTime(20, 10, false, true);
	DrawCurrentInfo(230, 30, CheckSensorsInterval, true);
	if (updateData(updateInterval)) {
		drawWeather(20, 90, true);
		drawDate(20, 10,true);
		drawForecast(34, 152, true);
		drawLocation(0, 210, true);
	}
}

bool infoFrame(UTFT_DLB *dispaly, UTFTUiState* state, int x, int y) {
	if (!frameDrawn) {
		TFT.setFont(DejaVuSans18plus);
		TFT.print("Web Server address:", 20, 20);
		TFT.print(WiFi.localIP().toString(), 20, 40);
		TFT.print("Hostname: ", 20, 60);
		TFT.print(Settings.hostname, 20, 80);
		TFT.print("Connected to:", 20, 100);
		TFT.print(Settings.ssid, 20, 120);
		frameDrawn = true;
	}
}

bool drawLocation(int x, int y, bool alignCenter) {
	if (Settings.country == '\0' || Settings.country == '\0') {
		Serial.println("Location Display Error");
		return false;
	}
	TFT.setFont(Sans12);
	String location = Settings.city + (String)", " + Settings.country;
	if (alignCenter) {
		TFT.print(location, (220 - TFT.getStringWidth(location))/2, y);
	}
	else {
		TFT.print((Settings.city + (String)", " + Settings.country), x, y);
	}
	return true;
}

bool updateData(unsigned long updateInterval) {								// if interval == 0; force update
	if (currTime - lastUpdate > updateInterval || updateInterval == 0) {
		timeClient.updateTime();
		String apikey = Settings.apikey;
		String country = Settings.country;
		String city = Settings.city;
		if (apikey.length() == 0 || country.length() == 0 || city.length() == 0) {
			Serial.println("Invalid data for weather update.");
			Serial.println("Please updata data from webserver.");
			TFT.print("Invalid weather data", 20, 115);
			TFT.print("Please update from webserver", 20, 135);
		}
		else
		{
			wunderground.updateConditions(Settings.apikey, "EN", Settings.country, Settings.city);
			wunderground.updateForecast(Settings.apikey, "EN", Settings.country, Settings.city);
			lastUpdate = currTime;
		}
		return true;
	}
	return false;
}

bool connectWifi() {
	// try to connect to Wifi
	if (Settings.ssid[0] != '\0') {
		WiFi.hostname("WeatherStation");
		Serial.printf("SSID: %s\nPassowrd: %s\n", Settings.ssid, Settings.password);
		WiFi.begin(Settings.ssid, Settings.password);
		WiFi.mode(WIFI_STA);
		int s = 0;
		while ((WiFi.status() != WL_CONNECTED) && (s < 2 * WIFI_CONNECT_TIMEOUT)) {
			delay(500);
			s++;
		}
	}

	// If didn't manage to connect go to AP mode
	if (WiFi.status() != WL_CONNECTED){
		WiFi.mode(WIFI_AP);
		WiFi.softAP("WeatherStation");
		Serial.printf("Going into AP mode\n");
		Serial.printf("IP address: \n");
		Serial.println(WiFi.softAPIP());
		TFT.setFont(DejaVuSans18plus);
		TFT.print("Wireless connection failed.", (TFT.getDisplayXSize() - TFT.getStringWidth("Wireless connection failed.")) / 2, 100);
		TFT.print("Connect to AP at: ", (TFT.getDisplayXSize() - TFT.getStringWidth("Connect to AP at: ")) / 2, 130);
		TFT.print(WiFi.softAPIP().toString(), (TFT.getDisplayXSize() - TFT.getStringWidth((String)WiFi.softAPIP())) / 2, 160);
		return false;
	}
	else {
		Serial.printf("\nConnected to: %s\n", Settings.ssid);
		Serial.printf("\nIP address: %s\n");
		Serial.println(WiFi.localIP());
		return true;
	}
}

//bool drawLoadBar(int x, int y, int length) {
//	TFT.setColor(VGA_WHITE);
//	TFT.drawRect(x, y, x + length, y+5);
//	for (int i = 1; i < length-4; i = i + length/30) {
//		TFT.drawHLine(x+2, y+2, i);
//		TFT.drawHLine(x+2, y+3, i);
//		delay(100);
//	}
//	return true;
//}

bool drawWeather(int x, int y, bool alignCenter) {

	if (Settings.city[0] != '\0' || Settings.country[0] != '\0' || Settings.apikey[0] != '\0') {
		TFT.setFont(DejaVuSans18plus);
		int tempOffset = TFT.getStringWidth(wunderground.getCurrentTemp() + +"°C");
		String wtext = wunderground.getWeatherText();
		int space = wtext.indexOf(" ");
		int textOffset;
		if (space != -1) {
			if (TFT.getStringWidth(wtext.substring(0, space)) > TFT.getStringWidth(wtext.substring(space + 1)))
				textOffset = TFT.getStringWidth(wtext.substring(0, space));
			else
				textOffset = TFT.getStringWidth(wtext.substring(space + 1));
		}
		else {
			textOffset = TFT.getStringWidth(wtext);
		}
		TFT.setFont(meteocons42);
		int iconOffset = TFT.getStringWidth(wunderground.getTodayIcon());
		int fullOffset = (220 - x - 16 - (tempOffset + iconOffset + textOffset)) / 2;

		//erase old text
		TFT.setColor(VGA_BLACK);
		TFT.fillRect(0, y, 220, y + 60);                //if needed
		TFT.setColor(VGA_WHITE);


		if (alignCenter) {

			TFT.setFont(DejaVuSans18plus);
			TFT.print(wunderground.getCurrentTemp() + "°C", x + fullOffset, y + 12);
			if (space != -1) {
				TFT.print(wtext.substring(0, space), x + tempOffset + iconOffset + 16 + fullOffset, y);
				TFT.print(wtext.substring(space + 1), x + tempOffset + iconOffset + 16 + fullOffset, y + 24);
			}
			else {
				TFT.print(wtext, x + tempOffset + iconOffset + 16 + fullOffset, y + 12);
			}
			TFT.setFont(meteocons42);
			TFT.print(wunderground.getTodayIcon(), x + tempOffset + 8 + fullOffset, y);
		}
		else {
			TFT.setColor(VGA_WHITE);
			TFT.setFont(DejaVuSans18plus);
			TFT.print(wunderground.getCurrentTemp() + "°C", x , y + 12);
			if (space != -1) {
				TFT.print(wtext.substring(0, space), x, iconOffset + 16, y);
				TFT.print(wtext.substring(space + 1), x + tempOffset + iconOffset + 16, y + 24);
			}
			else {
				TFT.print(wtext, x + tempOffset + iconOffset + 16, y + 12);
			}
			TFT.setFont(meteocons42);
			TFT.print(wunderground.getTodayIcon(), x + tempOffset + 8, y);
		}
	}
	else {
		return true;
	}
}

bool drawTime(int x, int y, bool forceUpdate, bool alignCenter) {

	TFT.setFont(DejaVuSans42);
	TFT.setColor(VGA_WHITE);
	String time = timeClient.getFormattedTime();
	if (time[0] != '\0') {
		currMinutes = time.charAt(4);
		//force update if time if not fully present
		int offset = (220 - TFT.getStringWidth(time)) / 2;
		if (forceUpdate) {
			if (alignCenter) {
				TFT.print(time, offset, y);
				TFT.drawHLine(offset + TFT.getStringWidth(time)*0.1, y + 43, TFT.getStringWidth(time)*0.8);
			}
			else {
				TFT.print(time, x, y);
				TFT.drawHLine(x + TFT.getStringWidth(time)*0.1, y + 43, TFT.getStringWidth(time)*0.8);
			}
			prevMinutes = currMinutes;
		}
		// TODO test if it works properly
		if (currMinutes != prevMinutes) {
			if (alignCenter) {
				TFT.print(time, offset, y);
				TFT.drawHLine(offset + TFT.getStringWidth(time)*0.1, y + 43, TFT.getStringWidth(time)*0.8);
			}
			else {
				TFT.print(time, x, y);
				TFT.drawHLine(x + TFT.getStringWidth(time)*0.1, y + 43, TFT.getStringWidth(time)*0.8);
			}
			prevMinutes = currMinutes;
		}
		else {
			currSecond = timeClient.getSeconds().charAt(0);
			if (currSecond != prevSecond) {
				if (alignCenter) {
					TFT.print(timeClient.getSeconds(), offset + TFT.getStringWidth("55:55:"), y);
				}
				else {
					TFT.print(timeClient.getSeconds(), x + TFT.getStringWidth("55:55:"), y);
				}
				prevSecond = currSecond;
			}
			else {
				currSecond2 = timeClient.getSeconds().charAt(1);
				if (currSecond2 != prevSecond2)
				{
					if (alignCenter) {
						TFT.print(String(currSecond2), offset + TFT.getStringWidth("55:55:5"), y);
					}
					else {
						TFT.print(String(currSecond2), x + TFT.getStringWidth("55:55:5"), y);
					}
					prevSecond2 = currSecond2;
				}
			}
		//	prevSecond = currSecond;
		}
	return true;
	}
	else {
		return false;
	}
}

bool drawDate(int x, int y, bool alignCenter) {
	String date = wunderground.getDate();
	if (date[0] != '\0') {
		TFT.setColor(VGA_BLACK);
		TFT.fillRect(x, y + 50, 220, y + 70);
		TFT.setFont(DejaVuSans42);
		TFT.setColor(VGA_WHITE);
		String time = timeClient.getFormattedTime();
		int strw = TFT.getStringWidth(time);

		TFT.setFont(DejaVuSans18plus);
		int offset = (strw - TFT.getStringWidth(date)) / 2;
		if (alignCenter) {
			TFT.print(date, offset, y + 50);
		}
		else {
			TFT.print(date, x + offset, y + 50);
		}
		return true;
	}
	else {
		return false;
	}
}

bool drawForecast(int x, int y, bool alignCenter) {
	if (alignCenter) {
		TFT.setFont(DejaVuSans10);
		int offset = (220 - 132 - TFT.getStringWidth("55|55")) / 2;
		drawForecastDetails(offset, y, 0);
		drawForecastDetails(offset + 44, y, 2);
		drawForecastDetails(offset + 88, y, 4);
		drawForecastDetails(offset + 132, y, 6);
	}
	else {
		drawForecastDetails(x, y, 0);
		drawForecastDetails(x + 44, y, 2);
		drawForecastDetails(x + 88, y, 4);
		drawForecastDetails(x + 132, y, 6);
	}
	return true;
}

bool drawForecastDetails(int x, int y, int dayIndex) {
	TFT.setColor(VGA_WHITE);
	TFT.setFont(DejaVuSans10);
	String day = wunderground.getForecastTitle(dayIndex).substring(0, 3);			day.toUpperCase();
	TFT.print(day, x, y);
	TFT.print(wunderground.getForecastLowTemp(dayIndex) + "|" + wunderground.getForecastHighTemp(dayIndex), x - 5, y + 34);

	TFT.setFont(meteocons21);
	TFT.print(wunderground.getForecastIcon(dayIndex), x, y + 12);
	return true;
}

bool drawInfoOverlay(int x, int y) {										//draw overlay			230x20

	TFT.setColor(VGA_WHITE);
	TFT.setFont(Sans12);
	TFT.print("Temperature", x, y);
	TFT.drawHLine(x + TFT.getStringWidth("Temperature")*0.1, y + TFT.getFontHeight() + 2, TFT.getStringWidth("Temperature")*0.8);
	TFT.print("Pressure", x + 13, y + 50);
	TFT.drawHLine(x + 13 + TFT.getStringWidth("Pressure")*0.1, y + 50 + TFT.getFontHeight() + 2, TFT.getStringWidth("Pressure")*0.8);
	TFT.print("Altitude", x + 17, y + 100);
	TFT.drawHLine(x + 17 + TFT.getStringWidth("Altitude")*0.1, y + 100 + TFT.getFontHeight() + 2, TFT.getStringWidth("Altitude")*0.8);
	TFT.print("Humidity", x + 14, y + 150);
	TFT.drawHLine(x + 17 + TFT.getStringWidth("Humidity")*0.1, y + 150 + TFT.getFontHeight() + 2, TFT.getStringWidth("Altitude")*0.8);
	return true;
}

bool DrawCurrentInfo(int x, int y, unsigned long interval, bool alignCenter) {	
	// true if sensor is updated; false if it's not time to update
	//if not interval provided; data is always updated
	

	if (currTime - prevInfoUpdate > interval) {
		int tempWidth;
		int pressWidth;
		int altiWidth;
		int humWidth;
		if (alignCenter) {
			TFT.setFont(Sans12);
			tempWidth = TFT.getStringWidth("Temperature");
			pressWidth = TFT.getStringWidth("Pressure");
			altiWidth = TFT.getStringWidth("Altitude");
			humWidth = TFT.getStringWidth("Humidity");
		}
		
		TFT.setColor(VGA_WHITE);
		TFT.setFont(DejaVuSans18plus);

		currTemp = getTemp();
		if (prevTemp != currTemp || interval == 0) {
			if (alignCenter) {
				TFT.print(currTemp, x + (tempWidth - TFT.getStringWidth(currTemp)) / 2, y + 19);
			}
			else {
				TFT.print(currTemp, x + 12, y + 19);
			}
			prevTemp = currTemp;
		}
		
		currPressure = getPressure();										// pressure update
		if (prevPressure != currPressure || interval == 0) {
			if (alignCenter) {
				TFT.print(currPressure, x + (pressWidth - TFT.getStringWidth(currPressure)) / 2 + 13, y + 69);
			}
			else {
				TFT.print(currPressure, x + 5, y + 69);
			}
			prevPressure = currPressure;
		}
		
		currAltitude = getAltitude();										// altitude update
		if (prevAltitude != currAltitude || interval == 0) {
			if (alignCenter) {
				TFT.print(currAltitude, x + (altiWidth - TFT.getStringWidth(currAltitude)) / 2 + 17, y + 119);
			}
			else {
				TFT.print(currAltitude, x + 13, y + 119);
			}
			prevAltitude = currAltitude;
		}
		
		currHumidity = wunderground.getHumidity();							// humidity update
		if (currHumidity != prevHumidity || interval == 0) {
			if (alignCenter) {
				TFT.print(currHumidity, x + (humWidth - TFT.getStringWidth(currHumidity)) / 2 + 17, y + 169);
			}
			else {
				TFT.print(currHumidity, x + 26, y + 169);
			}
			prevHumidity = currHumidity;
		}
		prevInfoUpdate = currTime;
		return true;
	}
	else {
		return false;
	}
}

String getPressure() {
	sensors_event_t event;
	bmp.getEvent(&event);
	return String(event.pressure).substring(0,3) + " hPa";
}

String getTemp() {
	float temperature;
	bmp.getTemperature(&temperature);
	return String(temperature).substring(0,2) + "°C";
}

String getAltitude() {
	sensors_event_t event;
	bmp.getEvent(&event);
	float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
	return String(bmp.pressureToAltitude(seaLevelPressure, event.pressure)).substring(0,3) + " m" ;

}
