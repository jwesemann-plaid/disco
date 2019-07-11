#include <Arduino.h>
#include <Queue.h>
//#include <U8g2lib.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_PIXELS 12
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define OLED_CS_PIN 10
#define OLED_DC_PIN 9
#define OLED_RST_PIN 8
//U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS_PIN, OLED_DC_PIN, OLED_RST_PIN);

/*
pagerduty-alert/ (trigger) => PAGERDUTY_TRIGGER
pagerduty-alert/ (ack.)     => PAGERDUTY_ACKNOWLEDGE
pagerduty-alert/ (resolve) => PAGERDUTY_RESOLVE
burrito-alert/ => BURRITO
*/

enum COMMAND_ENUM { IDLE, PAGERDUTY_TRIGGER, PAGERDUTY_ACKNOWLEDGE, PAGERDUTY_RESOLVE, BURRITO, DISCO_START, DISCO_STOP, DEBUG};
const String COMMANDS[8] = {"IDLE", "PAGERDUTY_TRIGGER","PAGERDUTY_ACKNOWLEDGE", "PAGERDUTY_RESOLVE", "BURRITO", "DISCO_START", "DISCO_STOP", "DEBUG"};

#define COMMAND_QUEUE_LENGTH 10
DataQueue<COMMAND_ENUM> command_queue(COMMAND_QUEUE_LENGTH);

unsigned long current_millis = 0;
unsigned long previous_millis = 0;

bool oled_displaying = false;
unsigned long oled_busy_interval = 2000;
unsigned long oled_transition_millis = 0;

bool neopixel_busy = false;
COMMAND_ENUM command;
unsigned long neopixel_transition_millis = 0;
unsigned long neopixel_busy_interval = 5000;

String dbg_str = "";

void setup(void) {
  Serial.begin(9600);
  strip.begin();
  //  u8g2.begin();
  current_millis = millis();
  previous_millis = current_millis;
 
  for(uint16_t l = 0; l < NEOPIXEL_PIXELS; l++) {
    strip.setPixelColor(l, 0, 0, 0);
  }
  strip.show();
}

void loop(void) {
  // use current_millis to flag when certain actions should be started and ended
  current_millis = millis();

  // check serial rx for command strings and enqueue the valid ones
  // not sure how command strings should look like yet... 
  String received_string = checkSerial();
  int str_len = received_string.length();
  if (str_len > 0) {
    Serial.println("received: " + received_string);
  }

  // check if received string is a valid command and convert to int
  if (received_string.indexOf("PAGERDUTY_TRIGGER") > -1) {
    command_queue.enqueue(PAGERDUTY_TRIGGER);
  } else if (received_string.indexOf("PAGERDUTY_ACKNOWLEDGE") > -1) {
    command_queue.enqueue(PAGERDUTY_ACKNOWLEDGE);
  } else if (received_string.indexOf("PAGERDUTY_RESOLVE") > -1) {
    command_queue.enqueue(PAGERDUTY_RESOLVE);
  } else if (received_string.indexOf("BURRITO") > -1) {
    command_queue.enqueue(BURRITO);
  } else if (received_string.indexOf("DISCO_START") > -1) {
    command_queue.enqueue(DISCO_START);
  } else if (received_string.indexOf("DISCO_STOP") > -1) {
    command_queue.enqueue(DISCO_STOP);
  } else if (received_string.indexOf("DEBUG") > -1) {
    command_queue.enqueue(DEBUG);
  }

  /*
  // debug display command string on oled
  if (received_string.length() > 0 ) {
    dbg_str = received_string;
  }
  int dbg_str_len = dbg_str.length();
  char dbg_buff[dbg_str_len];
  dbg_str.toCharArray(dbg_buff, dbg_str_len);
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_ncenB08_tr);         
  u8g2.drawStr(0, 10, dbg_buff); // x_pos, y_pos, char* buff
  u8g2.sendBuffer();
  */
  
  // start/stop neopixel lighting routines
  if (neopixel_busy) {
    // disco start will keep lights going until disco stop is received
    if (command == DISCO_START) {
      rainbow();
      return;
    } else if (command == DISCO_STOP) {
      for(uint16_t l = 0; l < NEOPIXEL_PIXELS; l++) {
        strip.setPixelColor(l, 0, 0, 0);
      }
      neopixel_busy = false;
      return;
    }

    // all other commands stop after some fixed interval   
    if (current_millis - neopixel_transition_millis >= neopixel_busy_interval) {
      neopixel_busy = false;
      command = IDLE;
      for(uint16_t l = 0; l < NEOPIXEL_PIXELS; l++) {
        strip.setPixelColor(l, 0, 0, 0);
      }
      strip.show();
    } else {
      switch (command) {
        case PAGERDUTY_TRIGGER:
          twinkle();
          break;
        case PAGERDUTY_ACKNOWLEDGE:
          twinkle();
          break;
        case PAGERDUTY_RESOLVE:
          twinkle();
          break;
        case BURRITO:
          rainbow();
          break;
        case DEBUG:
          rainbow();
          break;
        default:
          break;
      }
    }
  } else if (command == PAGERDUTY_TRIGGER ||
             command == PAGERDUTY_ACKNOWLEDGE ||
             command == PAGERDUTY_RESOLVE ||
             command == BURRITO ||
             command == DISCO_START ||
             command == DISCO_STOP ||
             command == DEBUG) {
      Serial.println("starting a neopixel routine!");
      neopixel_transition_millis = millis();
      neopixel_busy = true;
  }

  // pop off items from the command queue and handle them
  if (neopixel_busy || command_queue.isEmpty()) {
    return;
  } else {
    command = command_queue.dequeue();
    Serial.print("popping off: " + command);
    return;
  }
}

