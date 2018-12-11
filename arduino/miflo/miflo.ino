#define SECONDS_PER_MINUTE 60

// THE LINE BELOW SHOULD OVERRIDE PUBSUBCLIENT'S BUT DOESN'T REALLY DO IT ...
#define MQTT_MAX_PACKET_SIZE 1024

#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>

#include "m_cool.h"

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};



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

enum State { CLOCK_STATE, TIME_TIMER_STATE, FINISHED_STATE, REMINDER_STATE, ALARM_STATE, TODO_STATE, GOED_GEDAAN_STATE, UPCOMING_STATE, SHOW_LOG_STATE };
State state = SHOW_LOG_STATE;
long time_timer = 0;

enum JPG {
  BRIL_JPG,
  DOUCHEN_JPG,
  DRINKEN_JPG,
  HAAR_JPG,
  HUISWERK_JPG,
  KAKA_JPG,
  KINE_JPG,
  KLUSSEN_JPG,
  LEGOPRUM_JPG,
  LEZEN_JPG,
  NAGELS_JPG,
  NOPANIC_JPG,
  PILLEN_JPG,
  PIPIKAKA_JPG,
  POMPEN_JPG,
  PYJAMA_JPG,
  TANDEN_JPG,
  UITMEST_JPG,
  WASMAND_JPG,
  UNKNOWN_JPG
};

int todo_jpgs[ 4 ] = { TANDEN_JPG, PYJAMA_JPG, UITMEST_JPG, WASMAND_JPG };
String todo_texts[ 4 ] = { "tanden poetsen", "pyjama ophangen", "bed uitmesten", "ondergoed in de was" };
bool todo_done[ 4 ] = { false, false, false, false };

char current_job_string[200] = "";

char* string2char(String s) {
  if (s.length() != 0) {
    char *p = const_cast<char*>(s.c_str());
    return p;
  }
}

char* int2char(int n) {
  char s[16];
  itoa(n, s, 10);
  return s;
}

#include <vector>
std::vector< String > log_history;
void add_log( String message ) {
  Serial.println( message );
  log_history.push_back( message );
  while ( log_history.size() > 16 ) {
    log_history.erase( log_history.begin() );
  }
  if ( state == SHOW_LOG_STATE ) {
    GD.ClearColorRGB(COLOR_BLACK);
    GD.Clear();
    GD.ColorRGB(COLOR_BEIGE);
    show_log();
    GD.swap();
  }
}

void show_log() {
  int x = 0;
  for ( std::vector< String >::iterator i = log_history.begin(); i != log_history.end(); i++ ) {
    GD.cmd_text(5, 25 + x * 14, 20, 0, string2char(*i));
    x++;
  }
}

void playmelody() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(15, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.10;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(15);
  }
}

