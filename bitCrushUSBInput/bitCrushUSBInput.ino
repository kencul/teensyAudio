#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=388,514
AudioEffectBitcrusher    bitcrusher2;    //xy=679,559
AudioEffectBitcrusher    bitcrusher1;    //xy=702,499
AudioMixer4              mixer2;         //xy=898,558
AudioMixer4              mixer1;         //xy=900,473
AudioOutputI2S           i2s1;           //xy=1077,514
AudioConnection          patchCord1(usb1, 0, bitcrusher1, 0);
AudioConnection          patchCord2(usb1, 0, mixer1, 0);
AudioConnection          patchCord3(usb1, 1, bitcrusher2, 0);
AudioConnection          patchCord4(usb1, 1, mixer2, 0);
AudioConnection          patchCord5(bitcrusher2, 0, mixer2, 1);
AudioConnection          patchCord6(bitcrusher1, 0, mixer1, 1);
AudioConnection          patchCord7(mixer2, 0, i2s1, 1);
AudioConnection          patchCord8(mixer1, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=872,690
// GUItool: end automatically generated code

#include <Bounce.h>

Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);  // 15 = 15 ms debounce time
Bounce button2 = Bounce(2, 15);

// Exponential Moving Average for smoothing pot input (REF: Deepseek)
// Struct to store previous values for smoothing + change detection
struct SmoothPot{
    float emaAlpha = 0.1;  // Smoothing factor (0.1 = smoother, 0.5 = more responsive)
    float smoothedValue = 0;
    int lastValue = 0;
};

SmoothPot pots[3];
const int pins[3] = {A1, A2, A3};

int readSmoothPot(int pin, SmoothPot &pot) {
    int raw = analogRead(pin);
    pot.smoothedValue = pot.emaAlpha * raw + (1 - pot.emaAlpha) * pot.smoothedValue;
    return pot.smoothedValue;
}

void changeBitDepth(u_int val){
  bitcrusher1.bits(val);
  bitcrusher2.bits(val);
}

void changeSampleRate(u_int val){
  bitcrusher1.sampleRate(val);
  bitcrusher2.sampleRate(val);
}

void changeBitMix(float val){
  mixer1.gain(1, val);
  mixer2.gain(1, val);
}

void setup() {
  Serial.begin(9600); // Turn on serial monitoring for arduino IDE
  AudioMemory(60); // Allocate x 
  sgtl5000_1.enable(); // turn on audio codec chip
  sgtl5000_1.volume(0.32); // set output volume of chip

  // set button pins to pullup resist
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);

  mixer1.gain(0, 0);
  mixer1.gain(1, 1);
  mixer2.gain(0, 0);
  mixer2.gain(1, 1);

  changeBitDepth(6);
}

u_int knob1History;
bool bitcrushOn = true;
float lastVolumeValue = -1;
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

  float volumeValue = pow((float)potValue[0] / 1023.0, 4.0);
  if (abs(volumeValue - lastVolumeValue) > 0.02){
    changeBitMix(volumeValue);
    lastVolumeValue = volumeValue;
    Serial.printf("Bitcrush volume: %f\n", volumeValue);
  }

  // get bit depth
  int bitDepthValue = round(pow((float)potValue[1] / 1023.0, 4.0) * 12)+4;

  // change only if changed significantly
  if (bitDepthValue != pots[1].lastValue) {
      changeBitDepth(bitDepthValue);
      pots[1].lastValue = bitDepthValue;
      Serial.printf("Bitcrush bits: %d\n", bitDepthValue);
  }

    // get bit depth
  int samplerateValue = round(pow((float)potValue[2] / 1023.0, 6.0) * 43999) + 1;

  // change only if changed significantly
  if (abs(samplerateValue - pots[2].lastValue) > 150) {
      changeSampleRate(samplerateValue);
      pots[2].lastValue = samplerateValue;
      Serial.printf("Bitcrush sample rate: %d\n", samplerateValue);
  }

  if (button0.fallingEdge()){
    bitcrushOn = !bitcrushOn;
    if(bitcrushOn){
      changeBitDepth(bitDepthValue);
      changeSampleRate(samplerateValue);
      Serial.println("Bitcrush on!");
    } else {
      changeBitDepth(16);
      changeSampleRate(44000);
      Serial.println("Bitcrush off!");
    }
  }
}
