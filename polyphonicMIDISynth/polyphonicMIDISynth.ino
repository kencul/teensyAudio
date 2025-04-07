#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioSynthWaveformSine   sine[4];
AudioEffectEnvelope      env[4];
AudioAmplifier           amp[4];
AudioMixer4              mix;
AudioOutputI2S           i2s1;

// Connections
AudioConnection          c1(sine[0], 0, env[0], 0);
AudioConnection          c2(env[0], 0, mix, 0);
AudioConnection          c3(sine[1], 0, env[1], 0);
AudioConnection          c4(env[1], 0, mix, 1);
AudioConnection          c5(sine[2], 0, env[2], 0);
AudioConnection          c6(env[2], 0, mix, 2);
AudioConnection          c7(sine[3], 0, env[3], 0);
AudioConnection          c8(env[3], 0, mix, 3);
AudioConnection          c9(mix, 0, i2s1, 0);
AudioConnection          c10(mix, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

const u_int numVoices = 4;
bool voicePlaying[4] = {false};
u_int voiceNote[4] = {0};
unsigned long voiceTime[4] = {0};
u_int numKeysPressed = 0;

void OnNoteOn(byte channel, byte note, byte velocity) {
  digitalWrite(LED_BUILTIN, HIGH); // Any Note-On turns on LED
  numKeysPressed++;
  
  // find a free voice
  int freeVoice = -1;
  for (u_int i = 0; i < 4; i++){
    if(!voicePlaying[i]){
      freeVoice = i;
      break;
    }
  }
  
  // if no voices are free, find oldest voice
  if(freeVoice == -1){
    unsigned long oldestTime = voiceTime[0];
    freeVoice = 0;
    for(u_int i = 1; i < 4; i++){
      if(voiceTime[i] < oldestTime){
        oldestTime = voiceTime[i];
        freeVoice = i;
      }
    }
  }

  voicePlaying[freeVoice] = true;
  voiceNote[freeVoice] = note;
  voiceTime[freeVoice] = millis();

  float freq = 440.0 * pow(2.0, (note - 69) / 12.0);
  float amp = velocity / 127.0;

  sine[freeVoice].amplitude(amp);
  sine[freeVoice].frequency(freq);
  env[freeVoice].noteOn();
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  numKeysPressed--;

  // find voice with the note number
  int voiceNum = -1;
  for(u_int i = 0; i < 4; i++){
    if (voiceNote[i] == note){
      voiceNum = i;
      break;
    }
  }

  // return early if matching voice number isn't found
  if (voiceNum == -1){
    return;
  }

  voicePlaying[voiceNum] = false;
  env[voiceNum].noteOff();

  if (numKeysPressed == 0){
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void onControlChange(byte channel, byte control, byte value){
  if(control == 21){
    int scaledValue = value/127.0 * 999 + 1;
    for(u_int i = 0; i < 4; i++){
      env[i].attack(scaledValue);
    }
    Serial.printf("Attack: %dms\n", scaledValue);
  }
  if(control == 22){
    int scaledValue = value/127.0 * 999 + 1;
    for(u_int i = 0; i < 4; i++){
      env[i].decay(scaledValue);
    }
    Serial.printf("Decay: %dms\n", scaledValue);
  }
  if(control == 23){
    float scaledValue = value/127.0;
    for(u_int i = 0; i < 4; i++){
      env[i].sustain(scaledValue);
    }
    Serial.printf("Sustain: %f\n", scaledValue);
  }
  if(control == 24){
    int scaledValue = value/127.0 * 999 + 1;
    for(u_int i = 0; i < 4; i++){
      env[i].release(scaledValue);
    }
    Serial.printf("Release: %dms\n", scaledValue);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  AudioMemory(20); // Allocate x 
  sgtl5000_1.enable(); // turn on audio codec chip
  sgtl5000_1.volume(0.32); // set output volume of chip

  // prepare built in LED
  pinMode(LED_BUILTIN, OUTPUT);

  // adding callback functions for midi note on and off
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(onControlChange);

  mix.gain(0, 0.25);  // Balance mixer levels (4 voices = 0.25 each)
  mix.gain(1, 0.25);
  mix.gain(2, 0.25);
  mix.gain(3, 0.25);
}

void loop() {
  usbMIDI.read();
  // Periodically print CPU/memory usage
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
      lastPrint = millis();
      Serial.printf("CPU: %.1f%%, Mem: %d/%d\n", 
                    AudioProcessorUsage(), 
                    AudioMemoryUsage(), 
                    AudioMemoryUsageMax());
  }
}
