/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 21.04.2013
 */

#ifndef TIME_HPP_
#define TIME_HPP_

#include <boost/timer/timer.hpp>
#include <chrono>
#include <ostream>

typedef std::chrono::duration<double, std::ratio<1>> seconds;

/**
 * \brief Helper struct for measuring time.
 */
struct CPUTime
{
	typedef std::chrono::nanoseconds _time;

	CPUTime ();
	CPUTime (const boost::timer::cpu_times&);

	_time wall;
	_time user;
	_time sys;

	CPUTime& operator+= (const CPUTime&);
	CPUTime operator+ (const CPUTime&) const;
	CPUTime operator- (const CPUTime&) const;

	double getSeconds() const;
};

std::ostream& operator<< (std::ostream& os, const CPUTime t);

#endif /* TIME_HPP_ */
