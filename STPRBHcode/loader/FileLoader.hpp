/**
 * \file
 * \brief
 *
 */

#ifndef FILELOADER_HPP_
#define FILELOADER_HPP_

#include <utility/ProgramOptions.h>
#include <instance/STPInstance.hpp>
#include "loader/FileFormatException.hpp"
#include "loader/STPLoader.hpp"
#include "utility/Logger.hpp"
#include <boost/algorithm/string.hpp>
#include <string>

/**
 * \brief FileLoader class.
 */
class FileLoader
{
public:
	static STPInstance readFile ();

};

#endif /* FILELOADER_HPP_ */
