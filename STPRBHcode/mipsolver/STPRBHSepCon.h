/*
 * STPRBHSepCon.h
 */

#ifndef STPRBHSepCon_H_
#define STPRBHSepCon_H_

#include <ilcplex/ilocplex.h>
#include <vector>

#include "instance/DiGraph.h"
#include "utility/ProgramOptions.h"

using namespace std;


/**
 * \brief Contains separation routines for integer solutions. Mainly similar to STPRBHSepFlow, aside from
 * tolerances and the separation for the connectivity cut constraints (done with connected components here).
 */
class STPRBHSepCon
{
	DiGraph* G;
	Graph sepG;
	vector<ogdf::node> myNodes;
	vector<ogdf::edge> myArcs;

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

	NodeArray<node> nodesGToSep;
	NodeArray<node> nodesSepToG;
	EdgeArray<edge> arcsGToSep;
	EdgeArray<edge> arcsSepToG;

	NodeArray<int> components;

	IloEnv env;
	IloNumVarArray x;
	IloNumVarArray y;

	HashArray2D<int, node, double> hyval;
	IloNumArray x_lp;
	IloNumArray y_lp;

	vector<IloRange> myCuts;
	vector<IloRange> myPurgeableCuts;

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

	static constexpr double threshold=0.5;

public:
	STPRBHSepCon(DiGraph* _G, IloEnv _env, IloNumVarArray _x, IloNumVarArray _y, bool integer);
	STPRBHSepCon(const STPRBHSepCon&);

	virtual ~STPRBHSepCon();

	int separation(IloNumArray x_lp, IloNumArray y_lp, double incObj, HashArray2D<int, node, double> hyval);

	vector<IloRange> getCuts();
	vector<IloRange> getPurgeableCuts();

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

#endif /* STPRBHSepCon_H_ */
