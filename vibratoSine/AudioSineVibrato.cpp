#include <Arduino.h>
#include "AudioSineVibrato.h"
#include "dspinst.h"

extern "C" {
extern const int16_t AudioWaveformSine[257];
}

void AudioSineVibrato::update(void)
{
    audio_block_t *block;
    uint32_t i, ph, inc, index, scale, incPM, vibPh, vibInc;
    int32_t val1, val2;

    if (magnitude) {
        block = allocate();
        if (block) {
            ph = phase_accumulator;
            inc = phase_increment;
            vibPh = vibPhaseAccumulator;
            vibInc = vibPhaseIncrement;

            for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
                // 1. Calculate LFO output (Q31: -1.0 to +1.0)
                index = vibPh >> 24;
                val1 = AudioWaveformSine[index];
                val2 = AudioWaveformSine[index + 1];
                scale = (vibPh >> 8) & 0xFFFF;
                
                // Linear interpolation (Q15 to Q31)
                val2 *= scale;
                val1 *= 0x10000 - scale;
                int32_t lfo_raw = val1 + val2;

                // 2. Apply vibrato depth (in Hz units)
                int32_t freq_offset = (int32_t)((int64_t)lfo_raw * vibDepth >> 31);
                
                // 3. Modulate phase increment (with underflow protection)
                int32_t modulated_inc = (int32_t)inc + freq_offset;
                if (modulated_inc < 0) modulated_inc = 0;  // Clamp negative
                incPM = (uint32_t)modulated_inc;

                // 4. Generate output sine wave
                index = ph >> 24;
                val1 = AudioWaveformSine[index];
                val2 = AudioWaveformSine[index + 1];
                scale = (ph >> 8) & 0xFFFF;
                val2 *= scale;
                val1 *= 0x10000 - scale;

                // 5. Output with magnitude scaling
                #if defined(__ARM_ARCH_7EM__)
                    block->data[i] = multiply_32x32_rshift32(val1 + val2, magnitude);
                #elif defined(KINETISL)
                    block->data[i] = (((val1 + val2) >> 16) * magnitude) >> 16;
                #endif

                // 6. Update phases
                ph += incPM;
                vibPh += vibInc;
            }

            phase_accumulator = ph;
            vibPhaseAccumulator = vibPh;
            transmit(block);
            release(block);
            return;
        }
    }
    phase_accumulator += phase_increment * AUDIO_BLOCK_SAMPLES;
}