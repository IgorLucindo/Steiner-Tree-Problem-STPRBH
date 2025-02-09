/*
 * STPRBHSepCon.cpp
 *
 *  Created on: Mar 28, 2014
 *      Author: markus
 */

#include "STPRBHSepCon.h"
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/extended_graph_alg.h>
#include <limits>

STPRBHSepCon::STPRBHSepCon(DiGraph* _G, IloEnv _env, IloNumVarArray _x, IloNumVarArray _y, bool _integer) :
	G(_G),
	nNodes(G->G.numberOfNodes()),
	nArcs (G->G.numberOfEdges()),
	nTerminals(G->Terminals.size()),
	nHoplink(0),
	nHopend(0),
	nNodearccut(0),
	nFlowbalance(0),
	nHopflowbalance(0),
	nOtahoplink(0),
	nEtahoplink(0),
	nCcuts(0),
	nGhoplink(0),
	env(_env),
	x(_x),
	y(_y),
	myCuts(vector<IloRange>()),
	myPurgeableCuts(vector<IloRange>()),
	incObj(std::numeric_limits<double>::max())
{
	//cerr << "STPRBHSepCon" << endl;
	initialize();
}

STPRBHSepCon::~STPRBHSepCon()
{
}

STPRBHSepCon::STPRBHSepCon(const STPRBHSepCon& _copy) :
	G(_copy.G),
	nNodes(G->G.numberOfNodes()),
	nArcs (G->G.numberOfEdges()),
	nTerminals(G->Terminals.size()),
	nHoplink(0),
	nHopend(0),
	nNodearccut(0),
	nFlowbalance(0),
	nHopflowbalance(0),
	nOtahoplink(0),
	nEtahoplink(0),
	nCcuts(0),
	nGhoplink(0),
	env(_copy.env),
	x(_copy.x),
	y(_copy.y),
	myCuts(vector<IloRange>()),
	myPurgeableCuts(vector<IloRange>()),
	incObj(std::numeric_limits<double>::max())
{
	initialize();
}

inline void STPRBHSepCon::initialize()
{

	nodesGToSep=NodeArray<node>(G->G);
	nodesSepToG=NodeArray<node>(sepG);
	arcsGToSep=EdgeArray<edge>(G->G);
	arcsSepToG=EdgeArray<edge>(sepG);

	components =NodeArray<int>(sepG);

	node n;
	forall_nodes(n,G->G)
	{
		nodesGToSep[n]=sepG.newNode();
		nodesSepToG[nodesGToSep[n]]=n;
		//cerr<<n<<endl;

	}

	edge e;
	forall_edges(e,G->G)
	{
		//cerr<<e<<endl;
		arcsGToSep[e]=sepG.newEdge(nodesGToSep[e->source()], nodesGToSep[e->target()]);
		arcsSepToG[arcsGToSep[e]]=e;
	}

}


vector<IloRange> STPRBHSepCon::getCuts()
{
	return myCuts;
}

vector<IloRange> STPRBHSepCon::getPurgeableCuts()
{
	return myPurgeableCuts;
}


