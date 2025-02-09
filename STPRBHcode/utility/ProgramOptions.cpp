/*
 * ProgramOptions.cpp
 *
 */

#include "ProgramOptions.h"

#include <iostream>
#include <sstream>

long seed = -1;

const string CONFIGURATION_FILENAME = "parameters.ini";

ProgramOptions::Parameters params;

/// Parse strings for parameter input
std::istream& operator>> (std::istream& in, MIPAlgorithm& alg)
{
	std::string token;
	in >> token;
	if (token == "primal")
		alg = ALG_Primal;
	else if (token == "dual")
		alg = ALG_Dual;
	else if (token == "sifting")
		alg = ALG_Sifting;
	else if (token == "barrier")
		alg = ALG_Barrier;
	else if (token == "concurrent")
		alg = ALG_Concurrent;
	else if (token == "network")
		alg = ALG_Network;
	else if (token == "auto")
		alg = ALG_Auto;
	else
		throw std::invalid_argument("invalid MIP algorithm, valid choices are: primal, dual, sifting, barrier, concurrent, network, auto");
	return in;
}

/// Parse strings for parameter input
std::ostream& operator<< (std::ostream& out, const MIPAlgorithm& alg)
{
	if (alg == ALG_Primal)
		out << "primal";
	else if (alg == ALG_Dual)
		out << "dual";
	else if (alg == ALG_Sifting)
		out << "sifting";
	else if (alg == ALG_Barrier)
		out << "barrier";
	else if (alg == ALG_Concurrent)
		out << "concurrent";
	else if (alg == ALG_Network)
		out << "network";
	else if (alg == ALG_Auto)
		out << "auto";
	else
		throw std::invalid_argument("invalid MIP algorithm");
	return out;
}

/// Parse strings for parameter input
std::istream& operator >> (std::istream& in, LOG_LEVEL& lvl)
{
	std::string token;
	in >> token;
	if (token == "debug")
		lvl = debug;
	else if (token == "info")
		lvl = info;
	else if (token == "warn")
		lvl = warn;
	else if (token == "severe")
		lvl = severe;
	else if (token == "critical")
		lvl = critical;
	else
		throw std::invalid_argument("invalid loglevel, valid choices are: critical, severe, warn, info, debug");
	return in;
}

/// Parse strings for parameter input
std::ostream& operator << (std::ostream& out, const LOG_LEVEL& lvl)
{
	if (lvl == debug)
		out << "debug";
	else if (lvl == info)
		out << "info";
	else if (lvl == warn)
		out << "warn";
	else if (lvl == severe)
		out << "severe";
	else if (lvl == critical)
		out << "critical";
	else
		throw std::invalid_argument("invalid loglevel");
	return out;
}


