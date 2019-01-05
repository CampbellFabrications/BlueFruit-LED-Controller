#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"
#include <Adafruit_NeoPixel.h>

#define FACTORYRESET_ENABLE     1
#define PIN                     6
#define NUMPIXELS               16



enum States { SOLID, BLINK , FADE, RAINBOW};
enum Colors { RED , GREEN , BLUE , WHITE};

States state;
Colors color;
char command;

int redState, greenState, blueState, whiteState = 0;
int brightness = 255;
int interval = 100;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGBW + NEO_KHZ800);
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];

//Color Picker
uint8_t red = 255;
uint8_t green = 255;
uint8_t blue = 255;
uint8_t animationState = 1;





void setup(void)
{
  delay(500);
  strip.begin(); // This initializes the NeoPixel library.

  for(uint8_t i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0,0,0)); // off
  }
  colorWipe(strip.Color(255, 0, 0), 15); 
  colorWipe(strip.Color(0, 0, 0), 15); 
  strip.show();

  Serial.begin(115200);
  Serial.println(F("Bluefruit Ice Skate Project - James Campbell"));
  Serial.println(F("--------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));
  
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  Serial.println(F("Changing Name to LED-L"));
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=LED-R" )) ) {
    error(F("Could not set device name?"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!



  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }

  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("***********************"));
}

void loop(void){

  switch(state) {
    case SOLID:
    Serial.println("Solid");
    solid();
    break;

    case BLINK:
    Serial.println("Blink");
    blinking();
    break;

    case FADE:
    Serial.println("Fade");
    fading();
    break;

    case RAINBOW:
    Serial.println("Rainbow");
    rainbow();
    break;    
  }

  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len != 0) {
    // Color Picker
    if (packetbuffer[1] == 'C') {
      uint8_t red = packetbuffer[2];
      uint8_t green = packetbuffer[3];
      uint8_t blue = packetbuffer[4];
      Serial.print ("RGB #");
      if (red < 0x10) Serial.print("0");
      Serial.print(red, HEX);
      if (green < 0x10) Serial.print("0");
      Serial.print(green, HEX);
      if (blue < 0x10) Serial.print("0");
      Serial.println(blue, HEX);

      for(uint8_t i=0; i<NUMPIXELS; i++) {
        strip.setPixelColor(i, strip.Color(green,red,blue,0));
      }
      strip.show();
    }
    //Controller
    else if (packetbuffer[1] == 'B') {
      uint8_t buttnum = packetbuffer[2] - '0';
      boolean pressed = packetbuffer[3] - '0';
      Serial.print("Button ");
      Serial.print(buttnum);
      animationState = buttnum;
      if(pressed) {
        Serial.println(" pressed");
        // Red
        if(animationState == 1){
          if(state != 4) {
            redState = !redState;
          }
        }

        // Green
        if (animationState == 2){
          if(state != 4) {
            greenState = !greenState;
          }
        }

        // Blue
        if (animationState == 3){
          if(state != 4) {
            blueState = !blueState;  
          }
        }

        // White
        if (animationState == 4){
          if(state != 4) {
            whiteState = !whiteState;
          }
        }

        // up
        if (animationState == 5) {
          switch(state) {
            case SOLID:
            brightness += 16;
            Serial.println("Brightness Up");
            break;
            case FADE:
            interval += 10;
            Serial.println("Fade Delay Up");
            break;
            case BLINK:
            interval += 10;
            Serial.println("Strobe Delay Up");
            break;
            case RAINBOW:
            interval += 10;
            Serial.println("Rainbow Delay Up");
            break;
          }
        }

        // down
        if (animationState == 6) {
          switch(state) {
            case SOLID:
            brightness -= 16;
            Serial.println("Brightness Down");
            break;
            case FADE:
            interval -= 10;
            Serial.println("Fade Delay Down");
            break;
            case BLINK:
            interval -= 10;
            Serial.println("Blink Delay Down");
            break;
            case RAINBOW:
            interval -= 10;
            Serial.println("Rainbow Delay Down");
            break;
          }
        }
        // left
        if (animationState == 7) {
          state = (States)(state - 1);
        }
        // right
        if (animationState == 8) {
          state = (States)(state + 1);
        }

        

      } else {
        Serial.println("released");
      }
    }   
  }

}

void blinking(){
  static unsigned long preMillis = millis();
  static boolean ledState = true;

  if(millis() - preMillis >= interval){
    preMillis = millis();
    ledState = !ledState;
    //digitalWrite(pins[color], ledState);
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      strip.setPixelColor(i, 
        ((greenState && ledState) ? brightness : 0), 
        ((redState && ledState) ? brightness : 0), 
        ((blueState && ledState) ? brightness : 0), 
        ((whiteState && ledState) ? brightness : 0));
    }
    strip.show();
  }
}


void fading(){
  static unsigned long preMillis = millis();
  static int b = 0; 
  static bool rising = true;
  int fadeAmount = 15;

  if(millis() - preMillis >= interval){
    preMillis = millis();
    if(rising) {
      b += fadeAmount;
    }
    else{
      b -= fadeAmount;
    }
    if(b >= brightness || b <= 0){
      rising = !rising;
    }
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      strip.setPixelColor(i, (greenState ? b : 0), (redState ? b : 0), (blueState ? b : 0), (whiteState ? b : 0));
    }
    strip.show();

  }
}



void solid(){
  switch(command){
    case 1:
    color = RED;
    break;

    case 2:
    color = GREEN;
    break;

    case 3:
    color = BLUE;
    break;

    case 4:
    color = WHITE;
  }
  for(uint8_t i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, (greenState ? brightness : 0), (redState ? brightness : 0), (blueState ? brightness : 0), (whiteState ? brightness : 0));
  }
  strip.show();
}

void rainbow() {
  static unsigned long preMillis = millis();
  static int j = 0; 
  int speed = 5;

  if(millis() - preMillis >= interval){
    preMillis = millis();
    j += speed;

    if(j >= 255 || j <= 0){
      speed = -speed;
    }
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
  }
}


void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
