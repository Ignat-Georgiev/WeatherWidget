# WeatherWidget
Embedded weather station project revolving around the ESP8266 platform. Inspired by the numerous similar projects on the web. Have created a PCB for added convinience; you can easily change it with any PCB CAD software and manufacture them for ~20$.

![Imgur](http://i.imgur.com/BgfqPQ0.jpg)


Concept
--------------------------
To essentailly be a Nest-like device with features like
- Displaying current time, weather and forecast from the web
- Displaying real-time about the environment
- Control various things around the house like doors, windows, lights etc. (TODO)


Bill of Materials
-------------------------
- ESP8266
- Any screen with ILI9341 driver and 240x320 resolution (or you can adopt it to the screen you want)
- BMP085/BMP180/BMP280
- MCP23008 (Optional; nedded to physically control hardware)
- TSC2046 (Optional; needed for touch screen)



Libraries needed:
-------------------------
- [ESP8266 Libraries](https://github.com/esp8266/Arduino)
- [UTFT_DLG](https://sites.google.com/site/dlbarduino/home) and [UTFT_Ui](https://github.com/gnulabis/UTFT-ESP8266) (Optional; needed for home control UI)
- [WUndergroudn API @squix78](https://github.com/squix78/esp8266-projects/tree/master/arduino-ide/weather-station-v2)
- [Adafruit BMP085 library](https://github.com/adafruit/Adafruit-BMP085-Library)
- [Adafruit MCP23008](https://github.com/adafruit/Adafruit-MCP23008-library)

TODO:
-----------------------
- Touch screen library
- Calculate Altitude properly
- Fix various visual bugs
