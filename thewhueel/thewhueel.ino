// #define STR_PRODUCT             L"The Whueel MIDI"
#include "ACE128.h"  // Absolute Contact Encoder
#include <ACE128map12345678.h> // mapping for pin order 12345678
#include <Wire.h> // I2C bus communication library - required to support ACE128
#include <Adafruit_NeoPixel.h>

// Create an ACE128 instance called myACE
ACE128 myACE(10,9,8,7,11,14,13,12, (uint8_t*)encoderMap_12345678);

uint8_t pinPos = 0; // pin values
uint8_t oldPos = 255;
uint8_t old_upos = 255;
uint8_t upos = 0;
uint8_t seen = 0;
elapsedMillis msec = 0;


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 12 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  int error = 1;
  myACE.begin();    // this is required for each instance, initializes the pins
  pinPos = myACE.acePins();          // get IO expander pins
  oldPos = pinPos;                 // remember where we are
  Serial.begin(9600);
  Serial.println(myACE.acePins());
  pixels.begin();
}


void loop() {
 
  pinPos = myACE.acePins();          // get IO expander pins
  upos = myACE.upos();               // get logical position - unsigned

  if (pinPos != oldPos) {            // did we move?
    seen |= pinPos ^ oldPos;         // what changed?
    oldPos = pinPos;                 // remember where we are
    if (seen < 255) {
      Serial.print("looking for pins: ");
      for (uint8_t i = 0; i <= 7; i++) {
        if (! (seen & 1 << i)) {
          Serial.print(i, DEC);
        }
      }
      Serial.println("");
    } else {
      Serial.print(" upos ");
      Serial.println(upos, DEC);
    }
  }

  // only check the analog inputs 50 times per second,
  // to prevent a flood of MIDI messages
  pixels.setBrightness(0x10);
  if (msec >= 20) {
    msec = 0;
    // only transmit MIDI messages if analog input changed
    if (upos != old_upos) {
      usbMIDI.sendControlChange(111, upos, 7);
      old_upos = upos;
      pixels.setPixelColor(4, (upos >= 123 || upos < 6) ? 0xff0000 : 0x400000);
      pixels.setPixelColor(5, (upos >= 6 && upos < 17) ? 0xbf3f00 : 0x301000);
      pixels.setPixelColor(6, (upos >= 17 && upos < 27) ? 0x7f7f00 : 0x202000);
      pixels.setPixelColor(7, (upos >= 27 && upos < 38) ? 0x3fbf00 : 0x103000);
      pixels.setPixelColor(8, (upos >= 38 && upos < 49) ? 0x00ff00 : 0x004000);
      pixels.setPixelColor(9, (upos >= 49 && upos < 59) ? 0x00bf3f : 0x003010);
      pixels.setPixelColor(10, (upos >= 59 && upos < 70) ? 0x007f7f : 0x002020);
      pixels.setPixelColor(11, (upos >= 70 && upos < 81) ? 0x003fbf : 0x001030);
      pixels.setPixelColor(0, (upos >= 81 && upos < 91) ? 0x0000ff: 0x000040);
      pixels.setPixelColor(1, (upos >= 91 && upos < 102) ? 0x3f00bf : 0x100030);
      pixels.setPixelColor(2, (upos >= 102 && upos < 113) ? 0x7f007f: 0x200020);
      pixels.setPixelColor(3, (upos >= 113 && upos < 123) ? 0xbf003f: 0x300010);
      
      //for (int i=0; i < pixels.numPixels(); i++) {
      //  pixels.setPixelColor(i, () << 16
        //pixels.setPixelColor(i, upos);
      //}
      pixels.show();
    }
  }

  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
    // ignore incoming messages
  }

}