//int samplebases[26] = {F_DDUIM, F_WAUW, F_BOL, F_UITST, F_GO, F_KOMAAN, G_COOL, G_FORMID, G_MACHTI, G_STIEVA, G_WOEW, G_DDUIM2, G_GOED, G_SUPER, G_WOOP, G_ALLEZ, G_DEMAX, G_GOEDZO, G_PRIMA, G_TSJING, G_ZOGOED, G_BOL, G_FANTAS, G_HOPLA, G_SCOOL, G_WIII};
//int samplefreqs[26] = {F_DDUIM_FREQ, F_WAUW_FREQ, F_BOL_FREQ, F_UITST_FREQ, F_GO_FREQ, F_KOMAAN_FREQ, G_COOL_FREQ, G_FORMID_FREQ, G_MACHTI_FREQ, G_STIEVA_FREQ, G_WOEW_FREQ, G_DDUIM2_FREQ, G_GOED_FREQ, G_SUPER_FREQ, G_WOOP_FREQ, G_ALLEZ_FREQ, G_DEMAX_FREQ, G_GOEDZO_FREQ, G_PRIMA_FREQ, G_TSJING_FREQ, G_ZOGOED_FREQ, G_BOL_FREQ, G_FANTAS_FREQ, G_HOPLA_FREQ, G_SCOOL_FREQ, G_WIII_FREQ};
//int samplelengths[26] = {F_DDUIM_LENGTH, F_WAUW_LENGTH, F_BOL_LENGTH, F_UITST_LENGTH, F_GO_LENGTH, F_KOMAAN_LENGTH, G_COOL_LENGTH, G_FORMID_LENGTH, G_MACHTI_LENGTH, G_STIEVA_LENGTH, G_WOEW_LENGTH, G_DDUIM2_LENGTH, G_GOED_LENGTH, G_SUPER_LENGTH, G_WOOP_LENGTH, G_ALLEZ_LENGTH, G_DEMAX_LENGTH, G_GOEDZO_LENGTH, G_PRIMA_LENGTH, G_TSJING_LENGTH, G_ZOGOED_LENGTH, G_BOL_LENGTH, G_FANTAS_LENGTH, G_HOPLA_LENGTH, G_SCOOL_LENGTH, G_WIII_LENGTH};
int samplebases[20] = {G_COOL, G_FORMID, G_MACHTI, G_STIEVA, G_WOEW, G_DDUIM2, G_GOED, G_SUPER, G_WOOP, G_ALLEZ, G_DEMAX, G_GOEDZO, G_PRIMA, G_TSJING, G_ZOGOED, G_BOL, G_FANTAS, G_HOPLA, G_SCOOL, G_WIII};
int samplefreqs[20] = {G_COOL_FREQ, G_FORMID_FREQ, G_MACHTI_FREQ, G_STIEVA_FREQ, G_WOEW_FREQ, G_DDUIM2_FREQ, G_GOED_FREQ, G_SUPER_FREQ, G_WOOP_FREQ, G_ALLEZ_FREQ, G_DEMAX_FREQ, G_GOEDZO_FREQ, G_PRIMA_FREQ, G_TSJING_FREQ, G_ZOGOED_FREQ, G_BOL_FREQ, G_FANTAS_FREQ, G_HOPLA_FREQ, G_SCOOL_FREQ, G_WIII_FREQ};
int samplelengths[20] = {G_COOL_LENGTH, G_FORMID_LENGTH, G_MACHTI_LENGTH, G_STIEVA_LENGTH, G_WOEW_LENGTH, G_DDUIM2_LENGTH, G_GOED_LENGTH, G_SUPER_LENGTH, G_WOOP_LENGTH, G_ALLEZ_LENGTH, G_DEMAX_LENGTH, G_GOEDZO_LENGTH, G_PRIMA_LENGTH, G_TSJING_LENGTH, G_ZOGOED_LENGTH, G_BOL_LENGTH, G_FANTAS_LENGTH, G_HOPLA_LENGTH, G_SCOOL_LENGTH, G_WIII_LENGTH};

void sample() {
  digitalWrite(15, HIGH);
  GD.play(UNMUTE);
  uint32_t base, len, freq;
  int i = rand() % 20;
  base = samplebases[i];
  len = samplelengths[i];
  freq = samplefreqs[i];
  GD.sample(base, len, freq, ADPCM_SAMPLES);
  delay(2000 * len / freq);
  GD.play(MUTE);
  digitalWrite(15, LOW);
}

void jingle( int n = 0 ) {
  digitalWrite(15, HIGH);
  if ( n == 0 ) {
    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
    delay(500);
    GD.play( MUTE );
  } else if ( n == 1 ) {
    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
    delay(250);
    GD.play( MUSICBOX, 60 );
    delay(500);
    GD.play( MUTE );
  } else if ( n == 2 ) {
    GD.play( PIANO, 55 );
    delay(250);
    GD.play( PIANO, 64 );
    delay(250);
    GD.play( PIANO, 60 );
    delay(500);
    GD.play( MUTE );
  }
  digitalWrite(15, LOW);
}

#include <map>
std::map< int, String > cache;

