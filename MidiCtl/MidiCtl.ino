#include <Bounce.h>

#define channel 1

Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);  // 15 = 15 ms debounce time
Bounce button2 = Bounce(2, 15);

// Exponential Moving Average for smoothing pot input (REF: Deepseek)
// Struct to store previous values for smoothing + change detection
struct SmoothPot{
    float emaAlpha = 0.1;  // Smoothing factor (0.1 = smoother, 0.5 = more responsive)
    float smoothedValue = 0;
    int lastMidiValue = 0;
};

SmoothPot pots[3];

int readSmoothPot(int pin, SmoothPot &pot) {
    int raw = analogRead(pin);
    pot.smoothedValue = pot.emaAlpha * raw + (1 - pot.emaAlpha) * pot.smoothedValue;
    return pot.smoothedValue;
}

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Set button pins to input with pullup resistors
    pinMode(0, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
}

int knob1History, knob2History, knob3History;
const float knobError = 0.025;

const int midiCC[3] = {1, 2, 3};
const int pins[3] = {A1, A2, A3};

void loop()
{
    // Update button states
    button0.update();
    button1.update();
    button2.update();

    // button 0: C
    if (button0.fallingEdge()) {
        usbMIDI.sendNoteOn(72, 127, 1);  // Note 60, velocity 127, channel 1
        digitalWrite(LED_BUILTIN, HIGH); // Visual feedback
        //Serial.println("Pressed!");
    }
    if (button0.risingEdge()) {
        usbMIDI.sendNoteOff(72, 0, 1);   // Note 60, velocity 0, channel 1
        digitalWrite(LED_BUILTIN, LOW);  // Visual feedback
        //Serial.println("Released!");
    }

    // button 1: D
    if (button1.fallingEdge()) {
        usbMIDI.sendNoteOn(74, 127, 1);  // Note 60, velocity 127, channel 1
        digitalWrite(LED_BUILTIN, HIGH); // Visual feedback
        //Serial.println("Pressed!");
    }
    if (button1.risingEdge()) {
        usbMIDI.sendNoteOff(74, 0, 1);   // Note 60, velocity 0, channel 1
        digitalWrite(LED_BUILTIN, LOW);  // Visual feedback
        //Serial.println("Released!");
    }

    // button 3: E
    if (button2.fallingEdge()) {
        usbMIDI.sendNoteOn(76, 127, 1);  // Note 60, velocity 127, channel 1
        digitalWrite(LED_BUILTIN, HIGH); // Visual feedback
        //Serial.println("Pressed!");
    }
    if (button2.risingEdge()) {
        usbMIDI.sendNoteOff(76, 0, 1);   // Note 60, velocity 0, channel 1
        digitalWrite(LED_BUILTIN, LOW);  // Visual feedback
        //Serial.println("Released!");
    }

    for (int i = 0; i < 3; i++) {
        // Read smoothed value (0–1023)
        int potValue = readSmoothPot(pins[i], pots[i]);

        // Apply your non-linear curve and map to MIDI (0–127)
        int midiValue = (int)(pow((float)potValue / 1023.0, 4.0) * 127);

        // Send MIDI CC only if changed significantly
        if (midiValue != pots[i].lastMidiValue) {
            usbMIDI.sendControlChange(midiCC[i], midiValue, 1);  // Ch. 1
            pots[i].lastMidiValue = midiValue;
            //Serial.printf("MIDI CC%d: %d\n", midiCC[i], midiValue);
        }
    }

    // MIDI Controllers should discard incoming MIDI messages.
    while (usbMIDI.read()) {
        // Ignore incoming messages
    }
    
    // Small delay to prevent overwhelming the USB MIDI
    //delay(1);
}