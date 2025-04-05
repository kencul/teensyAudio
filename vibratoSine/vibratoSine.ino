#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "AudioSineVibrato.h"

AudioSineVibrato sine;
AudioOutputI2S           i2s1;

AudioConnection c1(sine, 0, i2s1, 0);
AudioConnection c2(sine, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  AudioMemory(20); // Allocate x 
  sgtl5000_1.enable(); // turn on audio codec chip
  sgtl5000_1.volume(0.32); // set output volume of chip

  sine.frequency(440);
  sine.amplitude(1);
  sine.vibFrequency(1);
  sine.vibratoDepth(220);
}

void loop() {
  // put your main code here, to run repeatedly:

}
