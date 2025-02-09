/*
 * STPRBHHeuristics.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: markus
 */

#include <ilcplex/ilocplex.h>
#include <ogdf/basic/HashArray2D.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/List.h>
#include <limits>

#include "heuristics/STPRBHHeuristics.h"
#include "utility/Timer.hpp"

STPRBHHeuristics::STPRBHHeuristics(DiGraph* _G): G(_G), nNodes(G->G.numberOfNodes()), nArcs(G->G.numberOfEdges()),
sol(Graph()),
myNodes(NodeArray<node>(G->G)),
myNodesBack(NodeArray<node>(sol)),
myEdges(EdgeArray<edge>(G->G)),
myEdgeBack(EdgeArray<edge>(sol))
{


}

STPRBHHeuristics::~STPRBHHeuristics()
{

}


/**
 * \brief wrapper for the call of the heuristic as start heuristic, where the original arc weights are used
 *
 * \param inc pointer, where the constructed solution will be saved
 */
double STPRBHHeuristics::PrimI(Solution* inc)
{
	HashArray2D<int, node, double> hyval=HashArray2D<int, node, double>();

	NodeArray<double> myWeights(G->G,1);
	return PrimSTPRBH(inc, G->myArcWeights,myWeights,hyval,0);
}

/**
 * \brief call of the heuristic
 *
 * Modification of the PrimI heuristic for the classical Steiner tree problem.
 * Takes into account budget, hop-limits and profits.
 *
 * \param inc pointer, where the constructed solution will be saved
 * \param weights arc-weights for the heuristic
 * \param nodeWeights node-weights for the heuristic
 * \param hyval weights for the y^h variables, only used when params.heurweights=1
 * \param ub used as indicator, if we are calling it as starting heuristic or not
 */
