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

#include <Bounce.h>

Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);  // 15 = 15 ms debounce time
Bounce button2 = Bounce(2, 15);

// Exponential Moving Average for smoothing pot input (REF: Deepseek)
// Struct to store previous values for smoothing + change detection
struct SmoothPot{
    float emaAlpha = 0.1;  // Smoothing factor (0.1 = smoother, 0.5 = more responsive)
    float smoothedValue = 0;
    float lastValue = 0;
};

SmoothPot pots[3];
const int pins[3] = {A1, A2, A3};
const float AMP = 0.2f;

int readSmoothPot(int pin, SmoothPot &pot) {
    int raw = analogRead(pin);
    pot.smoothedValue = pot.emaAlpha * raw + (1 - pot.emaAlpha) * pot.smoothedValue;
    return pot.smoothedValue;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  AudioMemory(20); // Allocate x 
  sgtl5000_1.enable(); // turn on audio codec chip
  sgtl5000_1.volume(0.32); // set output volume of chip

  sine.frequency(440);
  sine.amplitude(AMP);
  sine.vibFrequency(1);
  sine.vibratoDepth(220);

  // set button pins to pullup resist
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
}

u_int knob1History;
bool sineOn = true;
float freqScaled, vibDepthScaled, vibFreqScaled;

void loop() {
  // get button values
  button0.update();
  button1.update();
  button2.update();

  u_int potValue[3];

  for (u_int i = 0; i < 3; i++) {
    // Read smoothed value (0–1023)
    potValue[i] = readSmoothPot(pins[i], pots[i]);
  }

  float freqValue = pow((float)potValue[0] / 1023.0, 4.0);
  if (abs(freqValue - pots[0].lastValue) > 0.02){
    freqScaled = freqValue * 1500;
    pots[0].lastValue = freqValue;
    sine.frequency(freqScaled);
    Serial.printf("Freq: %f\n", freqScaled);
  }

  // get vibrato frequency
  float vibFreqValue = pow((float)potValue[1] / 1023.0, 4.0);
  if (abs(vibFreqValue - pots[1].lastValue) > 0.02){
    pots[1].lastValue = vibFreqValue;
    vibFreqScaled = vibFreqValue * 15;
    sine.vibFrequency(vibFreqScaled);
    Serial.printf("Vibrato freq: %f\n", vibFreqScaled);
  }

  // get vibrato depth
  float vibDepthValue = pow((float)potValue[2] / 1023.0, 4.0);
  if (abs(vibDepthValue - pots[2].lastValue) > 0.02){
    pots[2].lastValue = vibDepthValue;
    vibDepthScaled = freqScaled * 0.26f * vibDepthValue;
    sine.vibratoDepth(vibDepthScaled);
    Serial.printf("Vibrato depth: %f\n", vibDepthValue);
  }

  if (button0.fallingEdge()){
    sineOn = !sineOn;
    if(sineOn){
      sine.amplitude(AMP);
      Serial.println("Synth on!");
    } else {
      sine.amplitude(0);
      Serial.println("Synth off!");
    }
  }
}
