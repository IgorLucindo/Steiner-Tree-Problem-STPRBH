/**
 * \file
 * \brief
 *
 */

#include "loader/FileLoader.hpp"

#include <fstream>
#include <string>

using namespace std;
using namespace boost;

STPInstance FileLoader::readFile ()
{
	ifstream file(params.file);
	if (!file.good())
		throw FileFormatException("Error opening input file", params.file, 0);

	string buffer;
	u_int line = 0;
	do
	{
		getline(file, buffer);
		line++;
	}
	while (buffer.empty() || buffer.front() == '#');

	if (starts_with(buffer, "33D32945") || starts_with(buffer, "33d32945"))
	{
		LOG(info) << "Detected STPRBH File" << endl;
		file.seekg(0);
		return STPLoader::readFile(file);
	}
	else
	{
		LOG(info) << params.file << ": Unknown File Signature: " << buffer << endl;
		throw FileFormatException("Unknown File Signature", params.file, line);
	}
}

