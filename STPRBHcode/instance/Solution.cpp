/*
 * Solution.cpp
 *
 *  Created on: Mar 30, 2014
 *      Author: markus
 */

#include "Solution.h"
#include <limits>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/extended_graph_alg.h>
#include "utility/ProgramOptions.h"
#include <string>
#include <sstream>


Solution::Solution(DiGraph* _G):
	G(_G),
	sol(Graph()),
	feas(false),
	integer(true),
	created(false),
	obj(-1),
	ub(numeric_limits<double>::max()),
	budget(0),
	hops(0),
	findtime(0)
{

	//cerr<<"solution constructor"<<endl;
	nNodes = 0;
	nArcsRoot = 0;
	nArcs = 0;
	root = nullptr;
	cntSol = 0;


	if(G != nullptr)
		root = G->rootNode;

	xv = EdgeArray<double>(G->G,0);
	yv = NodeArray<double>(G->G,0);
	yv[root]=1;

	hyv = HashArray2D<int, node, double>();

	node n;
	forall_nodes(n,G->G)
	{
		if(n==G->rootNode)
			continue;
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(!G->hyvar.isDefined(j,n))
				continue;
			hyv(j,n)=0;
		}
	}

	constructionID = 0;

}

Solution::~Solution()
{

}

void Solution::clear()
{
	obj = -1;
	budget = 0;
	hops = 0;
	findtime = 0;
}

double Solution::computeObjective()
{
	obj = 0;
	edge e;
	forall_edges(e,G->G)
	{
		budget += G->myArcWeights[e] * xv[e];
	}

	node n;
	forall_nodes(n,G->G)
	{
		obj += G->myNodeWeights[n] * yv[n];
	}

	return obj;
}

int Solution::createSolution(const vector<double>& x_lp, const vector<double>& y_lp, double _findtime, double _ub)
{

	sol.clear();
	obj = 0;
	budget = 0;
	hops = -1;
	xv.fill(0);
	yv.fill(0);
	//cerr<<_findtime<<endl;
	findtime=_findtime;
	ub=_ub;
	//cerr<<findtime<<endl;

	edge e;
	forall_edges(e,G->G)
	{
		xv[e] = x_lp[G->x_id[e]];
		//cerr<<e<<" "<<x_lp[G->x_id[e]]<<endl;
		budget += G->myArcWeights[e] * xv[e];
		if (xv[e] < 0.99 && xv[e] > 0.01)
			integer = false;
	}

	node n;
	forall_nodes(n,G->G)
	{
		yv[n] = y_lp[G->y_id[n]];
		//cerr<<obj<<" "<<G->myNodeWeights[n]<<" "<<yv[n]<<" "<<n<<endl;
		obj += G->myNodeWeights[n] * (yv[n]);
		if (yv[n] < 0.99 && yv[n] > 0.01)
			integer = false;
	}

	forall_nodes(n,G->G)
	{
		if(n==G->rootNode)
			continue;
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(!G->hyvar.isDefined(j,n))
				continue;
			hyv(j,n)=0;
		}
	}



	//cerr<<budget<<" "<<G->budget<<endl;

	if (integer && budget<=G->budget)
	{
		NodeArray<node> myNodes = NodeArray<node>(G->G);
		NodeArray<node> myNodesBack = NodeArray<node>(sol);

		node n;
		forall_nodes(n,G->G)
		{
			if (yv[n] > 0.5)
			{
				//cerr<<n<<endl;
				myNodes[n] = sol.newNode();
				myNodesBack[myNodes[n]]=n;
				nNodes++;
			}
		}

		edge e;

		forall_edges(e,G->G)
		{
			if (xv[e] > 0.5 )
			{
				//cerr<<e<<endl;
				//cerr<<myNodes[e->source()]<<endl;
				//cerr<<myNodes[e->target()]<<endl;
				sol.newEdge(myNodes[e->source()], myNodes[e->target()]);
				nArcsRoot++;
				nArcs++;
			}
		}


		feas = isConnected(sol);
		//cerr<<"connected "<<feas<<endl;
		if(feas)
		{
			NodeArray<int> level(sol, -1);
			Stack<node> stack;

			level[myNodes[G->rootNode]] = 0;
			stack.push(myNodes[G->rootNode]);

			node v,w;
			while (!stack.empty())
			{
				v = stack.pop();
				forall_adj_edges(e, v)
				{
					w = e->opposite(v);
					if( e->source() == v)
					{
						if(level[w]==-1)
						{
							level[w] = level[v]+1;
							hyv(level[w],myNodesBack[w])=1;
							//cerr<<myNodesBack[w]<<" "<<level[w]<<" "<<myNodesBack[v]<<" "<<hyv(level[w],myNodesBack[w])<<endl;
							stack.push(w);
						}
					}
				}
			}
			forall_nodes(n,sol)
			{
				//cerr<<myNodesBack[n]<<" "<<level[n]<<endl;
				if(level[n]>hops)
					hops=level[n];
			}
		}

		if(hops>G->hoplimit)
			feas=false;
	}
	else
	{
		feas=false;
	}

	//cerr<<obj<<" "<<budget<<" "<<hops<<" "<<feas<<endl;

	//cerr<<"feas "<<feas<<endl;
	created = true;
	//writeGML();

	LOG(info) << "created solution with objective " << obj << endl;


	if(!feas)
		return -1;

	return obj;
}

