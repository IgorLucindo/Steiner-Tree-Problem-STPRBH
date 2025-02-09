/**
 * \file
 * \brief Main File, calls instance reader, solver, etc
 *
 * \author Markus Sinnl
 */

#include <boost/program_options.hpp>

#include <utility/Logger.hpp>
#include <utility/Timer.hpp>
#include <utility/TimerRegistry.hpp>
#include <loader/FileLoader.hpp>
#include <loader/FileFormatException.hpp>
#include <instance/STPInstance.hpp>

#include <mipsolver/STPRBHSolver.h>
#include <instance/Solution.h>

using namespace std;
namespace po = boost::program_options;

void printParameters (STPInstance* inst)
{
	cout << endl;
	cout << "--------------------------------------" << endl;
	cout << "name     \t" << setw(10) << inst->Name << endl;
	cout << "nodes    \t" << setw(10)  << inst->G.numberOfNodes() << endl;
	cout << "edges    \t" << setw(10) << inst->G.numberOfEdges() << endl;
	cout << "term     \t" << setw(10) << inst->Terminals.size() << endl;
	cout << "hoplimit \t" << setw(10) << inst->hoplimit << endl;
	cout << "budget   \t" << setw(10) << inst->budget << endl;
	cout << "--------------------------------------" << endl;
	cout << endl;
}

int main (int argc, char* argv[])
{
	Timer load(true);

	ProgramOptions po(argc, argv);

	if (!params.valid) {
		return 1;
	}

	if (params.seed == -1) {
		srand(time(NULL));
	} else {
		srand(params.seed);
	}

	LOG(debug) << "Debug Log Enabled" << endl;

	try {

		STPInstance in = FileLoader::readFile();

		STPInstance* inst = &(in);

		printParameters(inst);

		inst->valid();

		load.stop();
		load.registerTime(C_Loader);

		inst->createDiGraphs(inst->Root);

		Solution solution(&(inst->basicG));

		STPRBHSolver solver(inst);
		solver.solve();
		solution = solver.getSolution();

		CPUTime lpSolveTime = TimerRegistry::get(C_CPLEX) - TimerRegistry::get(C_MIP_HCallback) - TimerRegistry::get(C_MIP_LCallback) - TimerRegistry::get(C_MIP_UCallback);
		CPUTime totalTime = TimerRegistry::get(C_INITIALIZATION)+ TimerRegistry::get(C_CPLEX) + TimerRegistry::get(C_Loader);

		//cerr<<

		cout << endl;
		cout << "-------------------------------------------------" << endl;
		cout << "feasible \t\t"<< setw(10)  << solution.isFeasible() << endl;
		cout << "objective \t\t"<< setw(10)  << solution.getObjective() << endl;
		cout << "used budget \t\t"<< setw(10)  << solution.getBudget() << endl;
		cout << "hops \t\t\t"<< setw(10)  << solution.getHops() << endl;
		cout << "-------------------------------------------------" << endl;
		cout << endl;
		cout << "load+preprocess\t\t" << setprecision(3) << setw(10) << TimerRegistry::get(C_Loader) << "s" << endl;
		cout << "init model\t\t" << setprecision(3) << setw(10) << TimerRegistry::get(C_INITIALIZATION) << "s" << endl;
		cout << "user-cb\t\t\t" << setprecision(3) << setw(10) << TimerRegistry::get(C_MIP_UCallback) << "s" << endl;
		cout << "lazy-cb\t\t\t" << setprecision(3) << setw(10) << TimerRegistry::get(C_MIP_LCallback) << "s" << endl;
		cout << "heur-cb\t\t\t" << setprecision(3) << setw(10) << TimerRegistry::get(C_MIP_HCallback) << "s" << endl;
		cout << "lpsolve\t\t\t" << setprecision(3) << setw(10) << lpSolveTime << "s" << endl;
		cout << "B&C\t\t\t" << setprecision(3) << setw(10) << TimerRegistry::get(C_CPLEX) << "s" << endl;
		cout << "total\t\t\t" << setprecision(3) << setw(10) << totalTime << "s" << endl;
		cout << "-------------------------------------------------" << endl;

		try {
			if (!params.outputfile.empty())
			{
				ofstream os(params.outputfile);
				os<<solver.statisticString()<<endl;
				os.close();
			}
		} catch (exception& e) {
			cout << "Could not write solution to file: " << params.outputfile << endl;
		}

		try {
			if (!params.outputfile2.empty())
			{
				ofstream os(params.outputfile2);
				os<<solver.solutionString()<<endl;
				os.close();
			}
		} catch (exception& e) {
			cout << "Could not write Auxiliary to file: " << params.outputfile3 << endl;
		}

		try {
			if (!params.outputfile4.empty())
			{
				ofstream os(params.outputfile4);
				os<<solver.dimacsString();
				os.close();
			}
		} catch (exception& e) {
			cout << "Could not write Auxiliary to file: " << params.outputfile4 << endl;
		}

	} catch (FileFormatException& e) {
		cout << "Could not read instance file:" << e.what() << endl;
	}

	return 0;
}
