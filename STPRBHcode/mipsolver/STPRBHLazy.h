/*
 * STPRBHLazy.h
 */

#ifndef STPRBHLAZY_H_
#define STPRBHLAZY_H_

#include <ilcplex/ilocplex.h>

class STPRBHSolver;

#include "STPRBHSolver.h"
#include "instance/DiGraph.h"
#include "STPRBHSepCon.h"
#include <heuristics/STPRBHHeuristics.h>

/**
 * \brief Class for the lazy-cut-callback of CPLEX, which will get called for integer solutions
 * Note that Separation will be done in class STPRBHSepCon
 */
class STPRBHLazy: public IloCplex::LazyConstraintCallbackI
{
	int nNodes;
	int nArcs;
	int nTerminals;

	IloEnv env;

	IloNumVarArray x;
	IloNumVarArray y;
	DiGraph* G;
	STPRBHSolver* solver;
	STPRBHHeuristics* heur;

	STPRBHSepCon* separatorCon;

	double incObj;

public:
	IloCplex::CallbackI* duplicateCallback() const
	{
		return (new (getEnv()) STPRBHLazy(*this));
	}
	STPRBHLazy(IloEnv env, STPRBHSolver *_solver);
	STPRBHLazy(const STPRBHLazy& _copy);


	inline void initialize();


	virtual ~STPRBHLazy();
	void main();
};



#endif /* STPRBHLazy_H_ */
