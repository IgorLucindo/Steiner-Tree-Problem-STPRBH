/**
 * \file
 * \brief
 *
 * \author Margin Luipersbeck
 * \date 2014-01-29
 */

#include "instance/STPInstance.hpp"

#include "utility/Logger.hpp"
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/simple_graph_alg.h>

#ifdef __CDT_PARSER__
// This is a hack to let eclipse play nice with ogdf (intention!)
#undef forall_adj_edges
#define forall_adj_edges(e, v) for(e = v->firstAdj()->theEdge(); ; )
#endif

using namespace std;
using namespace ogdf;

STPInstance::STPInstance () :
			Weight(G, 0),
			Prize(G, 0),
			NodeId(G, 0),
			Terminals(G),
			EdgeId(G, 0),
			layerStart(G,-1),
			layerEnd(G,-1),
			rootCost(G,std::numeric_limits<int>::max()),
			Id(rand()),
			budget(0),
			hoplimit(0)
{

}

STPInstance::STPInstance (const STPInstance& inst) :
			G(inst.G),
			NodeId(G),
			EdgeId(G),
			Terminals(G),
			Weight(G),
			Prize(G),
			layerStart(G,-1),
			layerEnd(G,-1),
			rootCost(G,std::numeric_limits<int>::max()),
			Description(inst.Description),
			Name(inst.Name),
			Root(inst.Root),
			Id(inst.Id),
			isInteger(inst.isInteger),
			budget(inst.budget),
			hoplimit(inst.hoplimit)
{
	// We need to manually copy, since graph association is copied in the constructor,
	// so arrays and sets have to be reseeded.
	node n0, n1;
	n0 = inst.G.firstNode();
	n1 = G.firstNode();
	while ((n0) || (n1))
	{
		NodeId[n1] = inst.NodeId[n0];
		Prize[n1] = inst.Prize[n0];
		if (inst.isTerminal(n0))
			Terminals.insert(n1);
		n0 = n0->succ();
		n1 = n1->succ();
	}
	edge e0, e1;
	e0 = inst.G.firstEdge();
	e1 = G.firstEdge();
	while ((e0) || (e1))
	{
		EdgeId[e1] = inst.EdgeId[e0];
		Weight[e1] = inst.Weight[e0];
		e0 = e0->succ();
		e1 = e1->succ();
	}

}

STPInstance::~STPInstance ()
{

}


bool STPInstance::operator == (const STPInstance& rhs) const
{
	return this->Id == rhs.Id;
}

bool STPInstance::preprocessing(node root)
{
	node v;
	edge e;

	int removed=0;
	forall_adj_edges(e, root) {
		if(e->target()==root)
			rootCost[e->source()]=Weight[e];
		else
			rootCost[e->target()]=Weight[e];
	}

	vector<edge> toRemoveEdge;

	forall_edges(e,G)
	{
		if(e->source()==root || e->target()==root )
			continue;
		if(Weight[e]>=rootCost[e->source()] && Weight[e]>=rootCost[e->target()])
			toRemoveEdge.push_back(e);
	}
	for(edge e: toRemoveEdge)
	{
		G.delEdge(e);
		removed++;
	}

	cout<<endl;
	cout<<"-----------------------------------------------"<<endl;
	cout<<"undirected root cost test removed "<< removed<<" edges"<<endl;

	vector<node> toRemove;
	removed=0;

	do
	{
		toRemove.clear();
		forall_nodes(v,G)
		{
			if(!isTerminal(v) && v->degree()<=1)
			{
				toRemove.push_back(v);
			}
		}


		for(node n: toRemove)
		{
			G.delNode(n);
			removed++;
		}
	}
	while(toRemove.size()>0);

	cout<<"degree 1 test removed "<< removed<<" nodes"<<endl;

	Queue<node> queue;
	node w;

	//on which layer can a node first be, rootNode is on layer zero
	layerStart[root] = 0;
	queue.append(root);
	while (!queue.empty()) {
		v = queue.pop();
		// iii: outgoing
		forall_adj_edges(e, v) {
			w=e->opposite(v);

			if(Weight[e]>=rootCost[w] && v!=root)
				continue;

			//first check in the or should be redundant, since we do a bfs
			if(layerStart[w]>layerStart[v]+1 || layerStart[w] == -1)
			{
				layerStart[w] = layerStart[v]+1;
				//cerr<<w<<" "<<layer[w]<<endl;
				queue.append(w);
			}
		}
	}

	toRemove.clear();

	//on which layer can a Steiner node last be, depends on distance to next terminal
	node n;
	forall_nodes(n, G)
	{

		if(layerStart[n]>hoplimit)
		{
			toRemove.push_back(n);
			continue;
		}

		if(isTerminal(n) || n==root)
		{
			layerEnd[n]=hoplimit;
			continue;
		}

		NodeArray<int> dist=NodeArray<int>(G,0);
		bool stop=false;
		queue.append(n);
		while (!queue.empty() && !stop) {
			v = queue.pop();

			forall_adj_edges(e, v) {

				w = e->opposite(v);

				//already take into account the directed root cost test
				if(Weight[e]>=rootCost[w] && v!=root)
					continue;

				//first check in the or should be redundant, since we do a bfs
				if(dist[w]>dist[v]+1 || dist[w] == 0)
				{
					dist[w] = dist[v]+1;
					//cerr<<w<<" "<<layer[w]<<endl;
					queue.append(w);

					//we found a terminal (which will not have been already removed)
					if(isTerminal(w) && layerStart[w]<=hoplimit && w!= root)
					{
						stop=true;
						layerEnd[n]=hoplimit-dist[w];
						if(layerEnd[n]<layerStart[n])
						{
							toRemove.push_back(n);
							//cerr<<n<<" "<<layerStart[n]<<" "<<layerEnd[n]<<endl;
						}
						break;
					}
				}
			}
		}
		queue.clear();
	}


	removed=0;
	for(node n: toRemove)
	{
		G.delNode(n);
		removed++;
	}

	cout<<"start/end layer test removed "<<removed<<" nodes"<<endl;

	if(removed>0)
	{
		removed=0;
		do
		{
			toRemove.clear();
			forall_nodes(v,G)
			{
				if(!isTerminal(v) && v->degree()<=1)
				{
					toRemove.push_back(v);
				}
			}

			for(node n: toRemove)
			{
				G.delNode(n);
				removed++;
			}
		}
		while(toRemove.size()>0);

		cout<<"degree 1 test removed "<<removed<<" nodes"<<endl;
	}


	return true;
}


