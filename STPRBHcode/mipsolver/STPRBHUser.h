/*
 * STPRBHUser.h
 */

#ifndef STPRBHUser_H_
#define STPRBHUser_H_

#include <ilcplex/ilocplex.h>

class STPRBHSolver;

#include "STPRBHSolver.h"
#include "instance/DiGraph.h"
#include "STPRBHSepFlow.h"

/**
 * \brief Class for the user-cut-callback of CPLEX, which will get called for fractional solutions.
 * Note that Separation will be done in class STPRBHSepFlow.
 */
class STPRBHUser: public IloCplex::UserCutCallbackI
{
	int nNodes;
	int nArcs;
	int nTerminals;


	double incObj;

	IloEnv env;

	IloNumVarArray x;
	IloNumVarArray y;
	DiGraph* G;

	STPRBHSolver* solver;
	STPRBHSepFlow* separator;

public:
	IloCplex::CallbackI* duplicateCallback() const
	{
		return (new (getEnv()) STPRBHUser(*this));
	}
	STPRBHUser(IloEnv env, STPRBHSolver *solver);
	STPRBHUser(const STPRBHUser& _copy);


	inline void initialize();

	virtual ~STPRBHUser();
	void main();
};



#endif /* STPRBHUser_H_ */
