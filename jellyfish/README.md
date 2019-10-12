## For Thursday 24th:


#### Ahmed, Pangna, and Hatim's JellyBot

#### 09/10/2019 Update:

We are making a jellyfish Robot. The JellyBot is composed of 3 main parts:

  1. A traction system.

  2. The jellyfish body.

  3. The 4 tentacles of the jellyfish.


The features that we've already done:

  1. Built a body out of cardboard and painted it blue, with holes for the light, with tracing paper on top.

  2. Neopixels that shift between blue and purple inside the body.

  3. Built a support for the lifting mechanism.

  4. Built the basics for the legs and tested servos on them. 


The features that we want for sure to include in our JellyBot are (including partially completed ones), which will make our JellyBot "complete":

  1. All the parts are assembled together

  2. An autonomous lifting mechanism, with a spring between the mechanism and the body.

  3. Lights coming out of the body.

  4. Legs that move automatically, using randomness or noise.


Features that we might add depending on time and constraints:

  1. Wireless controls for the lift.

  2. Lights that react to a distance sensor.

  3. Legs that react to the distance sensor, maybe becoming wilder as people get closer.

  4. Springs between the body and the legs.
  
 #### 12/10/2019 Update:
 
 (Since I haven't posted any proper update, this is going to include everything I've done so far.)
 
 The first thing I did for this project was building the outer body for the Jellybot, which I wanted to be dome-like, but still made of cardboard for the sake of lightness and not to over-complicate it. I followed [this video](https://youtu.be/W-626HKjzQI) with slight modifications:
 
  1. I used smaller shapes (7 inch)
  
  2. I stopped adding pieces after I had made a half-sphere.
  
  3. I cut circular holes for light to come out of the jellyfish, and I glued tracing paper to filter it. I also cut a hole on top to carry the Jellybot up and down.
  
  4. I painted it with a light coat of blue.
  
  
  ![](IMG_20191012_170208.jpg)
  
  
  ![](IMG_20191012_170158.jpg)
  
  
  When I was done with this, I cut a long piece of cardboard to serve as the base of the body. On top of it, I built a sort of christmas tree shape with cardboard so that I could attach neopixels to it, this way, the light could be equally distributed amongst the holes of the Jellybot body. As for the neopixels themselves, I put a few pieces together and connected the power to 5V, ground to ground, and data in to pin 13, and I soldered it all together. Following [this tutorial](https://learn.adafruit.com/adafruit-neopixel-uberguide/basic-connections), I also added a 1000 µF capacitor to "prevent the initial onrush of current from damaging the pixels" between 5V and ground, and a 470 ohm resistor between data in and pin 13. 
  
  
  ![The tree I built for the neopixels](IMG_20191012_165759.jpg)
  
  
  ![The cables coming out of the neopixels](IMG_20191012_165825.jpg)
  
  
  After this, I decided to add a distance sensor to the circuit, as to be able to change the colors coming out of the Jellybot depending on proximity, to simulate fear from predators. I attached the sensor's power to 5V, ground to ground, trigger to pin 9, and echo to pin 10, and soldered everything. I then put zip ties on the cables to have a bit less of a mess and give some relief to the connections. I also used velcro to attach the Arduino to the base as well as a power bank. I used [this tutorial](https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/) to figure out the sensor connections and code.
  
  
  ![The distance sensor](IMG_20191012_165807.jpg)
  
  
  ![Arduino connections](IMG_20191012_165817.jpg)
  
  
  ![Stress relief](IMG_20191012_165845.jpg)
  
  
  ![Schematics for the entire circuit](IMG_20191012_173315.jpg)
  
  
  As for the code, I started by modifying the strandtest example on the Arduino neopixels library to make the colors sweep from purple to blue. Then, I added code that takes data from the distance sensor to change the range of the colors; red to yellow if something is very close, green to blue in the middle, etc. The most difficult part of the code was making it so that the changes in color weren't too choppy/random since the sensor is extremely sensitive. I fixed this by calculating the average of the sensor values every few frames, and using a goTowards function so that the color ranges would change gradually rather than instantly.


``` 
float goTowards(float from, float to){
  if((from>to && from-lightStep<=to)||(from<to && from+lightStep>=to)||(from==to)){
    return to;
  }else if(from>to){
    return from-=lightStep;
  }else if(to>from){
    return from+=lightStep;  
  }
}
```


[My friend playing with the distance sensor](Video_20191012174602465_by_videoshow.mp4)


For now, I just blu-tack'd the sensor to the body of the Jellybot, as shown in the video, but I plan to eventually put it somewhere stationary so that the movement of the Jellybot doesn't interfere with it, and at chest level rather than leg level, which is easier to detect for the sensor.