int STPRBHSepCon::separation(IloNumArray _x_lp, IloNumArray _y_lp, double _incObj, HashArray2D<int, node, double> _hyval)
{
	myCuts.clear();
	myPurgeableCuts.clear();
	hyval=_hyval;
	x_lp=_x_lp;
	y_lp=_y_lp;
	incObj=_incObj;

	//cerr<<"start separation"<<endl;

	int nCuts=0;

	if(params.sephoplink==1)
	{
		nHoplink=separateHopLink();
		nCuts+=nHoplink;
	}

	if(params.sephopend==1)
	{
		nHopend=separateHopEnd();
		nCuts+=nHopend;
	}


	if(params.sepnodearccut==1)
	{
		nNodearccut=separateNodeArcCut();
		nCuts+=nNodearccut;
	}

	if(params.sepflow==1)
	{
		nFlowbalance=separateFlowBalance();
		nCuts+=nFlowbalance;
	}

	if(params.sepflowhop==1)
	{
		nHopflowbalance=separateFlowBalanceHop();
		nCuts+=nHopflowbalance;
	}


	if(params.sepotahoplink==1)
	{
		nOtahoplink=separateOddTwoHopLink();
		nCuts+=nOtahoplink;
	}

	if(params.sepetahoplink==1)
	{
		nEtahoplink=separateEvenTwoHopLink();
		nCuts+=nEtahoplink;
	}

	if(params.sepghoplink==1)
	{
		nGhoplink=separateGHopLink();
		nCuts+=nGhoplink;
	}


	//do it as last, since it modifies x_lp for nested
	if(params.sepcut==1)
	{
		nCcuts=separateCut();
		//cerr<<"con "<<nCcuts<<endl;
		nCuts+=nCcuts;
	}


	return nCuts;
}

int STPRBHSepCon::separateCut()
{

	int nCuts=0;
	bool found=false;

	do
	{
		found=false;

		sepG.restoreAllEdges();
		edge e;

		//hide arcs, which are zero
		vector<edge> arcsToHide=vector<edge>();
		forall_edges(e,sepG)
		{
			if(x_lp[G->x_id[arcsSepToG[e]]] < 0.5)
			{
				arcsToHide.push_back(e);
			}
		}

		for(edge e: arcsToHide)
			sepG.hideEdge(e);



		//caculate the connected components
		int nComponents = ogdf::connectedComponents(sepG, components);

		//find root component
		int rootC = components[nodesGToSep[G->rootNode]];

		node v;
		int cmp = -1;

		HashArray<int, vector<node> > myComponents;
		if(nComponents > 1) {

			//extract the componentes
			forall_nodes(v, sepG) {
				cmp = components[v];
				if(myComponents.isDefined(cmp)) {
					myComponents[cmp].push_back(nodesSepToG[v]);
				} else {
					myComponents[cmp] = vector<node>(1, nodesSepToG[v]);
				}
			}

			for(int i = 0; i < nComponents; i++) {
				if(i == rootC)
					continue;

				//GSEC=cuts of size two are always added statically in the beginning
				if(myComponents[i].size() > 1) {
					found=true;
					nCuts++;
					double revInComp = 0.0;
					double score = -1;
					double bestScore = -1;
					node term = nullptr;
					IloExpr cut(env);


					//calculate "best" terminal in the component to make cut to
					for (unsigned j = 0; j < myComponents[i].size(); j++) {
						revInComp += G->myNodeWeights[myComponents[i].at(j)];
						score = 1e6 * G->myNodeTerminal[ myComponents[i].at(j)] + max(-1e4, min(G->myNodeWeights[ myComponents[i].at(j)], 1e4));
						if (score > bestScore)
							term = myComponents[i].at(j);
					}

					//create the lhs of the cut, and set all arcs in it to one for nested
					for(unsigned j = 0; j < myComponents[i].size(); j++)
					{
						edge e;
						forall_adj_edges(e,myComponents[i].at(j))
						{
							if(e->source()==myComponents[i].at(j))
								continue;
							if (components[e->source()] != components[e->target()]) {
								cut += G->xvar[e];
								x_lp[G->x_id[e]] = 1;
							}
						}

					}

					double revOutComp=G->sum_rev-revInComp;
					//cout<<revOutComp<<" "<<incObj<<endl;
					//if not enough prizes outside of component, we can lift the rhs
					if (revOutComp < incObj) {
						myCuts.push_back(IloRange(env, 1, cut, IloInfinity));

					} else {

						cut -= G->yvar[term];
						myCuts.push_back(IloRange(env, 0, cut, IloInfinity));
					}
					//cout<<cut<<endl;
					cut.end();
				}
			}
		}
	}
	while(nCuts<params.nestedcuts && found);

	return nCuts;
}

