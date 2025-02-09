/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 21.04.2013
 */

#include "Time.hpp"

CPUTime::CPUTime () :
			wall(),
			user(),
			sys()
{
}

CPUTime::CPUTime (const boost::timer::cpu_times& t) :
			wall(t.wall),
			user(t.user),
			sys(t.system)
{
}

CPUTime& CPUTime::operator+= (const CPUTime& rhs)
{
	wall += rhs.wall;
	user += rhs.user;
	sys += rhs.sys;
	return *this;
}

CPUTime CPUTime::operator+ (const CPUTime& rhs) const
{
	CPUTime t;
	t.wall = wall + rhs.wall;
	t.user = user + rhs.user;
	t.sys = sys + rhs.sys;
	return t;
}

CPUTime CPUTime::operator- (const CPUTime& rhs) const
{
	CPUTime t;
	t.wall = wall - rhs.wall;
	t.user = user - rhs.user;
	t.sys = sys - rhs.sys;
	return t;
}

double CPUTime::getSeconds () const
{
	//user + sys
	return std::chrono::duration_cast<seconds>(wall).count();
}

std::ostream& operator<< (std::ostream& os, const CPUTime t)
{
	os << t.getSeconds();
	return os;
}
