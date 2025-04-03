#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=388,514
AudioMixer4              mixer1;         //xy=526,489
AudioEffectFreeverbStereo freeverbs1;     //xy=658,491
AudioMixer4              mixer3;         //xy=800,466
AudioMixer4              mixer2;         //xy=801,540
AudioAmplifier           amp2;           //xy=933,527
AudioAmplifier           amp1;           //xy=934,492
AudioOutputI2S           i2s1;           //xy=1077,514
AudioConnection          patchCord1(usb1, 0, mixer1, 0);
AudioConnection          patchCord2(usb1, 0, mixer3, 0);
AudioConnection          patchCord3(usb1, 1, mixer1, 1);
AudioConnection          patchCord4(usb1, 1, mixer2, 0);
AudioConnection          patchCord5(mixer1, freeverbs1);
AudioConnection          patchCord6(freeverbs1, 0, mixer3, 1);
AudioConnection          patchCord7(freeverbs1, 1, mixer2, 1);
AudioConnection          patchCord8(mixer3, amp1);
AudioConnection          patchCord9(mixer2, amp2);
AudioConnection          patchCord10(amp2, 0, i2s1, 1);
AudioConnection          patchCord11(amp1, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=872,690
// GUItool: end automatically generated code



#include <Bounce.h>

Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);  // 15 = 15 ms debounce time
Bounce button2 = Bounce(2, 15);

// Sets the output volume of the reverbs
void setReverbAmt(float val){
  mixer2.gain(1, val);
  mixer3.gain(1, val);
}

void setOutputVolume(float val){
  amp1.gain(val);
  amp2.gain(val);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Turn on serial monitoring for arduino IDE
  AudioMemory(60); // Allocate x 
  sgtl5000_1.enable(); // turn on audio codec chip
  sgtl5000_1.volume(0.32); // set output volume of chip

  // set button pins to pullup resist
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);

  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);

  freeverbs1.roomsize(0.5);
  freeverbs1.damping(0.5);

  mixer2.gain(0,1);
  mixer3.gain(0,1);
}

bool reverbOn = true;
float knob1History, knob2History, knob3History;
const float knobError = 0.025;

void loop() {
  // get button values
  button0.update();
  button1.update();
  button2.update();
  // get knob values - power of 4 makes log taper pots behave linearly
  float knob1 = pow((float)analogRead(A1) / 1023.0, 4.0);
  float knob2 = pow((float)analogRead(A2) / 1023.0, 4.0);
  float knob3 = pow((float)analogRead(A3) / 1023.0, 4.0);

  // when button 0 is pressed
  if (button0.fallingEdge()){
    reverbOn = !reverbOn;
    if (reverbOn){
      setReverbAmt(knob2);
      Serial.println("Reverb enabled!");
    } else {
      setReverbAmt(0);
      Serial.println("Reverb disabled!");
    }
  }

  // Print knob value if significant change is detected
  if (knob1 >= knob1History + knobError || knob1 <= knob1History - knobError){
    knob1History = knob1;
    Serial.printf("Output volume: %f\n", knob1);
  }
  setOutputVolume(knob1);

  if(reverbOn){
    if (knob2 >= knob2History + knobError || knob2 <= knob2History - knobError){
      knob2History = knob2;
      Serial.printf("Reverb amount: %f\n", knob2);
      setReverbAmt(knob2);
    }
  }

  if (knob3 >= knob3History + knobError || knob3 <= knob3History - knobError){
    knob3History = knob3;
    Serial.printf("Reverb roomsize: %f\n", knob3);
  }
  freeverbs1.roomsize(knob3);
}
