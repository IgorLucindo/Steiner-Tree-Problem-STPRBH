/*
 * STPRBHHeurCallback.h
 */

#ifndef STPRBHHeurCallback_H_
#define STPRBHHeurCallback_H_

#include <ilcplex/ilocplex.h>

class STPRBHSolver;

#include "mipsolver/STPRBHSolver.h"
#include "heuristics/STPRBHHeuristics.h"
#include "instance/DiGraph.h"
#include "instance/Solution.h"

/**
* \brief Class for the heuristic-callback of CPLEX. Note that Implementation of the heuristic is in class STPRBHHeuristics.
*/
class STPRBHHeurCallback : public IloCplex::HeuristicCallbackI
{
	void main();
	void callHeur();

	IloEnv env;

	IloNumVarArray x;
	IloNumVarArray y;
	STPRBHSolver* solver;
	DiGraph* G;
	STPRBHHeuristics* heur;
	Solution* inc;

	int nNodesRoot;
	int nNodes;
	int nArcs;
	int nArcsRoot;
	int nTerminals;

public:
	IloCplex::CallbackI* duplicateCallback() const
	{
		return (new (getEnv()) STPRBHHeurCallback(*this));
	}

	STPRBHHeurCallback(IloEnv env, STPRBHSolver *_solver);
	STPRBHHeurCallback(const STPRBHHeurCallback& _copy);

	inline void initialize();

	virtual ~STPRBHHeurCallback();
};

#endif /* STPRBHHeurCallback_H_ */