bool STPInstance::createDiGraphs(node root)
{
	node v;
	edge e;

	preprocessing(root);
	cout<<"after undirected preprocessing"<<endl;
	cout<<"nodes "<<G.numberOfNodes()<<" edges "<<G.numberOfEdges()<<endl;


	basicG.hoplimit=hoplimit;
	basicG.budget=budget;

	unsigned counter=0;
	forall_nodes(v,G)
	{
		basicG.addNode(counter,v->index(),Prize[v],isTerminal(v),layerStart[v],layerEnd[v]);
		counter++;
	}

	forall_edges(e,G)
	{
		//if root is source, we need to add it (note that an undirected file may contain an
		//edge (NODE,ROOT) thus we need to check both source and target
		if(e->source()==root)
			basicG.addEdge(e->source()->index(),e->target()->index(),Weight[e]);
		if(e->target()==root)
			basicG.addEdge(e->target()->index(),e->source()->index(),Weight[e]);

		//incorporate the directed root cost test, and the start/end layer test
		if(e->source()!=root &&  e->target()!=root && Weight[e]<rootCost[e->target()] && layerStart[e->source()]<layerEnd[e->target()])
		{
			basicG.addEdge(e->source()->index(),e->target()->index(),Weight[e]);
		}
		if(e->source()!=root &&  e->target()!=root && Weight[e]<rootCost[e->source()] && layerStart[e->target()]<layerEnd[e->source()])
		{
			basicG.addEdge(e->target()->index(),e->source()->index(),Weight[e]);
		}
	}

	basicG.finalize(root);

	cout<<"after directed preprocessing"<<endl;
	cout<<"nodes "<<basicG.G.numberOfNodes()<<" arcs "<<basicG.G.numberOfEdges()<<endl;

	cout<<"----------------------------------------"<<endl;

	//basicG.G.writeGML("out.gml");

	return true;
}

bool STPInstance::valid () const
{
	bool status = true;
	if (!G.consistencyCheck())
	{
		LOG(severe) << "OGDF datastructure consistency check failed" << endl;
		return false;
	}
	if (!isConnected(G))
	{
		LOG(warn) << "Graph is not connected" << endl;
		status = false;
	}
	if (!isLoopFree(G))
	{
		LOG(warn) << "Graph contains Loops" << endl;
		status = false;
	}
	if (!isParallelFree(G))
	{
		LOG(warn) << "Graph contains parallel edges" << endl;
		status = false;
	}

	node n;
	forall_nodes(n, G)
	{
		if (Prize[n] < 0)
		{
			LOG(warn) << "Node " << NodeId[n] << " has a negative prize" << endl;
			status = false;
		}
	}

	edge e;
	forall_edges(e, G)
	{
		if (Weight[e] < 0)
		{
			LOG(warn) << "Edge (" << NodeId[e->source()] << ", " << NodeId[e->target()] << ") has a negative weight" << endl;
			status = false;
		}
	}

	return status;
}
