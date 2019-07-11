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


enum COMMAND_ENUM { IDLE, PAGERDUTY_ALERT, PAGERDUTY_ACK, BURRITO_ALERT, DEBUG_ALERT, DEBUG_ACK};
const String COMMANDS[6] = {"IDLE", "PAGERDUTY_ALERT","PAGERDUTY_ACK","BURRITO_ALERT","DEBUG_ALERT","DEBUG_ACK"};

#define COMMAND_QUEUE_LENGTH 10
DataQueue<COMMAND_ENUM> command_queue(COMMAND_QUEUE_LENGTH);
// ???

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

  /*while (current_millis - previous_millis <= 5000) {
    current_millis = millis();
    twinkle();
  }*/
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

  // check if received string is a valid command and convert to int
  if (received_string.indexOf("DEBUG_ALERT") > -1) {
    command_queue.enqueue(DEBUG_ALERT);
  } else if (received_string.indexOf("DEBUG_ACK") > -1) {
    command_queue.enqueue(DEBUG_ACK);
  }
  
  // debug display command string on oled
  if (received_string.length() > 0 ) {
    dbg_str = received_string;
  }
  /*int dbg_str_len = dbg_str.length();
  char dbg_buff[dbg_str_len];
  dbg_str.toCharArray(dbg_buff, dbg_str_len);
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_ncenB08_tr);         
  u8g2.drawStr(0, 10, dbg_buff); // x_pos, y_pos, char* buff
  u8g2.sendBuffer();
*/
  // start/stop neopixel lighting routines
  if (neopixel_busy) {
    if (current_millis - neopixel_transition_millis >= neopixel_busy_interval) {
      neopixel_busy = false;
      command = IDLE;
      for(uint16_t l = 0; l < NEOPIXEL_PIXELS; l++) {
        strip.setPixelColor(l, 0, 0, 0);
      }
      strip.show();
    } else {
      neopixel_busy = true;
      switch (command) {
        case PAGERDUTY_ALERT:
          break;
        case PAGERDUTY_ACK:
          break;
        case BURRITO_ALERT:
          break;
        case DEBUG_ALERT:
          twinkle();
          break;
        case DEBUG_ACK:
          rainbow();
          break;
        default:
          break;
      }
    }
  } else {
      switch (command) {
        case PAGERDUTY_ALERT:
          break;
        case PAGERDUTY_ACK:
          break;
        case BURRITO_ALERT:
          break;
        case DEBUG_ALERT:
          neopixel_busy = true;
          neopixel_transition_millis = millis();
          twinkle();
          break;
        case DEBUG_ACK:
          neopixel_busy = true;
          neopixel_transition_millis = millis();
          rainbow();
          break;
        default:
          break;
      }
  }

  // pop off items from the command queue and handle them
  if (neopixel_busy || command_queue.isEmpty()) {
    return;
  } else {
    // this command will also have some sort of payload (JSON)
    // probably need to use a struct here instead of just ints
    command = command_queue.dequeue();
    return;
  }

}

// checks serial rx and returns a string
String checkSerial() {
  if (Serial.available() > 0) {
    String incomingStr = Serial.readString();
    //Serial.print("received " + incomingStr);
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