double STPRBHHeuristics::PrimSTPRBH(Solution* inc, EdgeArray<double> weights, NodeArray<double> nodeWeights, HashArray2D<int, node, double> hyval, double ub)
{
	double cost = 0;
	BinaryHeap2<double, node> queue(G->G.numberOfNodes());
	vector<int> qpos=vector<int>(G->G.numberOfNodes());
	NodeArray<int> vIndex(G->G);
	int i = 0;
	node v;
	edge e;

	vector<double> y_lp=vector<double>(G->G.numberOfNodes(),0);
	vector<double> x_lp=vector<double>(G->G.numberOfEdges(),0);

	NodeArray<edge> predecessor(G->G, nullptr);
	NodeArray<double> distance(G->G,std::numeric_limits<double>::max() - G->maxArcWeight - 1);
	NodeArray<int> nodesToTree(G->G,std::numeric_limits<int>::max());
	NodeArray<int> hops(G->G,G->hoplimit+1);

	NodeArray<bool> inTree(G->G, false);
	NodeArray<bool> extracted(G->G, false);
	EdgeArray<bool> inTreeE(G->G, false);
	//EdgeArray<bool> badArcs(G->G, false);

	// setting distances to "infinity"
	forall_nodes(v, G->G) {
		vIndex[v] = i;
		queue.insert(v, distance[v], &qpos[i++]);
	}

	inTree[G->rootNode] = true;
	y_lp[G->y_id[G->rootNode]]=1;
	distance[G->rootNode] = 0;
	nodesToTree[G->rootNode] = 0;
	hops[G->rootNode]=0;
	queue.decreaseKey(qpos[vIndex[G->rootNode]], 0);


	while (!queue.empty()) {
		v = queue.extractMin();

		extracted[v] = true;

		if(distance[v]>=std::numeric_limits<double>::max() - G->maxArcWeight - 1-0.0001)
			break;
		int hopsw=hops[v]+1;

		//terminals, which are (nearly) not chosen in the LP are viewed as non-terminals
		if(!G->myNodeTerminal[v] || inTree[v]  || (G->myNodeTerminal[v] && nodeWeights[v]<0.1)) {
			forall_adj_edges(e, v) {
				if(e->target() == v)
					continue;
				if(e->target() == G->rootNode)
					continue;

				node w = e->opposite(v);
				double myWeight=weights[e];

				//check, if this is a "feasible layer" for a node
				if(!hyval.isDefined(hopsw,w) && ub!=0)
					continue;

				if(params.heurweights==1 && ub!=0)
				{
					double helper=G->myArcWeights[e]*(1-hyval(hopsw,w));
					if(helper>myWeight)
					{
						myWeight=helper;
					}
				}

				//improvement and allowed?
				if (distance[w] > distance[v] + myWeight + 0.0001 && hopsw<=G->layerEnd[w])
				{

					distance[w] = distance[v] + myWeight;
					hops[w] = hopsw;
					predecessor[w] = e;

					if(extracted[w]) {
						extracted[w] = false;
						queue.insert(w, distance[w], &qpos[vIndex[w]]);
					}
					queue.decreaseKey(qpos[vIndex[w]], distance[w]);
				}
				else if ((long)distance[w] == (long)(distance[v] + myWeight) && hops[w] > hops[v] && hopsw<=G->layerEnd[w])
				{
					predecessor[w] = e;
					hops[w]=hopsw;
				}
			}
		} else {

			node i = v;

			bool fit=true;
			int costAdd=0;
			//see if the path to the terminal fits in the bugdet
			while(!inTree[i]) {
				e = predecessor[i];
				if(e->source() == i)
					e = G->getOppositeArc(e);
				costAdd += G->myArcWeights[e];
				if(cost+costAdd > G->budget)
				{
					fit=false;
					break;
				}
				i = predecessor[i]->opposite(i);
			}

			//if yes, add, if no ignore
			if(fit)
			{
				i = v;
				while(!inTree[i]) {
					distance[i] = 1e-4 - 1e-7 *G->myNodeWeights[i];
					queue.insert(i, distance[i], &qpos[vIndex[i]]);
					inTree[i] = true;
					y_lp[G->y_id[i]]=1;
					e = predecessor[i];
					if(e->source() == i)
						e = G->getOppositeArc(e);
					cost += G->myArcWeights[e];
					inTreeE[e] = true;
					x_lp[G->x_id[e]]=1;

					i = predecessor[i]->opposite(i);
				}
			}
		}
	}

	double time=Timer::total.elapsed().getSeconds();

	inc->createSolution(x_lp,y_lp,time,ub);


	//try to improve the solution by simple arc-exchanges of leaf-arcs
	sol.clear();
	myNodes.init(G->G);
	myNodesBack.init(sol);
	myEdges.init(G->G);
	myEdgeBack.init(sol);

	bool improved=false;
	node n;
	forall_nodes(n,G->G)
	{
		if (y_lp[G->y_id[n]] > 0.5)
		{
			//cerr<<n<<endl;
			myNodes[n] = sol.newNode();
			myNodesBack[myNodes[n]]=n;
		}
	}

	forall_edges(e,G->G)
	{
		if (x_lp[G->x_id[e]] > 0.5 )
		{
			//cerr<<e<<endl;
			myEdges[e]=sol.newEdge(myNodes[e->source()], myNodes[e->target()]);
			myEdgeBack[myEdges[e]]=e;
		}
	}

	double newCosts=cost;
	forall_edges(e,sol)
	{
		node t=e->target();
		double revT=G->myNodeWeights[myNodesBack[t]];
		edge chosen=myEdgeBack[e];
		node replace=myNodesBack[t];

		if (t->degree()==1)
		{
			//cerr<<"before "<<newCosts<<endl;
			newCosts-=G->myArcWeights[chosen];
			//cerr<<"after "<<newCosts<<endl;

			node s=e->source();
			edge f;
			forall_adj_edges(f,myNodesBack[s])
			{
				if(newCosts+G->myArcWeights[f]>G->budget)
					continue;
				node tar=f->target();
				if(tar==myNodesBack[s] || tar==myNodesBack[t])
					continue;
				if(y_lp[G->y_id[tar]]==0 && (G->myNodeWeights[tar]>revT || (G->myNodeWeights[tar]==revT
						&& G->myArcWeights[f]<G->myArcWeights[chosen] )) )
				{
					improved=true;
					revT=G->myNodeWeights[tar];
					y_lp[G->y_id[replace]]=0;
					y_lp[G->y_id[tar]]=1;
					x_lp[G->x_id[chosen]]=0;
					x_lp[G->x_id[f]]=1;
					chosen=f;
					replace=tar;
				}
			}
			newCosts+=G->myArcWeights[chosen];
			//cerr<<"end "<<newCosts<<endl;

		}
	}

	if(improved)
	{
		time=Timer::total.elapsed().getSeconds();
		inc->createSolution(x_lp,y_lp,time,ub);
		cost=newCosts;
	}

	return cost;
}
