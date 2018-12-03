# Arduino code voor de MiFlo

Deze code is geschreven voor een [Adafruit Feather HUZZAH ESP8266](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266) in combinatie met een [gameduino touchscreen](https://www.watterott.com/en/Gameduino-3). 

Bekijk in de [hardware](./hardware) folder van dit project de volledige hardware lijst.

## Configuratie en setup

Alle details over de Adafruit Feather HUZZAH ESP8266 staan op [de site van adafruit](Adafruit Feather HUZZAH ESP8266).

Kopiëer het bestand `settings_example.h` naar `settings.h` en vul alle parameters in.

Je hebt ook de [library van de gameduino](https://github.com/jamesbowman/gd2-lib) nodig.

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