int STPRBHSepCon::separateGHopLink()
{
	int nCuts=0;

	edge e;
	forall_edges(e, G->G)
	{
		if(e->source()==G->rootNode)
			continue;
		if(e->target()==G->rootNode)
			continue;

		double lhs=0.0;
		lhs+=x_lp[G->x_id[e]];

		if(lhs<threshold)
			continue;

		IloExpr gHopLink(env);

		gHopLink += G->xvar[e];


		double rhs=0.0;

		node s = e->source();
		node t = e->target();

		for (int i = 2; i <= G->hoplimit; i++) {
			double sourceH=-1.0;
			double targetH=-1.0;
			//take smaller one of y_s^{i-1} and y_t^i or nothing, if one of the variables do not exist
			//the latter case it the downlifting similar to the one for hoplink etc.
			if (G->hyvar.isDefined(i-1, s))
			{
				sourceH=hyval(i-1,s);
			}
			if (G->hyvar.isDefined(i, t))
			{
				targetH=hyval(i,t);
			}
			if(sourceH>=targetH && targetH>-1.0)
			{
				gHopLink-=G->hyvar(i, t);
				rhs+=targetH;
			}
			else if(sourceH>-1.0)
			{
				gHopLink-=G->hyvar(i-1, s);
				rhs+=sourceH;
			}
			if(rhs>=lhs)
				break;
		}

		if(lhs>rhs+threshold)
		{
			//cerr<<gHopLink<<endl;
			//cerr<<lhs<<" "<<rhs<<endl;
			myPurgeableCuts.push_back(IloRange(env,-IloInfinity, gHopLink, 0));
			nCuts++;
		}
		gHopLink.end();
	}

	return nCuts;
}


int STPRBHSepCon::separateNodeArcCut()
{
	int nCuts=0;
	node n;
	forall_nodes(n, G->G)
	{
		if(n==G->rootNode)
			continue;

		if(y_lp[G->y_id[n]]<threshold)
			continue;
		for(int j=2;j<=G->hoplimit;j++)
		{
			if(!G->hyvar.isDefined(j,n))
				continue;
			double rhs=hyval(j,n);
			if(rhs<threshold)
				continue;

			IloExpr myCut(env);
			myCut-=G->hyvar(j,n);

			double lhs=0;
			edge e;
			forall_adj_edges(e,n)
			{
				if(e->source()==n)
					continue;
				node m=e->source();
				if(!G->hyvar.isDefined(j-1,m))
					continue;

				double toAdd=hyval(j-1,m);
				if(toAdd+threshold>=x_lp[G->x_id[e]])
				{
					toAdd=x_lp[G->x_id[e]];
					myCut+=G->xvar[e];
				}
				else
				{
					myCut+=G->hyvar(j-1,m);
				}
				lhs+=toAdd;
				if(lhs>=rhs)
					break;
			}

			if(lhs+threshold<rhs)
			{
				//cerr<<lhs<<" "<<rhs<<endl;
				//cerr<<myCut<<endl;
				myPurgeableCuts.push_back(IloRange(env,0, myCut, IloInfinity));
				nCuts++;
			}
			myCut.end();

		}
	}
	return nCuts;
}

int STPRBHSepCon::separateHopEnd()
{
	int nCuts=0;
	edge e;
	forall_edges(e, G->G)
	{
		if(e->source()==G->rootNode || e->target()==G->rootNode || G->layerEnd[e->target()] >G->layerEnd[e->source()] || x_lp[G->x_id[e]]<threshold)
			continue;

		double sum=x_lp[G->x_id[e]]-y_lp[G->y_id[e->source()]];

		IloExpr myHopArc(env);

		myHopArc+=G->xvar[e];
		myHopArc-=G->yvar[e->source()];

		for(int k=G->layerEnd[e->target()] ;k<=G->layerEnd[e->source()];++k)
		{
			sum+=hyval(k,e->source());
			myHopArc+=G->hyvar(k,e->source());
		}


		if(sum>threshold)
		{
			//cout<<"HE "<<myHopArc<<endl;
			myCuts.push_back(IloRange(env, -IloInfinity, myHopArc, 0));
			nCuts++;
		}
		myHopArc.end();
	}

	return nCuts;
}


