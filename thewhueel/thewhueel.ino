// FIXME: Attempt to change USB device name!
#define STR_PRODUCT L"The Whueel MIDI"

#if !defined(__AVR_ATmega32U4__) || !defined(CORE_TEENSY)
#error "Only designed to run on Teensy 2.0!"
#endif

#if !defined(USB_MIDI)
#error "USB Type should be set to MIDI!"
#endif

#include "ACE128.h"
#include <ACE128map12345678.h>
#include <Adafruit_NeoPixel.h>

#define MIDI_CHANNEL 7
#define MIDI_CC_NUM 111

#define ACE_PIN1 10
#define ACE_PIN2 9
#define ACE_PIN3 8
#define ACE_PIN4 7
#define ACE_PIN5 11
#define ACE_PIN6 14
#define ACE_PIN7 13
#define ACE_PIN8 12

#define PIXEL_COUNT 12
#define PIXEL_OFFSET 8
#define PIXEL_PIN 6

// ACE128 instance with pin mapping.
ACE128 encoder(ACE_PIN1, ACE_PIN2, ACE_PIN3, ACE_PIN4, ACE_PIN5, ACE_PIN6, ACE_PIN7, ACE_PIN8, (uint8_t*)encoderMap_12345678);

// NeoPixel ring (12 LED).
Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    encoder.begin();
    encoder.setZero(0);
    pixels.begin();
    Serial.begin(9600);
    Serial.println(encoder.acePins());
}

static void check_pins(uint8_t pins)
{
    static uint8_t seen_pins = 0;
    static uint8_t old_pins = 255;

    if (pins == old_pins) {
        return;
    }

    seen_pins |= pins ^ old_pins;
    old_pins = pins;
    if (seen_pins < 255) {
        Serial.print("looking for pins: ");
        for (uint8_t i = 0; i <= 7; i++) {
            if (! (seen_pins & 1 << i)) {
                Serial.print(i, DEC);
            }
        }
        Serial.println("");
    }
    else {
        Serial.print("pins = ");
        Serial.println(pins, DEC);
    }
}

static void update_midi(uint8_t upos)
{
    static uint8_t old_upos = 255;
    static elapsedMillis msec = 0;

    // Send MIDI updates at most 50x/sec and only if input changed.
    if (msec >= 20 && upos != old_upos) {
        msec = 0;
        old_upos = upos;
        usbMIDI.sendControlChange(MIDI_CC_NUM, upos, MIDI_CHANNEL);
    }

    // MIDI Controllers should discard incoming MIDI messages.
    // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
    while (usbMIDI.read()) {
        // ignore incoming messages
    }
}

static void update_leds(uint8_t upos)
{
    static uint8_t old_upos = 255;

    if (upos == old_upos) {
        return;
    }

    old_upos = upos;
    pixels.setBrightness(255);

    for (uint16_t p = 0; p < pixels.numPixels(); p++) {
        uint16_t pIndex = (p + PIXEL_OFFSET) % pixels.numPixels();
        uint16_t pPos = pIndex * 128 / pixels.numPixels();
        uint16_t pDiff = abs((int16_t)pPos - (int16_t)upos);
        if (pDiff > 64) {
          pDiff = 128 - pDiff;
        }
        uint8_t pVal = 15;
        if (pDiff <= (128 / pixels.numPixels())) {
          pVal = 255 - pDiff * 288 / pixels.numPixels();
        }
        uint32_t pColor = pixels.ColorHSV(pIndex * (65535 / pixels.numPixels()), 255, pVal);
        pixels.setPixelColor(p, pColor);
    }

    pixels.show();
}

void loop()
{
    uint8_t pins = encoder.acePins();
    uint8_t upos = encoder.upos();

    check_pins(pins);
    update_midi(upos);
    update_leds(upos);
}
