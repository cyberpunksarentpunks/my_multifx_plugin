#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <algorithm>

namespace atone
{
inline float clamp(float x, float lo, float hi) noexcept
{
    return juce::jlimit(lo, hi, x);
}

inline float dbToGain(float db) noexcept
{
    return juce::Decibels::decibelsToGain(db);
}

inline float tanhApprox(float x) noexcept
{
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline float softClip(float x) noexcept
{
    return tanhApprox(x);
}

inline float hardClip(float x, float t = 1.0f) noexcept
{
    return clamp(x, -t, t);
}

inline float cubicDrive(float x) noexcept
{
    const float c = clamp(x, -1.0f, 1.0f);
    return 1.5f * c - 0.5f * c * c * c;
}

struct OnePole
{
    void setLP(float cutoffHz, double sampleRate) noexcept
    {
        const float sr = (float) sampleRate;
        const float fc = clamp(cutoffHz, 10.0f, sr * 0.45f);
        const float x = std::exp(-juce::MathConstants<float>::twoPi * fc / sr);
        a = x;
        b = 1.0f - x;
    }

    void setHP(float cutoffHz, double sampleRate) noexcept
    {
        setLP(cutoffHz, sampleRate);
        highpass = true;
    }

    float processLP(float x) noexcept
    {
        z = b * x + a * z;
        return z;
    }

    float process(float x) noexcept
    {
        const float lp = processLP(x);
        return highpass ? (x - lp) : lp;
    }

    void reset() noexcept { z = 0.0f; }

    float a = 0.0f, b = 1.0f, z = 0.0f;
    bool highpass = false;
};

struct DCBlock
{
    float process(float x) noexcept
    {
        const float y = x - xm1 + 0.995f * ym1;
        xm1 = x;
        ym1 = y;
        return y;
    }

    void reset() noexcept { xm1 = ym1 = 0.0f; }

    float xm1 = 0.0f, ym1 = 0.0f;
};

struct PeakMeter
{
    void process(const juce::AudioBuffer<float>& buffer) noexcept
    {
        float peak = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const auto* d = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                peak = juce::jmax(peak, std::abs(d[i]));
        }

        const float prev = value.load(std::memory_order_relaxed);
        const float next = peak > prev ? peak : prev * 0.955f;
        value.store(next, std::memory_order_relaxed);
    }

    float get() const noexcept { return value.load(std::memory_order_relaxed); }
    std::atomic<float> value { 0.0f };
};

struct HermiteDelay
{
    void prepare(double sampleRate, float maxDelayMs)
    {
        sr = sampleRate;
        const int n = (int) std::ceil(sr * (maxDelayMs / 1000.0) * 1.1) + 8;
        buffer.assign((size_t) juce::jmax(n, 64), 0.0f);
        mask = 0;
        size = (int) buffer.size();
        write = 0;
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        write = 0;
    }

    void writeSample(float x) noexcept
    {
        buffer[(size_t) write] = x;
        write = (write + 1) % size;
    }

    float readMs(float delayMs) const noexcept
    {
        const float delaySamples = clamp(delayMs * 0.001f * (float) sr, 1.0f, (float) size - 4.0f);
        return readSamples(delaySamples);
    }

    float readSamples(float delaySamples) const noexcept
    {
        float readPos = (float) write - delaySamples;
        while (readPos < 0.0f)
            readPos += (float) size;

        const int i1 = (int) readPos;
        const float f = readPos - (float) i1;
        const int i0 = (i1 - 1 + size) % size;
        const int i2 = (i1 + 1) % size;
        const int i3 = (i1 + 2) % size;

        const float y0 = buffer[(size_t) i0];
        const float y1 = buffer[(size_t) i1];
        const float y2 = buffer[(size_t) i2];
        const float y3 = buffer[(size_t) i3];

        const float c0 = y1;
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + c0;
    }

    double sr = 44100.0;
    std::vector<float> buffer;
    int size = 0, write = 0, mask = 0;
};

struct PitchShifter
{
    void prepare(double sampleRate)
    {
        delay.prepare(sampleRate, 80.0f);
        grain = 0.04f * (float) sampleRate;
        ph1 = 0.0f;
        ph2 = grain * 0.5f;
    }

    void reset()
    {
        delay.reset();
        ph1 = 0.0f;
        ph2 = grain * 0.5f;
    }

    float process(float x, float semitones) noexcept
    {
        delay.writeSample(x);
        const float ratio = std::pow(2.0f, semitones / 12.0f);
        auto grainRead = [&](float& ph) noexcept
        {
            ph += ratio;
            if (ph >= grain)
                ph -= grain;
            const float w = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * (ph / grain));
            return delay.readSamples(ph) * w;
        };

        const float mix = grainRead(ph1) + grainRead(ph2);
        return mix;
    }

    HermiteDelay delay;
    float grain = 1024.0f, ph1 = 0.0f, ph2 = 0.0f;
};

struct SimpleLimiter
{
    void prepare(double sampleRate)
    {
        attack = std::exp(-1.0f / (0.001f * (float) sampleRate));
        release = std::exp(-1.0f / (0.080f * (float) sampleRate));
        env = 0.0f;
    }

    void process(juce::AudioBuffer<float>& buffer, float ceiling = 0.98f) noexcept
    {
        const int n = buffer.getNumSamples();
        const int chs = buffer.getNumChannels();
        for (int i = 0; i < n; ++i)
        {
            float peak = 0.0f;
            for (int c = 0; c < chs; ++c)
                peak = juce::jmax(peak, std::abs(buffer.getSample(c, i)));

            const float coeff = peak > env ? attack : release;
            env = peak + coeff * (env - peak);
            const float g = env > ceiling ? (ceiling / (env + 1.0e-8f)) : 1.0f;
            for (int c = 0; c < chs; ++c)
                buffer.setSample(c, i, buffer.getSample(c, i) * g);
        }
    }

    float attack = 0.0f, release = 0.0f, env = 0.0f;
};

inline void applyStereoWidth(juce::AudioBuffer<float>& buffer, float width) noexcept
{
    if (buffer.getNumChannels() < 2)
        return;

    auto* l = buffer.getWritePointer(0);
    auto* r = buffer.getWritePointer(1);
    const float w = clamp(width, 0.0f, 2.0f);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float mid = 0.5f * (l[i] + r[i]);
        const float side = 0.5f * (l[i] - r[i]) * w;
        l[i] = mid + side;
        r[i] = mid - side;
    }
}

inline float lfo(float phase, int shape) noexcept
{
    switch (shape)
    {
        case 1: return phase < 0.5f ? (phase * 4.0f - 1.0f) : (3.0f - phase * 4.0f);
        case 2: return phase < 0.5f ? -1.0f : 1.0f;
        default: return std::sin(juce::MathConstants<float>::twoPi * phase);
    }
}
} // namespace atone
