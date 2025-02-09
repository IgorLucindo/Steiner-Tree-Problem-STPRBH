/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 2013-04-18
 */

#ifndef TIMERREGISTRY_HPP_
#define TIMERREGISTRY_HPP_

#include "ProgramOptions.h"
#include "Time.hpp"

/**
 * \brief Helper class for measuring time.
 */
class TimerRegistry
{
public:

	static const CPUTime get (const Components);

private:
	friend class Timer;

	static void append (const Components, CPUTime);

	static ComponentArray<CPUTime> storage;

};

#endif /* TIMERREGISTRY_HPP_ */