// checks serial rx and returns a string
String checkSerial() {
  if (Serial.available() > 0) {
    String incomingStr = Serial.readString();
    return incomingStr;
  }
  return "";
}

/*// writes string to the oled screen starting from top-left
void writeToOled(char* buff) {
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_ncenB08_tr);         
  u8g2.drawStr(0, 10, buff); // x_pos, y_pos, char* buff
  u8g2.sendBuffer();
}

// clears/writes oled screen
void updateOled(char* wr_buff) {
  if (oled_displaying) {
    if (current_millis - oled_transition_millis >= oled_busy_interval) {
      oled_transition_millis = current_millis;
      u8g2.clearDisplay();
    } 
  } else {
    writeToOled(wr_buff);
    oled_displaying = true;
    oled_transition_millis = current_millis;
  }
}*/

void twinkle() {
  float redStates[NEOPIXEL_PIXELS];
  float blueStates[NEOPIXEL_PIXELS];
  float greenStates[NEOPIXEL_PIXELS];
  float fadeRate = 0.96;

  if (random(20) == 1) {
    uint16_t i = random(NEOPIXEL_PIXELS);
    if (redStates[i] < 1 && greenStates[i] < 1 && blueStates[i] < 1) {
      redStates[i] = random(256);
      greenStates[i] = random(256);
      blueStates[i] = random(256);
    }
  }
  
  for(uint16_t l = 0; l < NEOPIXEL_PIXELS; l++) {
    if (redStates[l] > 1 || greenStates[l] > 1 || blueStates[l] > 1) {
      strip.setPixelColor(l, redStates[l], greenStates[l], blueStates[l]);
      
      if (redStates[l] > 1) {
        redStates[l] = redStates[l] * fadeRate;
      } else {
        redStates[l] = 0;
      }
      
      if (greenStates[l] > 1) {
        greenStates[l] = greenStates[l] * fadeRate;
      } else {
        greenStates[l] = 0;
      }
      
      if (blueStates[l] > 1) {
        blueStates[l] = blueStates[l] * fadeRate;
      } else {
        blueStates[l] = 0;
      }
      
    } else {
      strip.setPixelColor(l, 0, 0, 0);
    }
  }
  strip.show();
  delay(10);
}

void rainbow() {
  for (int i = 0; i < 6; i++) {
    strip.clear(); // Set all pixel colors to 'off'
    int red;
    int green;
    int blue;
    
      if (i == 0) {
        //Indigo
        red = 75;
        green = 0;
        blue = 130;
      } else if ( i == 1 ) {
        //Blue
        red = 0;
        green = 0;
        blue = 255;
      } else if ( i == 2 ) {
        //Green
        red = 0;
        green = 255;
        blue = 0;
      } else if ( i == 3 ) {
        //Yellow
        red = 255;
        green = 255;
        blue = 0;
      } else if ( i == 4 ) {
        //Orange
        red = 255;
        green = 127;
        blue = 0;
      } else if ( i == 5 ) {
        //Red
        red = 255;
        green = 0;
        blue = 0;
      } 

    for (int j = 0; j < NEOPIXEL_PIXELS; j++) {
      strip.setPixelColor(j, strip.Color(red, green, blue));
      strip.show();   // Send the updated pixel colors to the hardware.
      delay(50); // Pause before next pass through loop
    }
  }
}
