/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 2012-12-21
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "Time.hpp"
#include "TimerRegistry.hpp"

#include <boost/timer/timer.hpp>

/**
 * \brief Helper class for measuring time.
 */
class Timer
{
public:

	Timer (bool autostart = false);
	virtual ~Timer ();

	void start ();

	const CPUTime elapsed () const;

	const CPUTime stop ();

	void pause ();

	void resume ();

	void registerTime(Components) const;

	const static Timer total;


private:

	boost::timer::cpu_timer timer;
	static TimerRegistry time_registry;


};

#endif /* TIMER_H_ */
