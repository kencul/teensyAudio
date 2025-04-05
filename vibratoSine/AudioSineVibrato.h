#ifndef AudioSineVibrato_h_
#define AudioSineVibrato_h_

#include <Arduino.h>     // github.com/PaulStoffregen/cores/blob/master/teensy4/Arduino.h
#include <AudioStream.h> // github.com/PaulStoffregen/cores/blob/master/teensy4/AudioStream.h
#include <arm_math.h>    // github.com/PaulStoffregen/cores/blob/master/teensy4/arm_math.h

class AudioSineVibrato : public AudioStream
{
public:
    AudioSineVibrato() : AudioStream(0, NULL), vibPhaseAccumulator(0), magnitude(16384)
    {
        vibFrequency(6);
        vibratoDepth(1);
    }
    
    /// @brief changes frequency of base sine wave
    /// @param freq frequency of base sine wave
    void frequency(float freq)
    {
        if (freq < 0.0f)
            freq = 0.0;
        else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2.0f)
            freq = AUDIO_SAMPLE_RATE_EXACT / 2.0f;
        phase_increment = freq * (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT);
    }

    /// @brief sets frequency of vibrato LFO
    /// @param freq frequency of LFO
    void vibFrequency(float freq)
    {
        if (freq < 0.0f)
            freq = 0.0;
        else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2.0f)
            freq = AUDIO_SAMPLE_RATE_EXACT / 2.0f;
        vibPhaseIncrement = freq * (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT);
    }

    /// @brief sets phase of sine wave
    /// @param angle angle of phase in degrees
    void phase(float angle)
    {
        if (angle < 0.0f)
            angle = 0.0f;
        else if (angle > 360.0f)
        {
            angle = angle - 360.0f;
            if (angle >= 360.0f)
                return;
        }
        phase_accumulator = angle * (float)(4294967296.0 / 360.0);
    }

    /// @brief sets depth of vibrato
    /// @param freq frequency depth of vibrato
    void vibratoDepth(float freq)
    {
        // Convert Hz to phase increment offset
        vibDepth = freq * (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT);
    }

    /// @brief sets amplitude of base sine wave
    /// @param n amplitude scale of synth
    void amplitude(float n)
    {
        if (n < 0.0f)
            n = 0;
        else if (n > 1.0f)
            n = 1.0f;
        magnitude = n * 65536.0f;
    }

    virtual void update(void);

private:
    uint32_t phase_accumulator;
    uint32_t phase_increment;

    uint32_t vibPhaseAccumulator;
    uint32_t vibPhaseIncrement;
    uint32_t vibDepth;
    int32_t magnitude;
};

#endif
