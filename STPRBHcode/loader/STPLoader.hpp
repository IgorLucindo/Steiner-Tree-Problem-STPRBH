/**
 * \file
 * \brief Definition of the STP File Reader
 *
 * \author Max Resch
 * \date 2012-10-13
 */

#ifndef  STPLOADER_H_
#define  STPLOADER_H_

#include <instance/STPInstance.hpp>

#include <string>
#include <fstream>

/**
 * \brief Class encapsulating a static file reader for STP Files
 */
class STPLoader
{
public:
	/**
	 * \brief Read the given file in STP format and generate a OGDF Graph object.
	 */
	static STPInstance readFile(std::ifstream& file);

private:

	inline static bool isInteger(double val)
	{
		return (val - (long) val) < 1e-10;
	}
};

#endif // STPLOADER_H_
