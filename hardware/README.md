# Hardware voor de MiFlo

De hardware in de MiFlo is opgebouwd op eenvoudig verkrijgbare modules:

* In het originele project wordt een [Adafruit Feather HUZZAH](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266) gebruikt als de "Arduino". Deze bevat een ESP8266 chip waarmee we ook WiFi kunnen gebruiken. Zijn opvolger, de [Adafruit Feather HUZZAH32](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather) kan echter ook gebruikt worden. Deze is een pak krachtiger en bovendien een beetje zuiniger in verbruik.
* De [Gameduino 3](http://excamera.com/sphinx/gameduino3/) is een touchscreen dat je makkelijk op een Arduino kan monteren en aansturen. Het kan ook geluiden of filmpjes spelen van een SD kaart. Bijvoorbeeld [hier te koop](https://www.watterott.com/de/Gameduino-3).
* Met een [RTC breakout board](https://www.adafruit.com/product/3295) kunnen we de tijd bijhouden als de MiFlo geen stroom heeft.
* Een [geluidsversterker](https://www.adafruit.com/product/1752) zorgt ervoor dat de audio krachtig genoeg uit de speakers komt.
* We bouwden alles in een [SoundLogic draagbare speaker](https://www.action.com/nl-be/p/soundlogic-draagbare-speaker/) van den Action.


We ontwierpen zelf een PCB die deze componenten allemaal bij elkaar brengt, deze vind je (als `.brd` en `.sch` files voor Autodesk EAGLE) in deze repository. Met de PCB kan je zowel een klassieke Arduino of een Adafruit Feather verbinden met het Gameduino touchscreen.

Om de MiFlo mooi te bevestigen printten we kittenoortjes voor Minne en een Lego-helm voor Flo. Samen met 2 clips kan je de MiFlo zo bevestigen in de draagbare speakerkast. Je vindt deze STL files om zelf te printen als `kat.stl`, `legohelm.stl` en `clip.stl`.
