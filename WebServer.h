#ifndef ESPClockWebServer_h
#define ESPClockWebServer_h

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

class WebServerClass {  // This is really a namespace, just in odd Arduino-style
public:
  void begin();
  void loop();

  static void urlDecode(char *decoded, const char *encoded, size_t n);

private:
  static void _sendConnectionHeader();

  static void handleNotFound();

  static void handleBasicSetup();
  static void handleConfig();

  static void handleReset();

  static ESP8266WebServer _server;
};

extern WebServerClass WebServer;

#endif  // ESPClockWebServer_h
