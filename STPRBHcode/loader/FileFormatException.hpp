/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 21.04.2013
 */

#ifndef FILEFORMATEXCEPTION_HPP_
#define FILEFORMATEXCEPTION_HPP_

#include <stdexcept>
#include <string>


/**
 * \brief For error handling.
 */
class FileFormatException : public std::runtime_error
{
public:
	FileFormatException (const std::string, const std::string file, const int line);
	FileFormatException (const FileFormatException& _copy);

	virtual const char* what () const noexcept override;

private:
	const int line;
	const std::string text;
	const std::string file;
};

#endif /* FILEFORMATEXCEPTION_HPP_ */
