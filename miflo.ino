#define SECONDS_PER_MINUTE 60

// THE LINE BELOW SHOULD OVERRIDE PUBSUBCLIENT'S BUT DOESN'T REALLY DO IT ...
#define MQTT_MAX_PACKET_SIZE 1024

#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>

#include "settings.h"

#include <ESP8266WiFi.h>
WiFiClient espClient;

#include <PubSubClient.h>
PubSubClient client(espClient);

#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;

#include <ArduinoJson.h>

#define COLOR_BEIGE 0xffe9ac
#define COLOR_GREEN 0x588751
#define COLOR_RED 0xd41942
#define COLOR_ORANGE 0xff6f25
#define COLOR_YELLOW 0xffc700
#define COLOR_BLUE 0x2b19d4
#define COLOR_BLACK 0x030302
#define COLOR_WHITE 0xffe9e6

#define GOED_AANTAL 16
char* goed[GOED_AANTAL] = { "fantastisch", "buitengewoon", "geweldig", "fenomenaal", "toverachtig", "wonderlijk", "uitstekend", "eersteklas", "opperbest", "excellent", "briljant", "grandioos", "machtig", "sensationeel", "volmaakt", "uitzonderlijk" };
int goed1, goed2;

int points = 1;

enum State { CLOCK, TIME_TIMER, FINISHED, TOILET_REMINDER, DRINK_REMINDER, KINE_REMINDER, FLO, MINNE, GOED_GEDAAN, UPCOMING, SHOW_LOG };
State state = SHOW_LOG;
long time_timer = 0;

enum JPG { AANKLEDEN_JPG, BED_OPMAKEN_JPG, BRIL_JPG, DRINKEN_JPG, KINE_JPG, LEGO_JPG, ONDERBROEK_JPG, OPRUIMEN_JPG, PYJAMA_OPRUIMEN_JPG, TANDEN_JPG, TOILET_JPG };

JPG ritual_images[ 2 ][ 4 ] = {
  { TANDEN_JPG, PYJAMA_OPRUIMEN_JPG, BED_OPMAKEN_JPG, ONDERBROEK_JPG },
  { TANDEN_JPG, AANKLEDEN_JPG, BRIL_JPG, OPRUIMEN_JPG }
};

char* ritual_texts[ 2 ][ 4 ] = { 
  { "tanden poetsen", "pyjama ophangen", "bed opmaken", "ondergoed in de was" }, 
  { "tanden poetsen", "kleren aandoen", "bril aan", "kamer opruimen" } 
};
bool ritual_done[ 4 ] = { false, false, false, false };

char* timer_job = "";

#include <vector>
std::vector< String > log_history;
void add_log( String message ) {
  Serial.println( message );
  log_history.push_back( message );
  while( log_history.size() > 16 ) {
    log_history.erase( log_history.begin() );
  }
  if( state == SHOW_LOG ) {
    GD.ClearColorRGB(COLOR_BLACK);
    GD.Clear();
    GD.ColorRGB(COLOR_BEIGE);
    show_log();
    GD.swap();
  }
}

void show_log() {
  int x = 0;
  for( std::vector< String >::iterator i = log_history.begin(); i != log_history.end(); i++ ) {
    GD.cmd_text(5, 25 + x * 14, 20, 0, string2char(*i));
    x++;
  }
}

#include <map>
std::map< int, String > cache;

