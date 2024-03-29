  /*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN    6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 180

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_MotorShield AFMS2 = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);
Adafruit_DCMotor *myMotor2 = AFMS2.getMotor(2);

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         0
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


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

bool goingForward;
bool goingBackward;

int thirdOrder[60];
int secondOrder[60];

int state=0;

int saturation=170;
int brightness=90;
int hue=0;

uint32_t myColor;
//uint32_t colors2[12];
//uint32_t colors3[12];

int frameCount; 

bool thirdEye=false;
bool addSaturation=false;

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{ 
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.clear();
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  Serial.begin(115200);

  frameCount=0;

  thirdEye=false;

  state=0;
  
  bool dir=true;
  int k=0;
  for(int i=0;i<60;i++){      //here I set up arrays of neopixel positions so that it's easier to locate them on an xy axis.
     if(i%12==0){
      dir=!dir;
      k=i;
    }
    if(dir==true){
      secondOrder[i]=119-i;
      thirdOrder[i]=179-(11-(i%12))-k;
    }else if(dir==false){
      secondOrder[i]=119-(11-(i%12))-k;
      thirdOrder[i]=179-i;
    }
  }
  
  goingForward=false;
  goingBackward=false;
      AFMS.begin();
  AFMS2.begin();
    myMotor->setSpeed(150);
    myMotor2->setSpeed(150);

  myMotor->run(FORWARD);
    myMotor2->run(FORWARD);

  // turn on motor
  myMotor->run(RELEASE);
  myMotor2->run(RELEASE);


  
  while (!Serial);  // required for Flora & Micro
  delay(500);

  
  Serial.println(F("Adafruit Bluefruit App Controller Example"));
  Serial.println(F("-----------------------------------------"));

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

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

  thirdEye=false;

}

int motorSpeed = 0;

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  //strip.clear();
  /* Wait for new data to arrive */
  //Serial.println(frameCount);

  frameCount++;

  if(frameCount%5==0){
    blink();
  }
  if(state==0){           //colorCycle changes the hue slowly
    colorCycle();
  }else if(state==1){     //multiColor assigns a random color to every LED
    multiColor();
  }else if(state==2){     //fullColor assigns one random color to all the LEDs
    fullColor();
  }

  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  /* Got a packet! */
  // printHex(packetbuffer, len);

  // Color
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
  }

  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    Serial.print ("Button "); Serial.print(buttnum);
    if (pressed) {
      Serial.println(" pressed");
    } else {
      Serial.println(" released");
    }

    if (pressed ) {
      if(buttnum==1){
        if(state!=1){           //goes multicolor
            state=1;
        }else{                  //goes back to normal
            state=0;
        }
       strip.show();
       Serial.println("current state");
       Serial.println(state);
      }else if(buttnum==2){     //a random color is picked
        if(state!=2){
          state=2;
          int k=random(0,65536);
          myColor=strip.ColorHSV(k,255,255);
          Serial.println("current hue");
          Serial.println(k);
        }else{                  //goes back to normal
          state=0;
        }
        Serial.println("current state");
       Serial.println(state);
      }else if(buttnum==3){     //opens or closes third eye
       thirdEye=!thirdEye;
       Serial.println("third eye");
       if(thirdEye==true){
        thirdEyeOpen();
       }else{
        thirdEyeClose();
       }
      }else if(buttnum==4){
        addSaturation=!addSaturation;     //adds brightness and saturation
        if(brightness==90){
          brightness=255;
        }else{
          brightness=90;
        }
      }
      if (buttnum == 5) {
        Serial.print(" Forward");
        myMotor->setSpeed(255);
        myMotor2->setSpeed(180);
        myMotor->run(FORWARD);
        myMotor2->run(BACKWARD);
        goingForward=true;
      }
      if (buttnum == 6) {
      Serial.print(" Backward");
        myMotor->setSpeed(255);
        myMotor2->setSpeed(255);
      myMotor->run(BACKWARD);
      myMotor2->run(FORWARD);
      goingBackward=true;
      }
      if (buttnum == 8) {
      Serial.print(" Right");
      if(goingForward==false && goingBackward==false){
        myMotor->setSpeed(255);
        myMotor2->setSpeed(255);
      myMotor->run(BACKWARD);
      myMotor2->run(BACKWARD);
      }
      else if(goingForward==true && goingBackward==false){      //moves diagonally forward
        Serial.print("Sliding");
        myMotor->setSpeed(125);
        myMotor2->setSpeed(255);
        myMotor->run(FORWARD);
        myMotor2->run(BACKWARD);
      }else if(goingBackward==true && goingForward==false){     //moves diagonally backward
        Serial.print("Sliding");
        myMotor->setSpeed(125);
        myMotor2->setSpeed(255);
        myMotor->run(BACKWARD);
        myMotor2->run(FORWARD);
      }
      }
      if (buttnum == 7) {
      Serial.print(" Left");
      if(goingForward==false && goingBackward==false){
        myMotor->setSpeed(255);
        myMotor2->setSpeed(255);
      myMotor->run(FORWARD);
      myMotor2->run(FORWARD);
      }else if(goingForward==true && goingBackward==false){   //diagonally forward
        Serial.print("Sliding");
        myMotor->setSpeed(255);
        myMotor2->setSpeed(70);
        myMotor->run(FORWARD);
        myMotor2->run(BACKWARD);
      }else if(goingForward==false && goingBackward==true){   //diagonally backward
        Serial.print("Sliding");
        myMotor->setSpeed(255);
        myMotor2->setSpeed(70);
        myMotor->run(BACKWARD);
        myMotor2->run(FORWARD);
      }
      }
    } else { // button has been released
      Serial.print(" Stop");
      myMotor->run(RELEASE);
      myMotor2->run(RELEASE);
      goingForward=false;
      goingBackward=false;
    }
    
  }

  // GPS Location
  if (packetbuffer[1] == 'L') {
    float lat, lon, alt;
    lat = parsefloat(packetbuffer+2);
    lon = parsefloat(packetbuffer+6);
    alt = parsefloat(packetbuffer+10);
    Serial.print("GPS Location\t");
    Serial.print("Lat: "); Serial.print(lat, 4); // 4 digits of precision!
    Serial.print('\t');
    Serial.print("Lon: "); Serial.print(lon, 4); // 4 digits of precision!
    Serial.print('\t');
    Serial.print(alt, 4); Serial.println(" meters");
  }

  // Accelerometer
  if (packetbuffer[1] == 'A') {
    float x, y, z;
    x = parsefloat(packetbuffer+2);
    y = parsefloat(packetbuffer+6);
    z = parsefloat(packetbuffer+10);
    Serial.print("Accel\t");
    Serial.print(x); Serial.print('\t');
    Serial.print(y); Serial.print('\t');
    Serial.print(z); Serial.println();
  }

  // Magnetometer
  if (packetbuffer[1] == 'M') {
    float x, y, z;
    x = parsefloat(packetbuffer+2);
    y = parsefloat(packetbuffer+6);
    z = parsefloat(packetbuffer+10);
    Serial.print("Mag\t");
    Serial.print(x); Serial.print('\t');
    Serial.print(y); Serial.print('\t');
    Serial.print(z); Serial.println();
  }

  // Gyroscope
  if (packetbuffer[1] == 'G') {
    float x, y, z;
    x = parsefloat(packetbuffer+2);
    y = parsefloat(packetbuffer+6);
    z = parsefloat(packetbuffer+10);
    Serial.print("Gyro\t");
    Serial.print(x); Serial.print('\t');
    Serial.print(y); Serial.print('\t');
    Serial.print(z); Serial.println();
  }

  // Quaternions
  if (packetbuffer[1] == 'Q') {
    float x, y, z, w;
    x = parsefloat(packetbuffer+2);
    y = parsefloat(packetbuffer+6);
    z = parsefloat(packetbuffer+10);
    w = parsefloat(packetbuffer+14);
    Serial.print("Quat\t");
    Serial.print(x); Serial.print('\t');
    Serial.print(y); Serial.print('\t');
    Serial.print(z); Serial.print('\t');
    Serial.print(w); Serial.println();
  }


}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void blink(){
  if(strip.getPixelColor(secondOrder[0]-60)!=0){

    for(int j=1;j<6;j++){           //every LED row is progressively turned off
      for(int i=0;i<12*j;i++){
        strip.setPixelColor(secondOrder[i]-60,strip.ColorHSV(0,255,0));
        strip.setPixelColor(secondOrder[i],strip.ColorHSV(0,255,0));
        if(thirdEye==true){
          strip.setPixelColor(thirdOrder[i],strip.ColorHSV(0,255,0));
        }
      }
      strip.show();
      delay(50);
    }
    delay(100);

    for(int j=1;j<6;j++){   //turned back on. the colors are different depending on the state.
      for(int i=0;i<12*j;i++){
        if(state==0){
          strip.setPixelColor(secondOrder[59-i]-60,strip.ColorHSV(hue+256*(11-(i%12)),saturation,brightness));
          strip.setPixelColor(secondOrder[59-i],strip.ColorHSV(hue+256*(35-(i%12)),saturation,brightness));
        }else if(state==1){
          strip.setPixelColor(secondOrder[59-i]-60,random(0,255),random(0,255),random(0,255));
          strip.setPixelColor(secondOrder[59-i],random(0,255),random(0,255),random(0,255));
        }else if(state==2){
          strip.setPixelColor(secondOrder[59-i]-60,myColor);
          strip.setPixelColor(secondOrder[59-i],myColor);          
        }
        if(thirdEye==true){
          if(state==0){
            strip.setPixelColor(thirdOrder[59-i],strip.ColorHSV(hue+256*(23-(i%12)),saturation,brightness)); 
          }else if(state==1){
            strip.setPixelColor(thirdOrder[59-i],random(0,255),random(0,255),random(0,255)); 
          }else if(state==2){
            strip.setPixelColor(thirdOrder[59-i],myColor); 
          }
        }
      }
      strip.show();
      delay(50);
    }
    Serial.println(hue);
    delay(100);
  }
}

