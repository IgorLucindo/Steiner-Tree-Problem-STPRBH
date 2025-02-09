/*
 * ProgramOptions.h
 *
 */

#ifndef PROGRAMOPTIONS_H_
#define PROGRAMOPTIONS_H_

#include <string>
#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <array>
#include <fstream>
#include <istream>
#include <ostream>
#include <utility/Logger.hpp>

using namespace std;

namespace po = boost::program_options;

enum MIPAlgorithm
{
	ALG_Primal,
	ALG_Dual,
	ALG_Sifting,
	ALG_Barrier,
	ALG_Concurrent,
	ALG_Network,
	ALG_Auto
};

std::istream& operator>> (std::istream& in, MIPAlgorithm& alg);
std::ostream& operator<< (std::ostream& out, const MIPAlgorithm& alg);


std::istream& operator>> (std::istream& in, LOG_LEVEL& lvl);
std::ostream& operator<< (std::ostream& out, const LOG_LEVEL& lvl);

/**
 *  \brief Class for handling the program options
 */
class ProgramOptions {

public:


	struct ParametersT
	{

		/// Input file to read
		std::string file;

		/// Output file to write
		std::string outputfile;
		/// Output file to write
		std::string outputfile2;
		/// Output file to write
		std::string outputfile3;
		/// Output file to write
		std::string outputfile4;

		bool valid = false;

		int seed = 0;


		//bool explicitGSEC=true;


		int sepflow = 2;
		int sepflowhop = 2;
		int sepnodearccut = 1;
		int sepotahoplink = 2;
		int sepetahoplink = 2;
		int sephoplink = 1;
		int sepghoplink = 2;
		int sepcover = 2;
		int sepcut = 1;
		int sephopend=1;


		///if true, gsecs are statically added
		bool sepgsecsizetwo=true;

		///use forwardcuts?
		bool forwardcuts=false;

		///use backcuts?
		bool backcuts = true;
		///usercut callback active?
		bool usercut = true;

		///use cplex cuts?
		int cplexcuts = 1;


		/// separate only if value of varibale on "rhs" is greater than this
		double septhreshold=1e-3;

		//only solve the root in branch-and-cut
		bool onlyroot = false;

		// "true" LP-relaxation per handmade cutting plane
		bool lprelaxation=false;

		/// Set Cardinality Cuts, set to 0 to switch it off
		/// This value is added in UserCuts to the weights
		double creepflow = 1E-5;

		/// The maximum number of iterations in a User/Lazy Callback,
		/// where the algorithm tries to find new cuts
		int nestedcuts = 250;

		/// Separation Tailoff Parameter
		double septailoff=1e-5;




		/// Number of CPLEX threads, 0 means we will use thread::hardware_concurrency()
		int threads = 1;

		///Use heuristics
		bool heurisitc = true;

		///Which weights to use in the heuristic: 0: only information from x, 1: also try to use y^h
		int heurweights = 0;

		bool writesol = false;

		/// how talkative should CPLEX be
		int cplexoutput = 3;

		/// write the models to file
		bool cplexexportmodels=false;

		/// Set CPLEX root solving algorithm, see CPLEX documentation
		MIPAlgorithm rootAlgorithm = ALG_Dual;

		/// Set CPLEX node solving algorithm, see CPLEX documentation
		MIPAlgorithm nodeAlgorithm = ALG_Dual;

		/// Set the CPLEX time limit in seconds, if the time runs out, CPLEX will abort
		long timelimit = 1000;

		/// Relative CPLEX gap, default is 1E-5, if the gap between LP bound and IP solution is smaller than
		/// this, CPLEX will abort
		double gap = 1E-5;

	};
    typedef struct ParametersT Parameters;

    ProgramOptions(int &argc, char ** &argv);
    virtual ~ProgramOptions();

};

enum Components
{
	C_Loader = 1,
	C_MIP_UCallback = 2,
	C_MIP_LCallback = 3,
	C_MIP_HCallback = 4,
	C_MIPModel = 5,
	C_CPLEX = 6,
	C_INITIALIZATION = 7,
};

/**
 * \brief Helper class for measuring time.
 */
template <typename T> class ComponentArray : public std::array<T, 8>
{
};

extern ProgramOptions::Parameters params;

#endif /* PROGRAMOPTIONS_H_ */