void Solution::createSolution(const IloNumArray& x_lp, const IloNumArray& y_lp,  HashArray2D<int, node, double> hy_lp, double _findtime, double _ub)
{
	//cerr<<"create"<<endl;
	sol.clear();
	obj = 0;
	budget = 0;
	hops = -1;
	xv.fill(0);
	yv.fill(0);
	findtime=_findtime;
	//cerr<<findtime<<endl;
	ub=_ub;

	edge e;
	forall_edges(e,G->G)
	{
		xv[e] = x_lp[G->x_id[e]];
		//cerr<<e<<" "<<x_lp[G->x_id[e]]<<endl;
		budget += G->myArcWeights[e] * xv[e];

		if (xv[e] < 0.99 && xv[e] > 0.01)
			integer = false;
	}

	node n;
	forall_nodes(n,G->G)
	{
		yv[n] = y_lp[G->y_id[n]];
		//cerr<<obj<<" "<<G->myNodeWeights[n]<<" "<<yv[n]<<" "<<n<<endl;
		obj += G->myNodeWeights[n] * (yv[n]);
		if (yv[n] < 0.99 && yv[n] > 0.01)
			integer = false;
	}

	forall_nodes(n,G->G)
	{
		if(n==G->rootNode)
			continue;
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(!G->hyvar.isDefined(j,n))
				continue;
			hyv(j,n)=hy_lp(j,n);
			//if(hyv(j,n)>0.5)
			//	cerr<<j<<" "<<n<<endl;
		}
	}

	if (integer && budget<=G->budget)
	{
		NodeArray<node> myNodes = NodeArray<node>(G->G);
		NodeArray<node> myNodesBack = NodeArray<node>(sol);

		node n;
		forall_nodes(n,G->G)
		{
			if (yv[n] > 0.5)
			{
				//cerr<<n<<endl;
				myNodes[n] = sol.newNode();
				myNodesBack[myNodes[n]]=n;
				nNodes++;
			}
		}

		edge e;

		forall_edges(e,G->G)
		{
			if (xv[e] > 0.5 )
			{
				//cerr<<e<<endl;
				sol.newEdge(myNodes[e->source()], myNodes[e->target()]);
				nArcsRoot++;
				nArcs++;
			}
		}
		//sol.writeGML("out.gml");

		feas = isConnected(sol);
		//cerr<<"connected "<<feas<<endl;
		if(feas)
		{
			NodeArray<int> level(sol, -1);
			Stack<node> stack;

			level[myNodes[G->rootNode]] = 0;
			stack.push(myNodes[G->rootNode]);

			node v,w;
			while (!stack.empty())
			{
				v = stack.pop();
				forall_adj_edges(e, v)
				{
					w = e->opposite(v);
					if( e->source() == v)
					{
						if(level[w]==-1)
						{
							level[w] = level[v]+1;
							//cerr<<myNodesBack[w]<<" "<<level[w]<<" "<<myNodesBack[v]<<endl;
							stack.push(w);
						}
					}
				}
			}
			forall_nodes(n,sol)
			{
				//cerr<<myNodesBack[n]<<" "<<level[n]<<endl;
				if(level[n]>hops)
					hops=level[n];
			}
		}

		if(hops>G->hoplimit)
			feas=false;
	}
	else
	{
		feas=false;
	}

	//cerr<<obj<<" "<<budget<<" "<<hops<<" "<<feas<<endl;

	created = true;
	//writeGML();

	LOG(info) << "created solution with objective " << obj << endl;

}

Solution::Solution()
{

}



string Solution::print()
{
	stringstream output;

	output << "#solution as arclist"<<endl;

	edge e;
	forall_edges(e, G->G)
	{
		if(xv[e]>0.5)
		{
			output<<G->backMapper[e->source()]+1 <<" "<<G->backMapper[e->target()]+1 <<endl;
		}
	}

	output << "#statistics" <<endl;
	output << "#feasible, " << isFeasible() << endl;
	output << "#objective, " << getObjective() << endl;
	output << "#used budget, " << budget << endl;
	output << "#hops, " << hops << endl;
	output << "#soltime, "<< findtime <<endl;
	output << "#ub, " <<ub<<endl;
	return output.str();
}

string Solution::printDimacs()
{
	stringstream output;

	int nodeCount=0;
	node n;
	forall_nodes(n, G->G)
	{
		if(yv[n]>0.5)
		{
			nodeCount++;
		}
	}
	output << "Vertices "<<nodeCount<<endl;
	forall_nodes(n, G->G)
	{
		if(yv[n]>0.5)
		{
			output<<"V "<<G->backMapper[n]+1<<endl;
		}
	}

	int edgeCount=0;
	edge e;
	forall_edges(e, G->G)
	{
		if(xv[e]>0.5)
		{
			edgeCount++;
		}
	}
	output<<"Edges "<<edgeCount<<endl;
	forall_edges(e, G->G)
	{
		if(xv[e]>0.5)
		{
			output<<"E "<<G->backMapper[e->source()]+1 <<" "<<G->backMapper[e->target()]+1 <<endl;
		}
	}

	return output.str();
}

void Solution::writeAuxFile()
{
	ofstream os(params.outputfile3);

	node n;
	forall_nodes(n,G->G)
	{
		os << n << " " << yv[n] << endl;
	}

	edge e;
	forall_edges(e,G->G)
	{
		os << e->index() << " " << xv[e] << endl;
	}

	os.close();
}

void Solution::writeGML()
{
	ofstream os("solution.gml");
	sol.writeGML(os);
	os.close();
}
