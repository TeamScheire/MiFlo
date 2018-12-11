# Arduino code voor de MiFlo

Deze code is geschreven voor een [Adafruit Feather HUZZAH ESP8266](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266) in combinatie met een [gameduino touchscreen](https://www.watterott.com/en/Gameduino-3). 

Bekijk in de [hardware](../hardware) folder van dit project de volledige hardware lijst.

## Configuratie en setup

Alle details over de Adafruit Feather HUZZAH ESP8266 staan op [de site van adafruit](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/using-arduino-ide).

Kopiëer het bestand `settings_example.h` naar `settings.h` en vul alle parameters in.

Je hebt ook de volgende libraries nodig:

* [gameduino](https://github.com/jamesbowman/gd2-lib)
* arduino EEPROM library
* [RTC](https://github.com/adafruit/RTClib) library.
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

In de gameduino library moet je enkele aanpassingen doen in de GD2.file:

*  Voeg
	
	```
	#define D8  0
	#define D9  16
	```
	toe net voor de lijn `#define CS D8`.
* Zoek naar `#define L2` en `#define L4` en comment deze lijnen.


De arduino code gaat ervanuit dat je de geconverteerde [assets](./assets) mee op de sd kaart gezet hebt.

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
