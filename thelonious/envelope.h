#ifndef THELONIOUS_ENVELOPE_H
#define THELONIOUS_ENVELOPE_H

#include <initializer_list>
#include <vector>
#include <algorithm>

#include "types.h"
#include "sizes.h"
#include "rates.h"
#include "unit.h"
#include "parameter.h"
#include "util.h"

namespace thelonious {
class EnvelopeSegment {
public:
    virtual Sample get(Sample startValue, Sample endValue, Sample position) {
        // Default to linear segments
        return linearInterpolate(startValue, endValue, position);
    }
};

// typedef allows us to create LinearSegments semantically
typedef EnvelopeSegment LinearSegment;

template <size_t N>
class EnvelopeN : public Unit<N> {
public:
    EnvelopeN(Sample initialValue,
             std::initializer_list<Sample> values,
             std::initializer_list<Sample> durations) :
        gate(0.0f, NONE), values(values), durations(durations),
        segments(values.size()), value(initialValue), time(0.0f),
        segmentIndex(0u), playing(false), canTrigger(true) {
    }

    void tick(Block<N> &block) {
        Chock gateChock = gate.get();
        for (uint32_t i=0; i<BLOCK_SIZE; i++) {
            if (gateChock[i] && canTrigger) {
                firstSegment();
                playing = true;
                canTrigger = false;
            }

            if (!gateChock[i]) {
                canTrigger = true;
            }

            while (playing && time > duration) {
                nextSegment();
            }

            if (playing) {
                value = segments[segmentIndex].get(startValue, endValue,
                                                   time * invDuration);
                time += INV_SAMPLE_RATE;
            }

            block[0][i] = value;
        }
    }

    Parameter gate;

private:
    void setSegmentVariables() {
        duration = durations[segmentIndex];
        invDuration = 1.0f / duration;
        startValue = value;
        endValue = values[segmentIndex];
    }

    void firstSegment() {
        segmentIndex = 0;
        setSegmentVariables();
        time = 0;
    }

    void nextSegment() {
        segmentIndex++;

        if (segmentIndex == segments.size()) {
            playing = false;
        }
        else {
            time -= duration;
            setSegmentVariables();
        }
    }

    std::vector<Sample> values;
    std::vector<Sample> durations;
    std::vector<EnvelopeSegment> segments;

    Sample value;
    Sample time;

    Sample duration;
    Sample invDuration;
    Sample startValue;
    Sample endValue;

    uint32_t segmentIndex;
    bool playing;
    bool canTrigger;
};

typedef EnvelopeN<1> Envelope;

}
#endif