void colorCycle(){
  if(addSaturation==true && saturation<=230){         //saturation progressively increases or decreases depending on the state
    saturation+=20;
  }else if(addSaturation==false && saturation>150){
    saturation-=20;
  }
  Serial.println(thirdEye);
  saturation=saturation+random(-10,10);
  if(saturation>255){                             
    saturation=255;
  }
  for(int i=0;i<12;i++){                              //hue changes a bit every frame
    int currentHue=hue+i*256;
    for(int j=0;j<5;j++){
      strip.setPixelColor(secondOrder[i+j*12]-60,strip.ColorHSV(currentHue,saturation,brightness));
    }
  }
  if(thirdEye==true){
    for(int i=0;i<12;i++){
      int currentHue=hue+(i+12)*256;
      for(int j=0;j<5;j++){
        strip.setPixelColor(thirdOrder[i+j*12],strip.ColorHSV(currentHue,saturation,brightness));
      }
    }
  }
  for(int i=0;i<12;i++){
    int currentHue=hue+(i+24)*256;
    for(int j=0;j<5;j++){
      strip.setPixelColor(secondOrder[i+j*12],strip.ColorHSV(currentHue,saturation,brightness));
    }
  }

  strip.show();
  hue+=500;
}

void multiColor(){                          //shows a different random color on all LEDs
  for(int i=0;i<12;i++){
  int currentHue=hue+i*256;
  for(int j=0;j<5;j++){
      strip.setPixelColor(secondOrder[i+j*12]-60,random(0,255),random(0,255),random(0,255));
    }
  }
  if(thirdEye==true){
    for(int i=0;i<12;i++){
      int currentHue=hue+(i+12)*256;
      for(int j=0;j<5;j++){
        strip.setPixelColor(thirdOrder[i+j*12],random(0,255),random(0,255),random(0,255));
      }
    }
  }
  for(int i=0;i<12;i++){
    int currentHue=hue+(i+24)*256;
    for(int j=0;j<5;j++){
      strip.setPixelColor(secondOrder[i+j*12],random(0,255),random(0,255),random(0,255));
    }
  }
  strip.show();
}

