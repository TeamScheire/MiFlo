# Arduino code voor de MiFlo

Deze code is geschreven voor een [Adafruit Feather HUZZAH ESP8266](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266) of [Adafruit Feather HUZZAH32](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather) in combinatie met een [gameduino 3 touchscreen](https://www.watterott.com/en/Gameduino-3). 

Bekijk in de [hardware](../hardware) folder van dit project de volledige hardware lijst.

## Configuratie en setup

Alle details over de Adafruit Feather [HUZZAH ESP8266](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/overview) of [HUZZAH32 ESP32](https://learn.adafruit.com/adafruit-feather-huzzah32-esp32/overview) staan op [de site van adafruit](https://learn.adafruit.com/search?q=HUZZAH).

Kopiëer het bestand `settings_example.h` naar `settings.h` en vul alle parameters in.

Je hebt ook de volgende libraries nodig:

* [gd2-lib](https://github.com/jamesbowman/gd2-lib) Gameduino 2/3 library
* arduino EEPROM library
* [RTClib](https://github.com/adafruit/RTClib) RTC library.
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) JSON library
* [PubSubClient](https://github.com/knolleary/pubsubclient) MQTT client library

In de gameduino 2/3 library moet je enkele aanpassingen doen zodat we de correcte pins van de HUZZAH of HUZZAH32 aanspreken:

* Voor de ESP8266 chip: Voer in het bestand `transports/wiring.h`
	```
	#define D8  0
	#define D9  16
	```
	toe net voor de lijn `#define CS D8`.
* Voor de ESP32 chip: 
  * Voer in het bestand `transports/wiring.h`
	```
	#define D8  15
	#define D9  32 
	```
	toe net na de lijn `#elif  defined(ESP32)` en wijzig de lijn 
	```
	#define CS 12
	```
	in 
	```
	#define CS 15
	```
  * Wijzig in het bestand `GD2.h`:
  	```
	#elif defined(ESP32)
	#define SD_PIN        13
	```
	in
	```
	#elif defined(ESP32)
	#define SD_PIN        32
	```
* (dit lijkt niet nodig: Zoek in bestand `GD2.h` naar `#define L2` en `#define L4` en comment deze lijnen uit.)


De arduino code gaat ervanuit dat je de geconverteerde [assets](../assets) mee op de sd kaart gezet hebt.

In de code wordt bv de header-file `m_cool.h` geladen.

```
#include "m_cool.h"
```

Daarnaast worden ook alle pictogrammen geladen:

```
void load_jpgs() 
{
	GD.BitmapHandle(BRIL_JPG);
	GD.cmd_loadimage(ASSETS_END, 0);
	GD.load("BRIL.jpg");

	...
	
}
```
