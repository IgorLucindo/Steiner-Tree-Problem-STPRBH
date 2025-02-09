/*
 * STPHeuristics.h
 */


#ifndef STPRBHHEURISTICS_H_
#define STPRBHHEURISTICS_H_

#include "instance/DiGraph.h"
#include "instance/Solution.h"

/**
 *  \brief Class for the implementation of the heuristic.
 */
class STPRBHHeuristics
{
	DiGraph* G;
	int nNodes;
	int nArcs;

	Graph sol;

	NodeArray<node> myNodes;
	NodeArray<node> myNodesBack ;
	EdgeArray<edge> myEdges;
	EdgeArray<edge> myEdgeBack;

public:
	STPRBHHeuristics(DiGraph* G);
	virtual ~STPRBHHeuristics();

	double PrimI(Solution* inc);

	double PrimSTPRBH(Solution* inc, EdgeArray<double> weight, NodeArray<double> nodeWeights, HashArray2D<int, node, double> hyval, double ub=-1);

};

#endif /* STPRBHHEURISTICS_H_ */
