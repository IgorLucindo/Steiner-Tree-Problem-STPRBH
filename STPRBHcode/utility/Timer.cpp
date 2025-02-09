/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 2012-12-21
 */

#include "Timer.hpp"
#include "TimerRegistry.hpp"

using namespace std;
using namespace boost::timer;

TimerRegistry Timer::time_registry;

const Timer Timer::total(true);

void Timer::start ()
{
	timer.start();
}

const CPUTime Timer::stop ()
{
	timer.stop();
	return CPUTime(timer.elapsed());
}

const CPUTime Timer::elapsed() const
{
	return CPUTime(timer.elapsed());
}

void Timer::pause()
{
	timer.stop();
}

void Timer::registerTime(Components c) const
{
	TimerRegistry::append(c, elapsed());
}

void Timer::resume()
{
	timer.resume();
}

Timer::Timer (bool autostart)
{
	if (autostart)
		timer.start();
}

Timer::~Timer ()
{
}
