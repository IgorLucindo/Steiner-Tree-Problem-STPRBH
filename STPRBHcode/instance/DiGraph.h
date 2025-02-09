/*
 * Digraph.h
 *
 *  Created on: Mar 25, 2014
 *      Author: markus
 */

#ifndef DIGRAPH_H_
#define DIGRAPH_H_

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/BinaryHeap2.h>
#include <ogdf/basic/HashArray.h>
#include <ogdf/basic/HashArray2D.h>
#include <ogdf/basic/NodeSet.h>
#include <ilcplex/ilocplex.h>


using namespace ogdf;
using namespace std;


/**
 * \brief Contains the instance graph transformed as digraph, and some additional stuff related to it.
 */
class DiGraph
{

	HashArray2D<node, node, edge> myArcMap; ///< to get the opposite edge fast

public:
	DiGraph();
	virtual ~DiGraph();


	Graph G;

	vector<edge> myArcs;
	vector<node> myNodes;

	int rootID;

	double sum_rev;
	double budget;
	int hoplimit;

	double maxArcWeight;

	EdgeArray<double> myArcWeights;
	NodeArray<double> myNodeWeights;

	vector<node> myTerminals;
	NodeArray<bool> myNodeTerminal;
	NodeSet Terminals; ///< NodeSet allows easy permutation for separation
	node rootNode;

	void init();

	NodeArray<int> layer; ///< first layer a node can be
	NodeArray<int> layerEnd; ///< last layer a node can be

	EdgeArray<IloNumVar> xvar; ///< arc variables
	NodeArray<IloNumVar> yvar; ///< node variables
	HashArray2D<int, node,IloNumVar> hyvar; ///< hopnode variables
	EdgeArray<int> x_id; ///< id of a given edge, should be the same as e->index()
	NodeArray<int> y_id; ///< id of a given node, should be the same as n->index()


	/**
	 * \brief set root node, create myArcMap, calculate sum of all revenues
	 */
	void finalize(node root);

	edge addEdge(unsigned src_id, unsigned dst_id, double e_cost);
	edge addEdge(node src_id, node  dst_id, double e_cost);

	node addNode(unsigned nId, int oldID, double nRev, bool term, int layerStart, int layerEnd);

	edge getOppositeArc(edge e);
	
	HashArray<int, node> mapper; ///< for mapping ID to node when creating the Graph
	NodeArray<int> backMapper; ///< for backmapping to original ID after preprocessing

};

#endif /* DIGRAPH_H_ */
