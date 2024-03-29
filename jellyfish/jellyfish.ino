#include <Wire.h>

//this code did ** t h a t ** 
// she, like, really did that
// sis
// she a queen 
//she finna change ya life 
//

#include <Arduino.h>
// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    13

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 117

const int trigPin = 9;
const int echoPin = 10;

int lightFreq=10;
float lightSum=0.0;
float lightAvg=0.0;
float currentAvg=9.0;
float lightStep=0.05;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

long colorArray[65536/256];
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

long duration;
int distance;
float lightValue;


void setup() {
  Wire.begin();
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  lightValue=3.0;
  Serial.begin(9600);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  for(int i=0;i<(65536/768);i++){
    colorArray[i]=i*256;
    Serial.println(i);
    Serial.println(colorArray[i]);
  }
  for(int i=0;i<65536/768;i++){
    colorArray[i+(65536/3)/256]=-(i*256);
    colorArray[i+(65536/3)/256]+=65536/3;
    Serial.println(i+(65536/3)/256);
    Serial.println(colorArray[i+(65536/3)/256]);
  }
  for(int i=0;i<65536/768;i++){
    colorArray[i+(65536*2/3)/256]=256*i;
    Serial.println(i+(65536*2/3)/256);
    Serial.println(colorArray[i+(65536*2/3)/256]);
  }
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

}

void loop() {
  rainbow(10);

}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:

  for(long i=0;i<(65536*2/3)/256;i++){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
// Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
// Calculating the distance
  distance= duration*0.034/2;
// Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(float(distance));
  Serial.print(" ");
  lightValue=map(float(distance),0.0,273.0,2.0,6.0);
  lightValue=distance*4.0/273.0+2.0;
  lightValue+=3.0;
  lightValue=constrain(lightValue,3.0,9.0);
  //Serial.println(lightValue);
  lightSum+=lightValue;
    if(i%lightFreq==0){
      lightAvg=lightSum/lightFreq;
      lightSum=0;
    }
   currentAvg=goTowards(currentAvg,lightAvg);
   Serial.println(currentAvg);
   Wire.beginTransmission(8);
   Wire.write(int(currentAvg*10)); //currentAvg between 5 and 9
   Wire.endTransmission();
    for(int j=0;j<strip.numPixels();j++){
      int pixelHue=colorArray[i+j*(65536/768)/strip.numPixels()];
      pixelHue+=65536*currentAvg/6;
      strip.setPixelColor(j, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}

float goTowards(float from, float to){
  if((from>to && from-lightStep<=to)||(from<to && from+lightStep>=to)||(from==to)){
    return to;
  }else if(from>to){
    return from-=lightStep;
  }else if(to>from){
    return from+=lightStep;  
  }
}