void parse_command( char* json ) {
  StaticJsonBuffer<1024> jsonBuffer;
  
  JsonObject& root = jsonBuffer.parseObject(json);
  
  const char* type = root["type"];

  if( strcmp( type, "reset" ) == 0 ) {
    add_log("Running reset");
    state = CLOCK;
  } else if( strcmp( type, "events" ) == 0 ) {
    add_log("Caching events");
    cache.clear();
    for( int i=0; i < root["events"].size(); i++ ) {
      int hour = root["events"][i]["hour"];
      int minute = root["events"][i]["minute"];
      int second = root["events"][i]["second"];
      int time = hour*10000+minute*100+second;
      const char* task = root["events"][i]["task"];
      cache[ time ] = task;
    }
  } else if( strcmp( type, "log" ) == 0 ) {
    add_log("Showing log");
    state = SHOW_LOG;
  } else if( strcmp( type, "plus" ) == 0 ) {
    add_log("Running plus");
    points++;
  } else if( strcmp( type, "min" ) == 0 ) {
    add_log("Running min");
    points--;
  } else if( strcmp( type, "flo" ) == 0 ) {
    add_log("Running flo");
    ritual_done[ 0 ] = false;
    ritual_done[ 1 ] = false;
    ritual_done[ 2 ] = false;
    ritual_done[ 3 ] = false;
    state = FLO;
  } else if( strcmp( type, "minne" ) == 0 ) {
    add_log("Running minne");
    ritual_done[ 0 ] = false;
    ritual_done[ 1 ] = false;
    ritual_done[ 2 ] = false;
    ritual_done[ 3 ] = false;
    state = MINNE;
  } else if( strcmp( type, "timetimer" ) == 0 ) {
    add_log("Running timetimer");
    int minutes = root["minutes"];
    Serial.print( "minutes: " );
    Serial.println( minutes );
    const char* job = root["job"];
    strcpy(timer_job, job);
    add_log( job );
    DateTime now = rtc.now();
    time_timer = now.unixtime() + minutes * SECONDS_PER_MINUTE;
    state = TIME_TIMER;

    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  add_log("Incoming MQTT message on topic:");
  add_log(topic);
  add_log("MQTT message:");
  char json[ length ];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    json[i] = (char)payload[i];
  }
  add_log( json );
  parse_command(json);
}

void mqttOnlineCheck() {
  if(!client.connected()) {
    client.setServer(mqttServer, mqttPort);
    client.setCallback(mqttCallback);
  
    while (!client.connected()) {
      add_log("Connecting to MQTT ...");
   
      if (client.connect("AnthonysLittleClient" )) {
   
        add_log("Connected to MQTT");  
   
      } else {
   
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
   
      }
    }
    client.subscribe(mqttTopic);
  }
}