void parse_command( char* json ) {
  StaticJsonBuffer<1024> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(json);

  const char* type = root["type"];

  if ( strcmp( type, "reset" ) == 0 ) {
    add_log("Running reset");
    state = CLOCK_STATE;
  } else if ( strcmp( type, "audio" ) == 0 ) {
    add_log("Playing audio");
    sample();
  } else if ( strcmp( type, "settime" ) == 0 ) {
    add_log("Setting the clock");
    int hour = root["hour"];
    int minute = root["minute"];
    String timestring = format_time( hour, minute, 0 );
    rtc.adjust(DateTime(__DATE__, string2char(timestring)));
  } else if ( strcmp( type, "events" ) == 0 ) {
    add_log("Caching events");
    cache.clear();
    for ( int i = 0; i < root["events"].size(); i++ ) {
      int hour = root["events"][i]["hour"];
      int minute = root["events"][i]["minute"];
      int second = root["events"][i]["second"];
      int time = hour * 10000 + minute * 100 + second;
      const char* task = root["events"][i]["task"];
      cache[ time ] = task;
    }
  } else if ( strcmp( type, "log" ) == 0 ) {
    add_log("Showing log");
    state = SHOW_LOG_STATE;
  } else if ( strcmp( type, "plus" ) == 0 ) {
    add_log("Running plus");
    points++;
    sample();
  } else if ( strcmp( type, "min" ) == 0 ) {
    add_log("Running min");
    points--;
  } else if ( strcmp( type, "todo" ) == 0 ) {
    add_log("Running todo");
    todo_texts[ 0 ] = (const char*)root["job"][0];
    todo_jpgs[ 0 ] = job2jpg( todo_texts[ 0 ] );
    todo_texts[ 1 ] = (const char*)root["job"][1];
    todo_jpgs[ 1 ] = job2jpg( todo_texts[ 1 ] );
    todo_texts[ 2 ] = (const char*)root["job"][2];
    todo_jpgs[ 2 ] = job2jpg( todo_texts[ 2 ] );
    todo_texts[ 3 ] = (const char*)root["job"][3];
    todo_jpgs[ 3 ] = job2jpg( todo_texts[ 3 ] );
    todo_done[ 0 ] = ( todo_texts[ 0 ] == "" );
    todo_done[ 1 ] = ( todo_texts[ 1 ] == "" );
    todo_done[ 2 ] = ( todo_texts[ 2 ] == "" );
    todo_done[ 3 ] = ( todo_texts[ 3 ] == "" );
    state = TODO_STATE;
  } else if ( strcmp( type, "timetimer" ) == 0 ) {
    add_log("Running timetimer");
    int minutes = root["minutes"];
    Serial.print( "minutes: " );
    Serial.println( minutes );
    const char* job = root["job"];
    strcpy(current_job_string, job);
    add_log( job );
    DateTime now = rtc.now();
    time_timer = now.unixtime() + minutes * SECONDS_PER_MINUTE;
    state = TIME_TIMER_STATE;
    jingle(1);
  } else if ( strcmp( type, "reminder" ) == 0 ) {
    add_log("Running reminder");
    const char* job = root["message"];
    strcpy(current_job_string, job);
    state = REMINDER_STATE;
  } else if ( strcmp( type, "alarm" ) == 0 ) {
    add_log("Running alarm");
    const char* job = root["message"];
    strcpy(current_job_string, job);
    state = ALARM_STATE;
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

long lastMQTTReconnectAttempt = 0;
void mqttOnlineCheck() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastMQTTReconnectAttempt > 5000) {
      lastMQTTReconnectAttempt = now;
      client.setServer(mqttServer, mqttPort);
      client.setCallback(mqttCallback);

      add_log("Connecting to MQTT ...");

      if (client.connect(mqttClient)) {

        add_log("Connected to MQTT");
        client.subscribe(mqttTopic);

      } else {

        add_log("MQTT connect failed");
        // add_log(client.state());

      }
    }
  }
}