int STPRBHSepCon::separateHopLink()
{
	int nCuts=0;
	edge e;

	forall_edges(e, G->G) {
		if(e->source()==G->rootNode ||e->target()==G->rootNode || x_lp[G->x_id[e]]<threshold)
				continue;

		int endLayer=min(G->layerEnd[e->source()]+1,G->layerEnd[e->target()]);
		for(int j=G->layer[e->source()]+1;j<=endLayer;++j)
		{

			double sum=hyval(j-1,e->source())+ x_lp[G->x_id[e]]-y_lp[G->y_id[e->source()]]-hyval(j,e->target());

			IloExpr myHopArc(env);
			myHopArc+=G->hyvar(j-1,e->source());
			myHopArc+=G->xvar[e];
			myHopArc-=G->yvar[e->source()];
			myHopArc-=G->hyvar(j,e->target());

			for(int k=G->layerEnd[e->target()] ;k<=G->layerEnd[e->source()];++k)
			{
				//cout<<k<<" "<<hyval(k,e->source())<<endl;
				sum+=hyval(k,e->source());
				myHopArc+=G->hyvar(k,e->source());
			}

			//cerr<<sum<<" "<<threshold<<endl;

			if(sum>threshold)
			{
				myCuts.push_back(IloRange(env, -IloInfinity, myHopArc, 0));
				nCuts++;
			}
			myHopArc.end();

		}
	}

	return nCuts;
}

int STPRBHSepCon::separateFlowBalanceHop()
{
	int nCuts=0;

	node v;
	forall_nodes(v,G->G)
	{
		if(G->myNodeTerminal[v] || v==G->rootNode)
			continue;

		for(int i=G->layer[v];i<G->layerEnd[v];i++)
		{
			if(!G->hyvar.isDefined(i,v))
				continue;

			double rhs=hyval(i,v);
			if(rhs<threshold)
				continue;

			double lhs=0.0;
			IloExpr flowBalance(env);
			flowBalance += G->hyvar(i,v);
			edge e;
			forall_adj_edges(e,v)
			{
				if(e->target()==v)
					continue;
				node m=e->target();
				if(!G->hyvar.isDefined(i+1,m))
					continue;
				double toAdd=hyval(i+1,m);
				if(toAdd>x_lp[G->x_id[e]])
				{
					toAdd=x_lp[G->x_id[e]];
					flowBalance-=G->xvar[e];
				}
				else
				{
					flowBalance-=G->hyvar(i+1,m);
					//hyval(j-1,m)=1;
				}
				lhs+=toAdd;
				if(lhs>=rhs)
					break;
			}

			//cerr<<outgoing<<" "<<lhs<<endl;
			if(lhs+threshold<rhs)
			{
				//cerr<<lhs<<" "<<rhs<<endl;
				//cerr<<flowBalance<<endl;
				myPurgeableCuts.push_back(IloRange(env, -IloInfinity, flowBalance, 0));
			}
			flowBalance.end();
		}
	}

	return nCuts;
}

int STPRBHSepCon::separateFlowBalance()
{
	int nCuts=0;
	node v;
	edge e;
	forall_nodes(v,G->G)
	{
		double lhs= y_lp[G->y_id[v]];

		if(lhs>threshold && !G->myNodeTerminal[v])
		{
			double outgoing=0.0;
			IloExpr flowBalance(env);
			flowBalance += G->yvar[v];
			forall_adj_edges(e,v)
			{
				if(e->source()==v)
				{
					flowBalance -= G->xvar[e];
					outgoing+=x_lp[G->x_id[e]];
					if(outgoing>=lhs)
						break;
				}
			}
			//cerr<<outgoing<<" "<<lhs<<endl;
			if(outgoing+threshold<lhs)
			{
				myPurgeableCuts.push_back(IloRange(env, -IloInfinity, flowBalance, 0));
				nCuts++;
			}
			flowBalance.end();
		}
	}
	return nCuts;
}