void load_jpgs()
{
  GD.BitmapHandle(AANKLEDEN_JPG);
  GD.cmd_loadimage(0, 0);
  GD.load("aanklede.jpg");
  GD.BitmapHandle(BED_OPMAKEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("bedopmak.jpg");
  GD.BitmapHandle(BRIL_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("bril.jpg");
  GD.BitmapHandle(DRINKEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("bril.jpg");
  GD.BitmapHandle(KINE_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("beweging.jpg");
  GD.BitmapHandle(LEGO_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("lego.jpg");
  GD.BitmapHandle(ONDERBROEK_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("onderbro.jpg");
  GD.BitmapHandle(OPRUIMEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("opruimen.jpg");
  GD.BitmapHandle(PYJAMA_OPRUIMEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("pyjaopru.jpg");
  GD.BitmapHandle(TANDEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("tanden.jpg");
  GD.BitmapHandle(TOILET_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("toilet.jpg");
}

void setup()
{
  Serial.begin(115200);

  Serial.println("Starting gameduino ...");
  GD.begin();
  GD.cmd_setrotate(0);
  add_log("Gameduino started");

  add_log("Loading JPGs ...");
  load_jpgs();
  add_log("JPGs loaded");

  WiFi.begin( ssid, password );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    add_log("Connecting to Wifi ...");
  }
  add_log("Connected to WiFi network:");
  add_log(WiFi.SSID());

  mqttOnlineCheck();

  add_log("Starting RTC ...");
  if (! rtc.begin()) {
    add_log("Couldn't find RTC");
    while (1);
  } else {
    add_log("RTC started");
  }

  // set clock to compile time
  // only uncomment if you want to set this date
  rtc.adjust(DateTime(__DATE__, __TIME__));

  add_log( "MiFlo has booted successfully!" );
  
  state = CLOCK;
  
}

char* string2char(String s){
    if(s.length()!=0){
        char *p = const_cast<char*>(s.c_str());
        return p;
    }
}

char* int2char(int n){
  char s[16];
  itoa(n, s, 10);
  return s;
}

String format_time( int hour, int minute, int second ) {
  return String( hour < 10 ? "0" : " " ) + hour + ( minute < 10 ? " : 0" : " : " ) + minute + ( second < 10 ? " : 0" : " : " ) + second;
}

String current_time() {
  DateTime now = rtc.now();
  return format_time( now.hour(), now.minute(), now.second() );
}

int animation_counter = 0;
void show_clock() {

  GD.Begin(POINTS);
  GD.ColorA(64);
  GD.ColorRGB(COLOR_BEIGE);
  for( int i = 0; i < points; i++ ) {
    GD.PointSize(16 * sqrt( i + 1 ) * 5 );
    GD.Vertex2ii(
       100 * ( cos(animation_counter/float(100+i*73%101)) + cos(animation_counter/float(100+i*101%73)) )+GD.w/2,
       40 * ( sin(animation_counter/float(100+i*103%89)) + cos(animation_counter/float(100+i*89%103)) )+GD.h/2
    );
  }
  animation_counter++;

  String time = current_time();
  GD.ColorRGB(COLOR_BEIGE);
  GD.ColorA(128);
  GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, string2char(time));
  GD.ColorA(255);

  GD.get_inputs();
  switch (GD.inputs.tag) {
  case 210:
    state = UPCOMING;
  break;
  }

  GD.ColorRGB(COLOR_BEIGE);
  GD.cmd_fgcolor(COLOR_BLACK);
  GD.ColorA(128);
  GD.Tag(210);
  GD.cmd_button( 20, GD.h-40, 20, 20, 20, OPT_FLAT, int2char(cache.size()) );
  GD.ColorA(255);

}

static void clock_ray(int centerx, int centery, int &x, int &y, int r, double i)
{
  uint16_t th = 0x8000 + 65536UL * ( i / 60.0 );
  GD.polar(x, y, r, th);
  x += 16 * centerx;
  y += 16 * centery;
}

void show_time_timer( double minutes, int centerx, int centery, int scale ) {
   int x, y;

  GD.Begin(POINTS);
  GD.ColorRGB(COLOR_BLACK);

  // time-timer labels
  /*const char *labels[] = {"0","55","50","45","40","35","30","25","20","15","10","5"};
  int align[12] = { OPT_CENTERX, 0, 0, 0, OPT_RIGHTX, OPT_RIGHTX, OPT_RIGHTX, 0, 0, 0, 0, 0 };
  for (int i = 0; i < 12; i++) {
    clock_ray(centerx, centery, x, y, 16 * scale, i*5);
    GD.cmd_text(x >> 4, y >> 4, 26, OPT_CENTER, labels[i]);
  }*/
  clock_ray(centerx, centery, x, y, 16 * scale * 1.1, -minutes);
  char minute_str[16];
  itoa((int)(minutes + 0.99), minute_str, 10);
  GD.cmd_text(x >> 4, y >> 4, 28, OPT_CENTER, minute_str);
  
  // clockface
  GD.LineWidth(8);
  GD.Begin(LINES);
  for (int i = 0; i < 60; i++) {
    if( i % 5 == 0 ) {
      clock_ray(centerx, centery, x, y, 16 * scale * 0.9, i);
      GD.Vertex2f(x, y);
      clock_ray(centerx, centery, x, y, 16 * scale * 0.8, i);
      GD.Vertex2f(x, y);
    } else {
      clock_ray(centerx, centery, x, y, 16 * scale * 0.9, i);
      GD.Vertex2f(x, y);
      clock_ray(centerx, centery, x, y, 16 * scale * 0.85, i);
      GD.Vertex2f(x, y);
    }
  }

  // time-timer
  if( minutes > 0 ) {
    Poly po;
    po.begin();
    po.v(centerx * 16, centery * 16);
    clock_ray(centerx, centery, x, y, 16* scale * 0.7, 60 - minutes);
    po.v(x, y);
    clock_ray(centerx, centery, x, y, 16* scale * 0.7, (int)( 60 - minutes + 1 ));
    po.v(x, y);
    GD.ColorRGB(COLOR_RED);
    po.draw();
    for (int i = 60 - minutes + 1; i < 60; i++) {
      Poly po;
      po.begin();
      po.v(centerx * 16, centery * 16);
      clock_ray(centerx, centery, x, y, 16* scale * 0.7, i + 1.1);
      po.v(x, y);
      clock_ray(centerx, centery, x, y, 16* scale * 0.7, i - 0.1);
      po.v(x, y);
      GD.ColorRGB(COLOR_RED);
      po.draw();
    }
  }
 
}

long next_alarm_bleep = 0;
void show_timer_finished() {

  DateTime now = rtc.now();

  if( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 2;
    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
  }
  
  GD.get_inputs();
  switch (GD.inputs.tag) {
  case 201:
    points++;
    goed1 = rand() % 16;
    goed2 = rand() % 16;
    state = GOED_GEDAAN;
    delay(200);
  break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(201);
  GD.cmd_button( GD.w - 90, GD.h-60, 80, 50, 27, OPT_FLAT, "OK" );

  show_time_timer( 0, GD.w / 2, GD.h / 2, 100 );

}

void statusbar() {

  GD.ColorA(128);

  // update WiFi status
  if( WiFi.status() == WL_CONNECTED ) GD.ColorRGB(COLOR_GREEN); else GD.ColorRGB(COLOR_RED);
  GD.cmd_text(5, 5, 20, 0, "Wifi");
  // update MQTT status
  client.loop();
  if( client.connected() ) GD.ColorRGB(COLOR_GREEN); else GD.ColorRGB(COLOR_RED);
  GD.cmd_text(25, 5, 20, 0, "MQTT");

  String time = current_time();
  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(GD.w - 65, 5, 20, 0, string2char(time));
  GD.ColorA(255);
}

void show_reminder(String job) {
  DateTime now = rtc.now();

  if( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 2;
    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
  }
  
  GD.get_inputs();
  switch (GD.inputs.tag) {
  case 201:
    state = CLOCK;
    points++;
  break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(GD.w / 2, GD.h / 2-40, 28, OPT_CENTER, "Niet vergeten!");
  GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, string2char(job));

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(201);
  GD.cmd_button( GD.w / 2 - 200, GD.h-60, 400, 50, 27, OPT_FLAT, "OK, ik zal het niet vergeten" );
}

void show_upcoming_events() {
  
  GD.get_inputs();
  switch (GD.inputs.tag) {
  case 211:
    state = CLOCK;
  break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(20, 30, 22, 0, "Upcoming calendar events");
  int x = 0;
  for( std::map<int, String>::iterator i = cache.begin(); i != cache.end(); i++ ) {
    GD.cmd_text(20, 60 + x * 20, 20, 0, string2char( format_time( i->first/10000, (i->first/100)%100, i->first%100 ) ) );
    GD.cmd_text(80, 60 + x * 20, 20, 0, string2char(i->second));
    x++;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(211);
  GD.cmd_button( GD.w / 2 + 100, GD.h-60, 100, 50, 27, OPT_FLAT, "OK" );
}

bool kine_done = false;
void kine_reminder(String job) {
  DateTime now = rtc.now();

  if( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 2;
    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
  }
  
  GD.get_inputs();
  switch (GD.inputs.tag) {
  case 201:
    state = CLOCK;
  break;
  case 202:
    kine_done = true;
    DateTime now = rtc.now();
    time_timer = now.unixtime() + 10 * SECONDS_PER_MINUTE;
    strcpy(timer_job, "kine");
    state = TIME_TIMER;
  break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(GD.w / 2, GD.h / 2-40, 28, OPT_CENTER, "Niet vergeten!");
  GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, string2char(job));

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_RED);
  GD.Tag(201);
  GD.cmd_button( GD.w / 2 - 200, GD.h-60, 180, 50, 27, OPT_FLAT, "Ik doe het later" );
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(202);
  GD.cmd_button( GD.w / 2 + 20, GD.h-60, 180, 50, 27, OPT_FLAT, "We doen het nu!" );
}

void show_goed_gedaan() {
  
  GD.get_inputs();
  switch (GD.inputs.tag) {
  case 201:
    state = CLOCK;
  break;
  }

  GD.ColorRGB(COLOR_BEIGE);
  GD.cmd_text(GD.w / 2, GD.h / 2-80, 28, OPT_CENTER, "Dat deed je");
  int r = rand() % GOED_AANTAL;
  GD.cmd_text(GD.w / 2, GD.h / 2-25, 31, OPT_CENTER, goed[ goed1 ]);
  GD.cmd_text(GD.w / 2, GD.h / 2+25, 31, OPT_CENTER, goed[ goed2 ]);

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_BEIGE);
  GD.Tag(201);
  GD.cmd_button( GD.w / 2 - 200, GD.h-60, 400, 50, 27, OPT_FLAT, "Graag gedaan!" );
}

void show_todo( int x, int y, int person, int job, int tag ) {
  GD.Begin(BITMAPS);
  GD.ColorRGB(COLOR_BEIGE);
  if( ritual_done[ job ] ) GD.ColorA(32); else GD.ColorA(255);
  GD.Tag(tag);
  GD.Vertex2ii(x, y, ritual_images[ person ][ job ] );
  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(x+50, y+110, 28, OPT_CENTER, ritual_texts[ person ][ job ]);
  if( ritual_done[ job ] ) {
    GD.Begin(LINE_STRIP);
    GD.ColorA(255); 
    GD.LineWidth(16 * 10);
    GD.ColorRGB(COLOR_GREEN);
    GD.Vertex2ii(x + 20, y + 50);
    GD.Vertex2ii(x + 50, y + 80);
    GD.Vertex2ii(x + 80, y + 10);
  }
  GD.ColorA( 255 );
}

void show_todos( int person ) {
  show_todo( 93, 20, person, 0, 100 );
  show_todo( 287, 20, person, 1, 101 );
  show_todo( 93, 140, person, 2, 102 );
  show_todo( 287, 140, person, 3, 103 );

  GD.get_inputs();
  if( GD.inputs.tag > 0 ) {
    ritual_done[ GD.inputs.tag - 100 ] = not ritual_done[ GD.inputs.tag - 100 ];
    delay(300);
  }

  if( ritual_done[0] && ritual_done[1] && ritual_done[2] && ritual_done[3] ) {
    points += 4;
    goed1 = rand() % GOED_AANTAL;
    goed2 = rand() % GOED_AANTAL;
    state = GOED_GEDAAN;
  }
}

void show_flo() {
  show_todos( 0 );
}

void show_minne() {
  DateTime now = rtc.now();

  if( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 30;
    GD.play( PIANO, 55 );
    delay(250);
    GD.play( PIANO, 64 );
    delay(250);
    GD.play( PIANO, 60 );
  }
  
  show_todos( 1 );
}

void run_time_timer() {

  DateTime now = rtc.now();
  double minutes = (time_timer-now.unixtime())/(double)SECONDS_PER_MINUTE;
  int current_h = now.hour();
  int current_m = now.minute();
  int current_s = now.second();

  if( strcmp( timer_job, "lego" ) == 0 ) {
    GD.Begin(BITMAPS);
    GD.ColorRGB(COLOR_BEIGE);
    GD.Vertex2ii(GD.w / 2 + GD.w / 4 - 50, GD.h / 2 - 50, LEGO_JPG );
    GD.ColorRGB(COLOR_BLACK);
    GD.cmd_text(GD.w / 2 + GD.w / 4, GD.h / 2 + 60, 28, OPT_CENTER, timer_job);
    show_time_timer( minutes, GD.w / 2 - GD.w / 4 + 50, GD.h / 2, 100 );
  } else if( strcmp( timer_job, "kine" ) == 0 ) {
    GD.Begin(BITMAPS);
    GD.ColorRGB(COLOR_BEIGE);
    GD.Vertex2ii(GD.w / 2 + GD.w / 4 - 50, GD.h / 2 - 50, KINE_JPG );
    GD.ColorRGB(COLOR_BLACK);
    GD.cmd_text(GD.w / 2 + GD.w / 4, GD.h / 2 + 60, 28, OPT_CENTER, timer_job);
    show_time_timer( minutes, GD.w / 2 - GD.w / 4 + 50, GD.h / 2, 100 );
  } else
    show_time_timer( minutes, GD.w / 2, GD.h / 2, 100 );
  if( now.unixtime() >= time_timer ) { 
    state = FINISHED; 
  }
}

void loop() {

  // clear screen
  uint32_t bgcolor = COLOR_BEIGE;
  switch (state) {
    case CLOCK:
    case SHOW_LOG:
      bgcolor = COLOR_BLACK;
    break;
    case TOILET_REMINDER:
    case DRINK_REMINDER:
    case KINE_REMINDER:
      bgcolor = COLOR_YELLOW;
    break;
    case GOED_GEDAAN:
      bgcolor = COLOR_GREEN;
    break;
  }
  GD.ClearColorRGB(bgcolor);
  GD.Clear();

  statusbar();

  DateTime now = rtc.now();
  double minutes = (time_timer-now.unixtime())/(double)SECONDS_PER_MINUTE;
  int current_h = now.hour();
  int current_m = now.minute();
  int current_s = now.second();
  
  // dependent on state, show contents
  switch (state) {
    case FINISHED:
      show_timer_finished();
    break;
    case TOILET_REMINDER:
      show_reminder( "Moet je naar toilet?" );
    break;
    case DRINK_REMINDER:
      show_reminder( "Drink je genoeg?" );
    break;
    case KINE_REMINDER:
      kine_reminder( "Is je kine gedaan?" );
    break;
    case TIME_TIMER:
      run_time_timer();
    break;
    case UPCOMING:
      show_upcoming_events();
    break;
    case SHOW_LOG:
      GD.ColorRGB(COLOR_BEIGE);
      show_log();
    break;
    case CLOCK:
      int cache_time;
      cache_time = current_h * 10000 + current_m * 100 + current_s;
      if( cache.count( cache_time ) ) {
        parse_command(string2char(cache[cache_time]));
        cache.erase(cache_time);
      }
      show_clock();
      if( ( current_m == 13 ) && ( current_s == 37 ) ) state = TOILET_REMINDER;
      if( ( current_m == 37 ) && ( current_s == 13 ) ) state = DRINK_REMINDER;
      if( !kine_done && ( current_m == 15 ) && ( current_s == 00 ) ) state = KINE_REMINDER;
      if( !kine_done && ( current_m == 40 ) && ( current_s == 00 ) ) state = KINE_REMINDER;
    break;
    case FLO:
      show_flo();
    break;
    case MINNE:
      show_minne();
    break;
    case GOED_GEDAAN:
      show_goed_gedaan();
    break;
  }

  // bring the contents to the front
  GD.swap();

  rand(); // keep generating random numbers to mess with the seed

}
