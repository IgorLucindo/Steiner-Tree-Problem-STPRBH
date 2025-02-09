/*
 * STPRBHSolver.h
 */

#ifndef STPRBHSOLVER_H_
#define STPRBHSOLVER_H_

#include "instance/STPInstance.hpp"
#include "instance/DiGraph.h"
#include "instance/Solution.h"
#include "heuristics/STPRBHHeurCallback.h"
#include <ilcplex/ilocplex.h>
#include <boost/thread/shared_mutex.hpp>

using namespace std;

ILOSTLBEGIN

/**
 * \brief The main solver class. The magic happens here.
 */
class STPRBHSolver
{
	friend class STPRBHLazy;
	friend class STPRBHUser;
	friend class STPRBHHeurCallback;
	friend class STPRBHIncCallback;

	stringstream solf;
	stringstream statf;
	stringstream dimacsf;

	boost::shared_mutex incMutex;

	IloEnv env;
	IloModel myModel;
	IloCplex cplex;

	STPInstance* inst;
	DiGraph* G;

	Solution* incumbent;
	Solution* lazySol;

	IloCplex::Callback lazy;
	IloCplex::Callback user;
	IloCplex::Callback heur;

	int nNodes;
	int nArcs;
	int nTerminals;

	int nodeID;


	IloNumVarArray x;
	IloNumVarArray y;
	IloNumArray x_lp;
	IloNumArray y_lp;

	double timeBest;
	double rootBound;

	static constexpr double epsInt=1e-03;
	static constexpr double epsOpt=1e-06;

	double oldObj;
	bool initialized = false;
	bool lazyImproved;
	double lbOld ;
	double lbCurrent ;

	int nHoplink;
	int nHopend;
	int nNodearccut;
	int nFlowbalance;
	int nHopflowbalance;
	int nOtahoplink;
	int nEtahoplink;
	int nCcuts;
	int nGhoplink;
	double bestBound;


	bool solveBaC();
	bool setCPLEXParameters();
	bool solveHeur();
	bool solveLPRelaxation();

	void createVariables();
	void createNACoupling();
	void createHopCoupling();
	void createEthalink();
	void createOthalink();
	void createHopLink();
	void createHopEnd();
	void createRootOutgoing();
	void createRootLayerOneHopCoupling();
	void createBudget();
	void createFlowConservation();
	void createGSECSizeTwo();
	void createObjective();


	void addMIPStart(Solution* s, const char* name = nullptr);
	void addPoolCuts();
	void writeSolution();
	void setSolution();
	void setStatistics();
	void prepareDIMACSOutputEnd();
	void prepareDIMACSOutputBegin();
	void createModel();


public:
	STPRBHSolver(STPInstance* _inst);
	virtual ~STPRBHSolver();

	bool solve();
	Solution getSolution();

	string solutionString()
	{
		return solf.str();
	}

	string statisticString()
	{
		return statf.str();
	}

	string dimacsString()
	{
		return dimacsf.str();
	}

};

#endif /* STPRBHSOLVER_H_ */
