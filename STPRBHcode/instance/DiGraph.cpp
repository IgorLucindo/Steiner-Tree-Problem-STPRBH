/*
 * Digraph.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: markus
 */

#include "instance/DiGraph.h"

DiGraph::DiGraph()
	: Terminals(G),sum_rev(0),rootID(0),budget(0), hoplimit(0), maxArcWeight(0),
	  myNodeWeights(G), myArcWeights(G),  myNodeTerminal(G), backMapper(G,-1),
	  rootNode(nullptr),
	  xvar(G), yvar(G), hyvar (HashArray2D<int, node,IloNumVar>()),
	  x_id(G), y_id(G),
	  myArcMap(HashArray2D<node, node, edge>()),
	  layer(G, -1), layerEnd(G,0)
{

}

DiGraph::~DiGraph()
{

}

edge DiGraph::getOppositeArc(edge e)
{
	if (myArcMap.isDefined(e->target(),e->source()))
		return myArcMap(e->target(),e->source());
	return nullptr;
}


node DiGraph::addNode(unsigned nID, int origID, double nRev, bool term, int startLayer, int endLayer)
{
	ogdf::node n = G.newNode(nID);
	mapper[origID] = n;
	backMapper[n] = origID;
	myNodes.push_back(n);
	myNodeWeights[n] = nRev;
	myNodeTerminal[n] = term;
	layer[n]=startLayer;
	layerEnd[n]=endLayer;

	//cerr<<"weight "<<myNodeWeights[n]<<" "<<n<<endl;
	if(term) {
		Terminals.insert(n);
	}
	return n;
}

edge DiGraph::addEdge(ogdf::node src, ogdf::node dst, double e_cost)
{
	edge myArc = G.newEdge(src, dst);
	myArcWeights[myArc] = e_cost;
	myArcs.push_back(myArc);
	if(e_cost > maxArcWeight)
			maxArcWeight = e_cost;
	return myArc;
}

edge DiGraph::addEdge(unsigned src_id, unsigned dst_id, double e_cost)
{
	return addEdge(mapper[src_id],mapper[dst_id], e_cost);
}


void DiGraph::finalize(node root)
{
	rootNode = mapper[root->index()];

	myArcMap.clear();

	edge e;
	forall_edges(e, G) {
		myArcMap(e->source(),e->target())=e;
	}

	sum_rev=0.0;
	node n;
	forall_nodes(n,G)
		sum_rev+=myNodeWeights[n];
}