int STPRBHSepCon::separateOddTwoHopLink()
{
	int nCuts=0;
	edge e;
	forall_edges(e, G->G)
	{
		if (e->source()->index() > e->target()->index())
			continue;
		if(e->source()==G->rootNode)
			continue;
		if(e->target()==G->rootNode)
			continue;

		double lhs=0.0;

		edge f = G->getOppositeArc(e);
		if(f==nullptr)
			continue;

		lhs+=x_lp[G->x_id[e]];
		lhs+=x_lp[G->x_id[f]];

		if(lhs<threshold)
			continue;

		IloExpr oddTwoHLink(env);
		oddTwoHLink += G->xvar[e];
		oddTwoHLink += G->xvar[f];


		double rhs=0.0;

		node s = e->source();
		node t = e->target();

		for (int i = 1; i <= G->hoplimit; i=i+2) {
			if (G->hyvar.isDefined(i, s))
			{
				if (G->hyvar.isDefined(i-1, t) || G->hyvar.isDefined(i+1, t))
				{
					rhs+=hyval(i,s);
					oddTwoHLink -= G->hyvar(i, s);
				}
			}
			if (G->hyvar.isDefined(i, t))
			{
				if (G->hyvar.isDefined(i-1, s) || G->hyvar.isDefined(i+1, s))
				{
					rhs+=hyval(i,t);
					oddTwoHLink -= G->hyvar(i, t);
				}
			}
			if(rhs>=lhs)
				break;
		}

		if(lhs>rhs+threshold)
		{
			//cerr<<oddTwoHLink<<endl;
			//cerr<<lhs<<" "<<rhs<<endl;
			myPurgeableCuts.push_back(IloRange(env,-IloInfinity, oddTwoHLink, 0));
			nCuts++;
		}
		oddTwoHLink.end();
	}

	return nCuts;
}

int STPRBHSepCon::separateEvenTwoHopLink()
{
	int nCuts=0;
	edge e;
	forall_edges(e, G->G)
	{
		if (e->source()->index() > e->target()->index())
			continue;
		if(e->source()==G->rootNode)
			continue;
		if(e->target()==G->rootNode)
			continue;

		double lhs=0.0;

		edge f = G->getOppositeArc(e);
		if(f==nullptr)
			continue;

		lhs+=x_lp[G->x_id[e]];
		lhs+=x_lp[G->x_id[f]];

		if(lhs<threshold)
			continue;

		IloExpr evenTwoHLink(env);
		evenTwoHLink += G->xvar[e];
		evenTwoHLink += G->xvar[f];


		double rhs=0.0;

		node s = e->source();
		node t = e->target();

		for (int i = 2; i <= G->hoplimit; i=i+2) {
			if (G->hyvar.isDefined(i, s))
			{
				if (G->hyvar.isDefined(i-1, t) || G->hyvar.isDefined(i+1, t))
				{
					rhs+=hyval(i,s);
					evenTwoHLink -= G->hyvar(i, s);
				}
			}
			if (G->hyvar.isDefined(i, t))
			{
				if (G->hyvar.isDefined(i-1, s) || G->hyvar.isDefined(i+1, s))
				{
					rhs+=hyval(i,t);
					evenTwoHLink -= G->hyvar(i, t);
				}
			}
			if(rhs>=lhs)
				break;
		}

		if(lhs>rhs+threshold)
		{
			//cerr<<evenTwoHLink<<endl;
			//cerr<<lhs<<" "<<rhs<<endl;
			myPurgeableCuts.push_back(IloRange(env,-IloInfinity, evenTwoHLink, 0));
			nCuts++;
		}
		evenTwoHLink.end();
	}

	return nCuts;
}
