/**
 * \file
 * \brief Implement Methods to handle SteinLib STP files
 *
 * \author Max Resch
 * \date 2012-10-12
 */

#include "STPLoader.hpp"
#include "FileFormatException.hpp"
#include "utility/ProgramOptions.h"

//#include <utility/Constructionparamseters.hpp>
#include <utility/Logger.hpp>

#include <ogdf/basic/Graph.h>
#include <algorithm>
#include <functional>
#include <string>
#include <libgen.h>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cctype>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace ogdf;

STPInstance STPLoader::readFile(std::ifstream& is)
{
	enum Section {
		COMMENT = 1,
		GRAPH = 2,
		TERMINALS = 3,
		NA = 0,
		END = 5,
	};

	char buffer[1024];
	enum Section section = NA;
	enum Section nextSection = COMMENT;
	char value[1024];
	char key[1024];
	long scannedElements;
	id n = 0;
	id v1, v2;
	long line = 0;
	double v3 = 0;
	weight w0 = 0;
	id i = 1;
	vector<node> indexToNode;
	bzero(buffer, 1024);
	bzero(value, 1024);
	bzero(key, 1024);

	STPInstance inst;
	inst.isInteger = true;

	line++;
	is.getline(buffer, 1024);
	if (strncasecmp(buffer, "33d32945", 8) != 0) {
		is.close();
		throw FileFormatException("Invalid File Signature", params.file, line);
	}

	while (!is.eof()) {
		line++;
		is.getline(buffer, 1024);

		if (buffer[0] == '#' || strlen(buffer) == 0 || buffer[0] == '\n' || buffer[0] == '\r')
			continue;

		// I hate Windows' cr-lf endings so much
		if (buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		switch (section) {
		case NA:
			if (!strncasecmp(buffer, "SECTION Comment", 15) && nextSection == COMMENT)
				section = COMMENT;
//			else if (!strncasecmp(buffer, "Section Comment", 15) && nextSection == COMMENT)
//					section = COMMENT;
			else if (!strncasecmp(buffer, "SECTION Graph", 13) && nextSection == GRAPH)
				section = GRAPH;
			else if (!strncasecmp(buffer, "SECTION ProfitableVertices", 26) && nextSection == TERMINALS)
				section = TERMINALS;
			else if (!strncasecmp(buffer, "EOF", 3) && (nextSection == END)) {
				if (inst.Description.empty())
					inst.Description = boost::filesystem::path(params.file).filename().string();
				inst.Root = inst.Terminals.nodes().front();
				// MX replaced basename
				inst.Name = boost::filesystem::path(params.file).stem().string();
				is.close();
				return inst;
			}
			break;
		case COMMENT:
			sscanf(buffer, "%1024s %1024c", key, value);
			if (strcmp(key, "Name") == 0) {
				//value[strlen(value) - 1] = '\0';
				inst.Description = value;
				boost::trim_if(inst.Description, boost::is_space() || boost::is_punct());
			} else if (!strcasecmp(key, "Date")) {
				inst.Date = value;
				boost::trim_if(inst.Description, boost::is_space() || boost::is_punct());
			} else if (!strcasecmp(key, "Creator")) {
				//creator = value;
			} else if (!strcasecmp(key, "Remark")) {
				//remark = value;
			} else if (!strcasecmp(key, "END")) {
				nextSection = GRAPH;
				section = NA;
			} else {
				// throw std::runtime_error("Invalid Comment Section Element");
				LOG(info) << "Unknown Comment Section Element: " << key << endl;
			}
			break;
		case GRAPH:
			// graph section
			v1 = v2 = 0;
			v3 = 0.0L;
			scannedElements = sscanf(buffer, "%1024s %ld %ld %lf", key, &v1, &v2, &v3);
			switch (scannedElements) {
			case 1: // END
				if (!strcasecmp(key, "END")) {
					nextSection = TERMINALS;
					section = NA;
				} else {
					is.close();
					throw FileFormatException("Invalid Graph Section Element", params.file, line);
				}
				break;
			case 2: //Number of nodes, edges or arcs
				if (!strcasecmp(key, "Nodes")) {
					n = static_cast<id>(v1);
					indexToNode.resize(n + 1);
					for (id i = 1; i <= n; i++) {
						node v;
						v = indexToNode[i] = inst.G.newNode();
						inst.Prize[v] = 0;
						inst.NodeId[v] = i;
					}
				} if (!strcasecmp(key, "Root")) {
					if (v1 > n) {
						is.close();
						throw std::runtime_error("Invalid Root Definition");
					}
					inst.Root = indexToNode[v1];
				}
				else if (!strcasecmp(key, "Budget")) {
					inst.budget = v1;
				}
				else if (!strcasecmp(key, "HopLimit")) {
					inst.hoplimit = v1;
				}
				break;
			case 4: // specific edge or arc
				switch (buffer[0]) {
				case 'E':
				case 'A':
					if (v1 > n || v2 > n || v1 < 1 || v2 < 1) {
						is.close();
						throw FileFormatException("Invalid Edge Definition", params.file, line);
					}
					w0 = static_cast<weight>(v3);
					edge e;
					e = inst.G.newEdge(indexToNode[v1], indexToNode[v2]);
					inst.Weight[e] = w0;
					inst.EdgeId[e] = i++;
					if (!isInteger(w0))
						inst.isInteger = false;
					break;
				default:
					is.close();
					throw FileFormatException("Invalid Graph Section Element", params.file, line);
				}
				;
				break;
			default:
				is.close();
				throw FileFormatException("Invalid Graph Section Element", params.file, line);
			}
			break;
		case TERMINALS:
			// terminals section
			v1 = 0;
			v3 = 0;
			sscanf(buffer, "%1024s %ld %lf", key, &v1, &v3);
			if (!strcasecmp(key, "Terminals")) {
				//inst.NumberOfTerminals = v1; // set number of terminals
			}
			else if (!strcasecmp(key, "PV")) {
				if (v1 > n) {
					is.close();
					throw std::runtime_error("Invalid Terminal Definition");
				}

				w0 = static_cast<weight>(v3);
				inst.Prize[indexToNode[v1]] = w0;
				inst.Terminals.insert(indexToNode[v1]);
			}

			else if (!strcasecmp(key, "END")) {
				nextSection = END;
				section = NA;
			}
			// no else: ignore unused keys
			break;

		default:
			assert(true);
			break;
		}
	}

	is.close();
	throw FileFormatException("EOF mark is missing", params.file, line);
}
