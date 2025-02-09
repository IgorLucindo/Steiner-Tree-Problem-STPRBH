/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 18.04.2013
 */

#include "TimerRegistry.hpp"

ComponentArray<CPUTime> TimerRegistry::storage;

const CPUTime TimerRegistry::get (const Components c)
{
	return storage[c];
}

void TimerRegistry::append (const Components c, CPUTime t)
{
	storage[c] = storage[c] + t;
}