void fullColor(){                     //shows one single color on all LEDs
  for(int i=0;i<12;i++){
  int currentHue=hue+i*256;
  for(int j=0;j<5;j++){
      strip.setPixelColor(secondOrder[i+j*12]-60,myColor);
    }
  }
  if(thirdEye==true){
    for(int i=0;i<12;i++){
      int currentHue=hue+(i+12)*256;
      for(int j=0;j<5;j++){
        strip.setPixelColor(thirdOrder[i+j*12],myColor);
      }
    }
  }
  for(int i=0;i<12;i++){
    int currentHue=hue+(i+24)*256;
    for(int j=0;j<5;j++){
      strip.setPixelColor(secondOrder[i+j*12],myColor);
    }
  }
  strip.show();
}

void thirdEyeClose(){                   //closing the third eye, similar to how blinking works
    for(int j=1;j<6;j++){
      for(int i=0;i<12*j;i++){
        strip.setPixelColor(thirdOrder[i],strip.ColorHSV(0,255,0));
      }
      strip.show();
      delay(50);
    }
    delay(100);
}

void thirdEyeOpen(){                  //opening third eye, similar to how opening an eye in the blinking function works
   for(int j=1;j<6;j++){
      if(state==0){
        for(int i=0;i<12*j;i++){
          strip.setPixelColor(thirdOrder[59-i],strip.ColorHSV(hue+256*(23-(i%12)),saturation,brightness)); 
        }
      }else if(state==1){
        for(int i=0;i<12*j;i++){
          strip.setPixelColor(thirdOrder[59-i],random(0,255),random(0,255),random(0,255)); 
        }
      }else if(state==2){
        for(int i=0;i<12*j;i++){
          strip.setPixelColor(thirdOrder[59-i],myColor); 
        }        
      }
      strip.show();
      delay(50);
    }
    //Serial.println(hue);
    delay(100);
}
