/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 25.02.2014
 */
#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <fstream>
#include <iostream>
#include <iomanip>

enum LOG_LEVEL
{
		critical = 0,
		severe = 1,
		warn = 2,
		info = 3,
		debug = 4
};

extern LOG_LEVEL GLOBAL_LOG_LEVEL;

namespace
{
std::ofstream __nullstream;
}


#ifdef MX_FLAG // MX is use this to see were my messages came from, but I didn't want to bother the rest with this infromation
#define LOG(lvl) ((lvl <= GLOBAL_LOG_LEVEL) ? std::cout << " " << __PRETTY_FUNCTION__ << " line " << __LINE__ << ": " : __nullstream)
#else
#define LOG(lvl) ((lvl <= GLOBAL_LOG_LEVEL) ? std::cout : __nullstream)
#endif

#endif /* LOGGER_HPP_ */