void load_jpgs()
{

  GD.BitmapHandle(BRIL_JPG);
  GD.cmd_loadimage(ASSETS_END, 0);
  GD.load("BRIL.jpg");
  GD.BitmapHandle(DOUCHEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("DOUCHEN.jpg");
  GD.BitmapHandle(DRINKEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("DRINKEN.jpg");
  GD.BitmapHandle(HAAR_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("HAAR.jpg");
  GD.BitmapHandle(HUISWERK_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("HUISWERK.jpg");
  GD.BitmapHandle(KAKA_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("KAKA.jpg");
  GD.BitmapHandle(KINE_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("KINE.jpg");
  GD.BitmapHandle(KLUSSEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("KLUSSEN.jpg");
  GD.BitmapHandle(LEGOPRUM_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("LEGOPRUM.jpg");
  GD.BitmapHandle(LEZEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("LEZEN.jpg");
  GD.BitmapHandle(NAGELS_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("NAGELS.jpg");
  GD.BitmapHandle(NOPANIC_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("NOPANIC.jpg");
  GD.BitmapHandle(PILLEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("PILLEN.jpg");
  GD.BitmapHandle(PIPIKAKA_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("PIPIKAKA.jpg");
  GD.BitmapHandle(POMPEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("POMPEN.jpg");
  GD.BitmapHandle(PYJAMA_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("PYJAMA.jpg");
  GD.BitmapHandle(TANDEN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("TANDEN.jpg");
  GD.BitmapHandle(UITMEST_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("UITMEST.jpg");
  GD.BitmapHandle(WASMAND_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("WASMAND.jpg");
  GD.BitmapHandle(UNKNOWN_JPG);
  GD.cmd_loadimage(-1, 0);
  GD.load("UNKNOWN.jpg");
}

void setup() {
  Serial.begin(115200);

  pinMode( 15, OUTPUT );

  Serial.println("Starting gameduino ...");
  GD.begin(0);
  GD.play( MUTE );
  GD.cmd_setrotate(0);
  GD.cmd_regwrite(REG_VOL_PB, 255);
  GD.cmd_regwrite(REG_VOL_SOUND, 255);

  add_log("Gameduino started");

  add_log("Loading sounds ...");
  LOAD_ASSETS();
  add_log("Sounds loaded");

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
  // rtc.adjust(DateTime(__DATE__, __TIME__));

  add_log( "MiFlo has booted successfully!" );

  state = CLOCK_STATE;

}

String format_time( int hour, int minute, int second ) {
  return String( hour < 10 ? "0" : "" ) + hour + ( minute < 10 ? ":0" : ":" ) + minute + ( second < 10 ? ":0" : ":" ) + second;
}

String format_time_space( int hour, int minute, int second ) {
  return String( hour < 10 ? "0" : "" ) + hour + ( minute < 10 ? " : 0" : " : " ) + minute + ( second < 10 ? " : 0" : " : " ) + second;
}

String current_time() {
  DateTime now = rtc.now();
  return format_time_space  ( now.hour(), now.minute(), now.second() );
}

int animation_counter = 0;
void show_clock() {

  GD.Begin(POINTS);
  GD.ColorA(64);
  GD.ColorRGB(COLOR_BEIGE);
  for ( int i = 0; i < points; i++ ) {
    GD.PointSize(16 * sqrt( i + 1 ) * 5 );
    GD.Vertex2ii(
      100 * ( cos(animation_counter / float(100 + i * 73 % 101)) + cos(animation_counter / float(100 + i * 101 % 73)) ) + GD.w / 2,
      40 * ( sin(animation_counter / float(100 + i * 103 % 89)) + cos(animation_counter / float(100 + i * 89 % 103)) ) + GD.h / 2
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
      state = UPCOMING_STATE;
      break;
  }

  if ( cache.size() > 0 ) {
    GD.ColorRGB(COLOR_BEIGE);
    GD.cmd_fgcolor(COLOR_BLACK);
    GD.ColorA(128);
    GD.Tag(210);
    GD.cmd_button( 20, GD.h - 40, 20, 20, 20, OPT_FLAT, int2char(cache.size()) );
    GD.ColorA(255);
  }

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
    if ( i % 5 == 0 ) {
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
  if ( minutes > 0 ) {
    Poly po;
    po.begin();
    po.v(centerx * 16, centery * 16);
    clock_ray(centerx, centery, x, y, 16 * scale * 0.7, 60 - minutes);
    po.v(x, y);
    clock_ray(centerx, centery, x, y, 16 * scale * 0.7, (int)( 60 - minutes + 1 ));
    po.v(x, y);
    GD.ColorRGB(COLOR_RED);
    po.draw();
    for (int i = 60 - minutes + 1; i < 60; i++) {
      Poly po;
      po.begin();
      po.v(centerx * 16, centery * 16);
      clock_ray(centerx, centery, x, y, 16 * scale * 0.7, i + 1.1);
      po.v(x, y);
      clock_ray(centerx, centery, x, y, 16 * scale * 0.7, i - 0.1);
      po.v(x, y);
      GD.ColorRGB(COLOR_RED);
      po.draw();
    }
  }

}

int job2jpg( String job ) {
  if ( job == "bril" )
    return BRIL_JPG;
  else if ( job == "douchen" )
    return DOUCHEN_JPG;
  else if ( job == "drinken" )
    return DRINKEN_JPG;
  else if ( job == "haar" )
    return HAAR_JPG;
  else if ( job == "huiswerk" )
    return HUISWERK_JPG;
  else if ( job == "kaka" )
    return KAKA_JPG;
  else if ( job == "kine" )
    return KINE_JPG;
  else if ( job == "klussen" )
    return KLUSSEN_JPG;
  else if ( job == "opruimen" )
    return LEGOPRUM_JPG;
  else if ( job == "lezen" )
    return LEZEN_JPG;
  else if ( job == "nagels" )
    return NAGELS_JPG;
  else if ( job == "nopanic" )
    return NOPANIC_JPG;
  else if ( job == "pillen" )
    return PILLEN_JPG;
  else if ( job == "pipikaka" )
    return PIPIKAKA_JPG;
  else if ( job == "pompen" )
    return POMPEN_JPG;
  else if ( job == "pyjama" )
    return PYJAMA_JPG;
  else if ( job == "tanden" )
    return TANDEN_JPG;
  else if ( job == "uitmesten" )
    return UITMEST_JPG;
  else if ( job == "wasmand" )
    return WASMAND_JPG;
  return -1;
}

void display_time_timer( double minutes ) {
  int jpg = job2jpg( current_job_string );
  if ( jpg != -1 ) {
    GD.Begin(BITMAPS);
    GD.ColorRGB(COLOR_BEIGE);
    GD.Vertex2ii(GD.w / 2 + GD.w / 4 - 50, GD.h / 2 - 50, jpg );
    GD.ColorRGB(COLOR_BLACK);
    GD.cmd_text(GD.w / 2 + GD.w / 4, GD.h / 2 + 60, 28, OPT_CENTER, current_job_string);
    show_time_timer( minutes, GD.w / 2 - GD.w / 4 + 50, GD.h / 2, 100 );
  } else {
    GD.cmd_text(GD.w / 2, GD.h - 20, 28, OPT_CENTER, current_job_string);
    show_time_timer( minutes, GD.w / 2 - 10, GD.h / 2, 100 );
  }
}

void run_time_timer() {

  DateTime now = rtc.now();
  double minutes = (time_timer - now.unixtime()) / (double)SECONDS_PER_MINUTE;
  display_time_timer( minutes );
  if ( now.unixtime() >= time_timer ) {
    state = FINISHED_STATE;
  }
}

long next_alarm_bleep = 0;
void show_timer_finished() {

  DateTime now = rtc.now();

  if ( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 2;
    jingle();
  }

  GD.get_inputs();
  switch (GD.inputs.tag) {
    case 201:
      strcpy(current_job_string, "");
      points++;
      goed1 = rand() % 16;
      goed2 = rand() % 16;
      state = GOED_GEDAAN_STATE;
      delay(200);
      break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(201);
  GD.cmd_button( GD.w - 90, GD.h - 60, 80, 50, 27, OPT_FLAT, "OK" );

  display_time_timer( 0 );
  //  show_time_timer( 0, GD.w / 2, GD.h / 2, 100 );

}

void statusbar( int hour, int minute, int second ) {

  GD.ColorA(128);

  // update WiFi status
  //if ( WiFi.status() == WL_CONNECTED ) GD.ColorRGB(COLOR_GREEN); else GD.ColorRGB(COLOR_RED);
  //GD.cmd_text(5, 5, 20, 0, "Wifi");
  // update MQTT status
  client.loop();
  if ( client.connected() ) {
    GD.ColorRGB(COLOR_GREEN); 
    GD.cmd_text(5, 5, 20, 0, "Online");
  } else {
    GD.ColorRGB(COLOR_RED);
    GD.cmd_text(5, 5, 20, 0, "Offline");
  }
    
  GD.ColorRGB(COLOR_BEIGE);
  GD.cmd_text(GD.w / 2, 12, 20, OPT_CENTER, person);

  String time = format_time( hour, minute, second );
  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(GD.w - 65, 5, 20, 0, string2char(time));
  GD.ColorA(255);
}

bool play_sample_next = false;
bool play_sample_next_next = false; // dirty, dirty hack but I must move fast

void show_reminder(String job) {
  DateTime now = rtc.now();

  if ( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 2;
    jingle();
  }

  GD.get_inputs();
  switch (GD.inputs.tag) {
    case 201:
      play_sample_next = true;
      state = CLOCK_STATE;
      points++;
      break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(GD.w / 2, GD.h / 2 - 40, 28, OPT_CENTER, "Niet vergeten!");
  GD.cmd_text(GD.w / 2, GD.h / 2, 29, OPT_CENTER, string2char(job));

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(201);
  GD.cmd_button( GD.w / 2 - 200, GD.h - 60, 400, 50, 27, OPT_FLAT, "OK, ik zal het niet vergeten" );
}

void show_alarm(String job) {
  DateTime now = rtc.now();

  if ( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 2;
    jingle();
  }

  GD.get_inputs();
  switch (GD.inputs.tag) {
    case 201:
      play_sample_next = true;
      state = CLOCK_STATE;
      points++;
      break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(GD.w / 2, GD.h / 2 - 40, 28, OPT_CENTER, "Alarm!");
  GD.cmd_text(GD.w / 2, GD.h / 2, 29, OPT_CENTER, string2char(job));

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(201);
  GD.cmd_button( GD.w / 2 - 200, GD.h - 60, 400, 50, 27, OPT_FLAT, "OK" );
}

void show_upcoming_events() {

  GD.get_inputs();
  switch (GD.inputs.tag) {
    case 211:
      state = CLOCK_STATE;
      break;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_text(20, 30, 22, 0, "Upcoming calendar events");
  int x = 0;
  for ( std::map<int, String>::iterator i = cache.begin(); i != cache.end(); i++ ) {
    GD.cmd_text(20, 60 + x * 20, 20, 0, string2char( format_time( i->first / 10000, (i->first / 100) % 100, i->first % 100 ) ) );
    GD.cmd_text(80, 60 + x * 20, 20, 0, string2char(i->second));
    x++;
  }

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_GREEN);
  GD.Tag(211);
  GD.cmd_button( GD.w / 2 + 100, GD.h - 60, 100, 50, 27, OPT_FLAT, "OK" );
}

void show_goed_gedaan() {

  GD.get_inputs();
  switch (GD.inputs.tag) {
    case 201:
      play_sample_next = true;
      state = CLOCK_STATE;
    break;
  }

  GD.ColorRGB(COLOR_BEIGE);
  GD.cmd_text(GD.w / 2, GD.h / 2 - 80, 28, OPT_CENTER, "Dat deed je");
  int r = rand() % GOED_AANTAL;
  GD.cmd_text(GD.w / 2, GD.h / 2 - 25, 31, OPT_CENTER, goed[ goed1 ]);
  GD.cmd_text(GD.w / 2, GD.h / 2 + 25, 31, OPT_CENTER, goed[ goed2 ]);

  GD.ColorRGB(COLOR_BLACK);
  GD.cmd_fgcolor(COLOR_BEIGE);
  GD.Tag(201);
  GD.cmd_button( GD.w / 2 - 200, GD.h - 60, 400, 50, 27, OPT_FLAT, "Graag gedaan!" );
}

void show_todo_icon( int x, int y, int job, int tag ) {
  if( todo_texts[ job ] != "" ) {
    GD.Begin(BITMAPS);
    GD.ColorRGB(COLOR_BEIGE);
    if ( todo_done[ job ] ) GD.ColorA(32); else GD.ColorA(255);
    GD.Tag(tag);
    if( todo_jpgs[ job ] != -1 )
      GD.Vertex2ii(x, y, todo_jpgs[ job ] );
    else
      GD.Vertex2ii(x, y, UNKNOWN_JPG );
    GD.ColorRGB(COLOR_BLACK);
    GD.cmd_text(x + 50, y + 110, 28, OPT_CENTER, string2char( todo_texts[ job ] ));
    if ( todo_done[ job ] ) {
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
}

void show_todo_icons() {
  show_todo_icon( 93, 10, 0, 100 );
  show_todo_icon( 287, 10, 1, 101 );
  show_todo_icon( 93, 140, 2, 102 );
  show_todo_icon( 287, 140, 3, 103 );

  GD.get_inputs();
  if ( GD.inputs.tag > 0 ) {
    if( !todo_done[ GD.inputs.tag - 100 ] )
      play_sample_next = true;
    todo_done[ GD.inputs.tag - 100 ] = not todo_done[ GD.inputs.tag - 100 ];
    delay(300);
  }

  if ( todo_done[0] && todo_done[1] && todo_done[2] && todo_done[3] ) {
    points += 4;
    goed1 = rand() % GOED_AANTAL;
    goed2 = rand() % GOED_AANTAL;
    state = GOED_GEDAAN_STATE;
  }
}

void show_todo() {
  DateTime now = rtc.now();

  if ( now.unixtime() >= next_alarm_bleep ) {
    next_alarm_bleep = now.unixtime() + 30;
    jingle(2);
  }

  show_todo_icons( );
}

void loop() {

  DateTime now = rtc.now();
  double minutes = (time_timer - now.unixtime()) / (double)SECONDS_PER_MINUTE;
  int current_h = now.hour();
  int current_m = now.minute();
  int current_s = now.second();

  // clear screen
  uint32_t bgcolor = COLOR_BEIGE;
  switch (state) {
    case CLOCK_STATE:
    case SHOW_LOG_STATE:
      bgcolor = COLOR_BLACK;
      break;
    case REMINDER_STATE:
      bgcolor = COLOR_YELLOW;
      break;
    case ALARM_STATE:
      bgcolor = COLOR_RED;
      break;
    case GOED_GEDAAN_STATE:
      bgcolor = COLOR_GREEN;
      break;
  }
  GD.ClearColorRGB(bgcolor);
  GD.Clear();

  // always show the statusbar
  statusbar( current_h, current_m, current_s );

  // dependent on state, show contents
  switch (state) {
    case FINISHED_STATE:
      show_timer_finished();
      break;
    case REMINDER_STATE:
      show_reminder( current_job_string );
      break;
    case ALARM_STATE:
      show_alarm( current_job_string );
      break;
    case TIME_TIMER_STATE:
      run_time_timer();
      break;
    case UPCOMING_STATE:
      show_upcoming_events();
      break;
    case SHOW_LOG_STATE:
      GD.ColorRGB(COLOR_BEIGE);
      show_log();
      break;
    case CLOCK_STATE:
      int cache_time;
      cache_time = current_h * 10000 + current_m * 100 + current_s;
      if ( cache.count( cache_time ) ) {
        parse_command(string2char(cache[cache_time]));
        cache.erase(cache_time);
      }
      show_clock();
      if( current_h == 1 ) points = 1;
      break;
    case TODO_STATE:
      show_todo();
      break;
    case GOED_GEDAAN_STATE:
      show_goed_gedaan();
      break;
  }

  // bring the contents to the front
  GD.swap();

  // dirty hack to play a sample in the next frame
  if( play_sample_next_next )
    sample();
  play_sample_next_next = play_sample_next;
  play_sample_next = false;


  // make sure we're still connected to MQTT
  mqttOnlineCheck();
  // keep generating random numbers to mess with the seed
  rand();

}
