/*
 * STPRBHSepFlow.h
 */

#ifndef STPRBHSEPFLOW_H_
#define STPRBHSEPFLOW_H_

#include <ilcplex/ilocplex.h>
#include <vector>

#include "instance/DiGraph.h"
#include "Maxflow.hpp"

using namespace std;

/**
 * \brief Contains separation routines for fractional solutions. Mainly similar to STPRBHSepCon, aside from
 * tolerances and the separation for the connectivity cut constraints (done with max-flow here).
 */
class STPRBHSepFlow
{
	DiGraph* G;

	int nNodes;
	int nArcs;
	int nTerminals;

	int nHoplink;
	int nHopend;
	int nNodearccut;
	int nFlowbalance;
	int nHopflowbalance;
	int nOtahoplink;
	int nEtahoplink;
	int nCcuts;
	int nGhoplink;

	IloEnv env;
	IloNumVarArray x;
	IloNumVarArray y;

	HashArray2D<int, node, double> hyval;
	IloNumArray x_lp;
	IloNumArray y_lp;

	List<node> Terminals;

	EdgeArray<bool> addedCons;

	// maxflow data
	Maxflow* myFlow;
	list<pair<u_int, u_int>> arcs;
	double* capacities;
	int* cut;

	struct CoverElements
	{
		CoverElements(edge _e, double w):e(_e),weight(w) {};
	    edge e;
	    double weight;
	    bool operator<(const CoverElements& a) const
		{
			return weight < a.weight;
		}

	};

	vector<IloRange> myCuts;
	vector<IloRange> myPurgeableCuts;

	double epsInt;
	double epsCreep;
	double epsOpt;
	double threshold;

	double incObj;

	int separateGHopLink();
	int separateCut();
	int separateHopLink();
	int separateHopEnd();
	int separateOddTwoHopLink();
	int separateEvenTwoHopLink();
	int separateNodeArcCut();
	int separateFlowBalance();
	int separateCover();
	int separateFlowBalanceHop();

	void inline initialize();

public:
	STPRBHSepFlow(DiGraph* _G, IloEnv _env, IloNumVarArray _x, IloNumVarArray _y, bool integer=false);
	STPRBHSepFlow(const STPRBHSepFlow&);
	virtual ~STPRBHSepFlow();

	vector<IloRange> getCuts();
	vector<IloRange> getPurgeableCuts();
	int separation(const IloNumArray& x_lp,const IloNumArray& y_lp, double incObj, HashArray2D<int, node, double> hyval);

	inline int getNHoplink() const
	{
		return nHoplink;
	}

	inline int getNHopend() const
	{
		return nHopend;
	}

	inline int getNNodearccut() const
	{
		return nNodearccut;
	}

	inline int getNFlowbalance() const
	{
		return nHopflowbalance;
	}

	inline int getNHopflowbalance() const
	{
		return nHopflowbalance;
	}

	inline int getNOtahoplink() const
	{
		return nOtahoplink;
	}

	inline int getNEtahoplink() const
	{
		return nEtahoplink;
	}

	inline int getNCcuts() const
	{
		return nCcuts;
	}

	inline int getNGhoplink() const
	{
		return nGhoplink;
	}


};

#endif /* STPRBHSEPFLOW_H_ */
