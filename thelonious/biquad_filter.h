#ifndef THELONIOUS_BIQUAD_FILTER_H
#define THELONIOUS_BIQUAD_FILTER_H

#include "types.h"
#include "sizes.h"
#include "unit.h"
#include "parameter.h"

namespace thelonious {

template <size_t N>
class BiquadFilter : public Unit<N> {
private:
    typedef struct {
        Sample a0;
        Sample a1;
        Sample a2;
        Sample b0;
        Sample b1;
        Sample b2;
    } Coefficients;

    typedef struct {
        Sample x1;
        Sample x2;
        Sample y1;
        Sample y2;
    } Samples;

public:
    BiquadFilter(Sample frequency, Sample damping) :
        frequency(frequency), damping(damping) {}

    void tick(Block<N> &block) {
        Chock frequencyChuck = frequency.get();
        Chock dampingChuck = damping.get();
        for (uint32_t i=0; i<BLOCK_SIZE; i++) {
            Sample frequency = frequencyChuck[i];
            Sample damping = dampingChuck[i];

            if (frequency != lastFrequency || damping != lastDamping) {
                calculateCoefficients(frequency, damping);
                lastFrequency = frequency;
                lastDamping = damping;
            }

            Sample invA0 = 1.0 / coefficients.a0;

            for (uint32_t j=0; j<N; j++) {
                Sample output = coefficients.b0 * invA0 * block[j][i] +
                                coefficients.b1 * invA0 * samples[j].x1 +
                                coefficients.b2 * invA0 * samples[j].x2 -
                                coefficients.a1 * invA0 * samples[j].y1 -
                                coefficients.a2 * invA0 * samples[j].y2;

                samples[j].x2 = samples[j].x1;
                samples[j].x1 = block[j][i];
                samples[j].y2 = samples[j].y1;
                samples[j].y1 = output;

                block[j][i] = output;
            }
        }
    }

    Parameter frequency;
    Parameter damping;

protected:
    Coefficients coefficients;

private:
    virtual void calculateCoefficients(Sample frequency) = 0;

    std::array<Samples, N> samples;
    Sample lastFrequency;
    Sample lastDamping;
};

}

#endif