ProgramOptions::ProgramOptions(int &argc, char ** &argv) {

	po::options_description general_options("General options");
			general_options.add_options()
					("help,h", "produce help message")
					("file,f", po::value<string>(&params.file), "instance file to process")
					("outstat,o", po::value<string>(&params.outputfile), "statistics file to write")
					("outsol", po::value<string>(&params.outputfile2), "solution file to write")
					("outbest", po::value<string>(&params.outputfile3), "best solution file to write")
					("outdimacs", po::value<string>(&params.outputfile4), "dimcas solution file to write")
					("seed", po::value<int>(&params.seed)->implicit_value(-1)->default_value(params.seed), "random seed, set to -1 to use TIME")
					("verbosity,v", po::value<LOG_LEVEL>(&GLOBAL_LOG_LEVEL)->implicit_value(info)->default_value(GLOBAL_LOG_LEVEL), "verbosity of logmessages, can be debug, info, warn, error, critical")
					;

			po::positional_options_description descPos;
			descPos.add("file", 1);
			descPos.add("output", 2);

			po::options_description mip_options("Options controlling behavior of CPLEX / MIP Model");
			mip_options.add_options()
					("cplex.timelimit,t", po::value<long>(&params.timelimit)->default_value(params.timelimit), "set the CPLEX time limit in seconds, if the time runs out, CPLEX will abort")
					("cplex.threads,T", po::value<int>(&params.threads)->default_value(1), "set the number of CPLEX threads, set to 0 to use thread::hardware_concurrency()")
					("cplex.gap", po::value<double>(&params.gap)->default_value(1E-6, "1E-6"), "set relative optimality gap for CPLEX, if the gap between LP bound and IP solution is smaller than this, CPLEX will abort")
					("cplex.rootalgorithm", po::value<MIPAlgorithm>(&params.rootAlgorithm)->default_value(params.rootAlgorithm), "set CPLEX root solving algorithm")
					("cplex.nodealgorithm", po::value<MIPAlgorithm>(&params.nodeAlgorithm)->default_value(params.nodeAlgorithm), "set CPLEX node solving algorithm")
					("cplex.exportmodel", po::value<bool>(&params.cplexexportmodels)->implicit_value(false)->default_value(params.cplexexportmodels), "export cplex models")
					("cplex.onlyroot", po::value<bool>(&params.onlyroot)->implicit_value(false)->default_value(params.onlyroot), "only root computation")
					("cplex.lprelaxation", po::value<bool>(&params.lprelaxation)->implicit_value(false)->default_value(params.lprelaxation), "consider LP-relaxation only")
					("cplex.output", po::value<int>(&params.cplexoutput)->implicit_value(3)->default_value(params.cplexoutput), "cplex output")
					("cplex.writesol", po::value<bool>(&params.writesol)->implicit_value(false)->default_value(params.writesol), "write cplex solutions")
					("cplex.usercut", po::value<bool>(&params.usercut)->implicit_value(false)->default_value(params.usercut), "user cut callback?")
					("cplex.cplexcuts", po::value<int>(&params.cplexcuts)->default_value(params.cplexcuts), "cplexcuts")
					;
			po::options_description sep_options("Separation settings [0=static, if not exponential number][1=yes][2=do not use]");
			sep_options.add_options()
					("sep.flow", po::value<int>(&params.sepflow)->implicit_value(0)->default_value(params.sepflow),  "flow conversation")
					("sep.flowhop", po::value<int>(&params.sepflowhop)->implicit_value(2)->default_value(params.sepflowhop), "hop flow conservation")
					("sep.nodearccut", po::value<int>(&params.sepnodearccut)->implicit_value(1)->default_value(params.sepnodearccut), "node arc")
					("sep.otahoplink", po::value<int>(&params.sepotahoplink)->implicit_value(1)->default_value(params.sepotahoplink), "odd two arc hop link")
					("sep.etahoplink", po::value<int>(&params.sepetahoplink)->implicit_value(1)->default_value(params.sepetahoplink), "odd two arc hop link")
					("sep.hoplink", po::value<int>(&params.sephoplink)->implicit_value(0)->default_value(params.sephoplink), "hop link")
					("sep.ghoplink", po::value<int>(&params.sepghoplink)->implicit_value(2)->default_value(params.sepghoplink), "generalized hop link")
					("sep.cover", po::value<int>(&params.sepcover)->implicit_value(2)->default_value(params.sepcover), "cover")
					("sep.hopend", po::value<int>(&params.sephopend)->implicit_value(0)->default_value(params.sephopend), "hop end")
					("sep.gsecsizetwo", po::value<bool>(&params.sepgsecsizetwo)->implicit_value(true)->default_value(params.sepgsecsizetwo), "static gsec size two true/false")
					("sep.cut", po::value<int>(&params.sepcut)->implicit_value(1)->default_value(params.sepcut), "cut")
					("sep.threshold", po::value<double>(&params.septhreshold)->default_value(params.septhreshold), "seperation threshold")
					("sep.tailoff", po::value<double>(&params.septailoff)->default_value(params.septailoff), "seperation tailoff")
					("sep.cut.mincardeps", po::value<double>(&params.creepflow)->default_value(1E-5, "1E-5"), "sets min cardinality cut epsilon value in usercuts, this is added to the edge weights, set to 0 to disable this feature")
					("sep.nestedcuts", po::value<int>(&params.nestedcuts)->default_value(params.nestedcuts), "sets maximum iteration count for nested cuts, set 0 to disable nested cuts, -1 to add infinitely many, or until no more cuts are found")
					("sep.backcuts", po::value<bool>(&params.backcuts)->implicit_value(true)->default_value(params.backcuts), "enable/disable backcuts in the directed MIP model")
					("sep.forwardcuts", po::value<bool>(&params.forwardcuts)->implicit_value(true)->default_value(params.forwardcuts), "enable/disable forwardcuts in the directed MIP model")
					;
			po::options_description heur_options("Heuristic settings");
			heur_options.add_options()
					("heur.heuristic",po::value<bool>(&params.heurisitc)->implicit_value(true)->default_value(params.heurisitc), "enable/disable heuristic callback and heuristic MIPstart")
					("heur.heurweights", po::value<int>(&params.heurweights)->implicit_value(0)->default_value(params.heurweights), "weights in heuristic: 0-xlp values, 1-also y^h influence")
			;

			po::options_description all("Allowed parameters");
			all.add(general_options).add(mip_options).add(sep_options).add(heur_options);

			po::variables_map vm;
			ifstream is(CONFIGURATION_FILENAME);
			po::store(po::command_line_parser(argc, argv).options(all).positional(descPos).run(), vm);
			if (is.good())
				po::store(po::parse_config_file(is, all), vm);
			is.close();
			po::notify(vm);

			if (vm.count("help"))
			{
				cout << all << endl;
				exit(0);
			}
			else if (!vm.count("file"))
			{
				cout << "No input file given!" << endl;
				exit(0);
			}

			params.valid = true;
}

ProgramOptions::~ProgramOptions() {

}
