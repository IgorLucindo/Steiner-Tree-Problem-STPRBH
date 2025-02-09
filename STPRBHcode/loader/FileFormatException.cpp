/**
 * \file
 * \brief
 *
 * \author Max Resch
 * \date 29.10.2013
 */

#include "FileFormatException.hpp"
#include <sstream>

using namespace std;

FileFormatException::FileFormatException (const string _text, const string _file, const int _line) :
			runtime_error(_text),
			text(_text),
			line(_line),
			file(_file)
{
}

FileFormatException::FileFormatException (const FileFormatException& _copy) :
			runtime_error(_copy),
			text(_copy.text),
			line(_copy.line),
			file(_copy.file)
{
}

const char* FileFormatException::what () const noexcept
{
	std::stringstream s;
	s << file << " in line " << line << ": " << text;
	return s.str().c_str();
}

