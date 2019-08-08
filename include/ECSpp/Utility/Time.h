#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>

namespace epp
{

class Clock
{
    using Clock_t      = std::chrono::high_resolution_clock;
    using TimePoint_t  = Clock_t::time_point;
    using Duration_t   = TimePoint_t::duration;
    using DTDuration_t = std::chrono::duration<float>;

public:
    Clock();

    float reset();
    float time() const;

    Duration_t resetChr();
    Duration_t timeChr() const;

    TimePoint_t        nowChr() const;
    TimePoint_t const& lastChr() const;

private:
    TimePoint_t last;

    friend class ConstTickClock;
};


class ConstTickClock
{
    using TimePoint_t = Clock::TimePoint_t;
    using Duration_t  = Clock::Duration_t;
    using Ticks_t     = int32_t;
    using DeltaT_t    = float;

public:
    ConstTickClock(DeltaT_t timestep = 1.f / 60.f, Ticks_t maxCatchUpTicks = 3);

    Ticks_t update();

    void setTimeStep(DeltaT_t dt);
    void setMaxCatchUpTicks(Ticks_t ticks);

    Ticks_t  getTicksToCatchUp() const;
    DeltaT_t getDT() const;

private:
    Duration_t timestep;
    Ticks_t    maxTicks;

    Clock      clock;
    Duration_t deltaTime;
    Duration_t timeOffset;
    Ticks_t    ticksToCatchUp;
};

} // namespace epp
#endif // CLOCK_H