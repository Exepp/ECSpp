#include <ECSpp/Utility/Time.h>
#include <thread>

using namespace epp;


Clock::Clock()
{
    last = nowChr();
}

float Clock::reset()
{
    return DTDuration_t(resetChr()).count();
}

float Clock::time() const
{
    return timeChr().count();
}

Clock::Duration_t Clock::resetChr()
{
    auto       now = nowChr();
    Duration_t dt  = now - last;
    last           = now;
    return dt;
}

Clock::Duration_t Clock::timeChr() const
{
    return nowChr() - last;
}

Clock::TimePoint_t Clock::nowChr() const
{
    return Clock_t::now();
}

Clock::TimePoint_t const& Clock::lastChr() const
{
    return last;
}


ConstTickClock::ConstTickClock(DeltaT_t timestep, Ticks_t maxCatchUpTicks)
    : timestep(Duration_t::rep(timestep * DeltaT_t(1e9)))
    , maxTicks(maxCatchUpTicks)
    , clock()
    , deltaTime(0)
    , timeOffset(0)
    , ticksToCatchUp(0)
{
}

ConstTickClock::Ticks_t ConstTickClock::update()
{
    TimePoint_t now           = clock.nowChr();
    Duration_t  sleepDuration = timestep - (now - clock.last) - timeOffset;

    if (sleepDuration > Duration_t(0))
    {
        std::this_thread::sleep_for(sleepDuration);
        now = clock.nowChr();
    }

    deltaTime  = (now - clock.lastChr());
    clock.last = now;

    timeOffset += deltaTime - timestep;
    ticksToCatchUp = std::min(int32_t(timeOffset / timestep), maxTicks);
    timeOffset -= ticksToCatchUp * timestep;

    return getTicksToCatchUp();
}

void ConstTickClock::setTimeStep(DeltaT_t dt)
{
    timestep = Duration_t(Duration_t::rep(dt * DeltaT_t(1e9)));
}

void ConstTickClock::setMaxCatchUpTicks(Ticks_t ticks)
{
    maxTicks = ticks;
}

ConstTickClock::Ticks_t ConstTickClock::getTicksToCatchUp() const
{
    return ticksToCatchUp;
}

ConstTickClock::DeltaT_t ConstTickClock::getDT() const
{
    static float dt = std::chrono::duration<float>(timestep).count();
    return dt;
}