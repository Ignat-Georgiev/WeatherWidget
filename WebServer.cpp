#include <Arduino.h>
#include <ESP8266mDNS.h>
//#include <ESP8266HTTPClient.h>

#include <errno.h>
#include <stdio.h>

#include "Settings.h"
#include "WebServer.h"
#include <ESP8266HTTPUpdateServer.h>

ESP8266WebServer WebServerClass::_server;

const char TEXT_PLAIN[] PROGMEM = "text/plain";
const char TEXT_HTML[] PROGMEM = "text/html";

WebServerClass WebServer;
ESP8266HTTPUpdateServer httpUpdater;
WiFiServer server(80);


static inline uint8_t hexvalue(char c) {
	c = toupper(c);
	if (c >= 'A') {
		return c - 'A' + 10;
	}
	else {
		return c - '0';
	}
}

void WebServerClass::urlDecode(char *decoded, const char *encoded, size_t n) {
	const char *p = encoded;
	char *q = decoded;
	while (*p != '\0') {
		// Check length limit first, make sure it's zero-terminated
		if ((q - decoded >= n) && (n > 0)) {
			q[-1] = '\0';
			return;
		}
		if (*p == '+') {
			*q = ' ';
		}
		else if (*p == '%' && isxdigit(p[1]) && isxdigit(p[2])) {
			*q = (char)(hexvalue(p[1]) * 16 + hexvalue(p[2]));
			p += 2;  // Additional chars to skip
		}
		else {
			*q = *p;
		}
		++p; ++q;
	}
	*q = '\0';
}

#include "basicsetup_html.h"  // Ditto


void WebServerClass::_sendConnectionHeader() {
	_server.sendHeader("Connection", "close");
}

void WebServerClass::handleNotFound() {
	// TODO Make it pretty
	_server.send_P(404, TEXT_PLAIN, PSTR("Invalid webpage"));
}

void WebServerClass::handleBasicSetup() {
	_server.send_P(200, TEXT_HTML, (const char*)basicsetup_html);
	Serial.println("Basic server set");
}
void WebServerClass::handleReset() {
	_server.send_P(200, TEXT_HTML, "Resetting Device");
	Serial.println("Resetting Device");
	ESP.restart();
}

void WebServerClass::handleConfig() {
	Serial.println("Handling netconfig");
	if (!_server.hasArg("ssid") || !_server.hasArg("password")) {
		_server.send_P(400, TEXT_PLAIN, PSTR("Missing fields"));
		return;
	}

	String ssid = _server.arg("ssid"),
		password = _server.arg("password"),
		hostname = _server.arg("hostname"),
		country = _server.arg("country"),
		city = _server.arg("city"),
		apikey = _server.arg("apikey"),
		alarm1time = _server.arg("alarm1time"),
		alarm2time = _server.arg("alarm2time"),
		alarm3time = _server.arg("alarm3time"),
		days1 = _server.arg("day1"),
		days2 = _server.arg("day2"),
		days3 = _server.arg("day3");




	//check if values are empty and save them to settings
	if (ssid.length() != 0) {
		char dec_ssid[32];
		urlDecode(dec_ssid, ssid.c_str(), sizeof(dec_ssid));
		strcpy(Settings.ssid, dec_ssid);
	}

	if (password.length() != 0) {
		char dec_password[32];
		urlDecode(dec_password, password.c_str(), sizeof(dec_password));
		strcpy(Settings.password, dec_password);
	}

	if (hostname.length() != 0) {
		char dec_hostname[32];
		urlDecode(dec_hostname, hostname.c_str(), sizeof(dec_hostname));
		strcpy(Settings.hostname, dec_hostname);
	}

	if (_server.hasArg("utcOffset")) {
		int utcOffset = (_server.arg("utcOffset")).toInt();
		if (utcOffset >= -12 && utcOffset <= 12) {
			Settings.utcOffset = utcOffset;
		}
	}

	if (country.length() != 0) {
		char dec_country[32];
		urlDecode(dec_country, country.c_str(), sizeof(dec_country));
		strcpy(Settings.country, dec_country);
	}

	if (city.length() != 0) {
		char dec_city[32];
		urlDecode(dec_city, city.c_str(), sizeof(dec_city));
		strcpy(Settings.city, dec_city);
	}

	if (apikey.length() != 0) {
		char dec_apikey[32];
		urlDecode(dec_apikey, apikey.c_str(), sizeof(dec_apikey));
		strcpy(Settings.apikey, dec_apikey);
	}

	if (alarm1time.length() != 0) {
		char dec_alarm1time[32];
		urlDecode(dec_alarm1time, alarm1time.c_str(), sizeof(dec_alarm1time));
		strcpy(Settings.alarm1time, dec_alarm1time);
	}

	if (alarm2time.length() != 0) {
		char dec_alarm2time[32];
		urlDecode(dec_alarm2time, alarm2time.c_str(), sizeof(dec_alarm2time));
		strcpy(Settings.alarm2time, dec_alarm2time);
	}

	if (alarm3time.length() != 0) {
		char dec_alarm3time[32];
		urlDecode(dec_alarm3time, alarm3time.c_str(), sizeof(dec_alarm3time));
		strcpy(Settings.alarm3time, dec_alarm3time);
	}

	if (days1.length() != 0) {
		char dec_days1[32];
		urlDecode(dec_days1, days1.c_str(), sizeof(dec_days1));
		strcpy(Settings.days1, dec_days1);
	}

	if (days2.length() != 0) {
		char dec_days2[32];
		urlDecode(dec_days2, days2.c_str(), sizeof(dec_days2));
		strcpy(Settings.days2, dec_days2);
	}

	if (days3.length() != 0) {
		char dec_days3[32];
		urlDecode(dec_days3, days3.c_str(), sizeof(dec_days3));
		strcpy(Settings.days3, dec_days3);
	}

	Settings.save();
	_server.send_P(200, TEXT_PLAIN, PSTR("Settings stored. Restarting Device."));
	ESP.restart();
}


void WebServerClass::begin() {
	// Start mDNS responder and register web interface
	httpUpdater.setup(&_server);
	_server.on("/", HTTP_GET, handleBasicSetup);
	_server.on("/config", HTTP_POST, handleConfig);
	_server.on("/reset", HTTP_GET, handleReset);
	_server.onNotFound(handleNotFound);

	_server.begin();
	//_server.handleClient();

	if (WiFi.status() == WL_CONNECTED && MDNS.begin(Settings.hostname)) {
		MDNS.addService("http", "tcp", 80);
	}
}

void WebServerClass::loop() {
	_server.handleClient();
	//HTTPClient http;
	//int httpCode = http.GET();
}

