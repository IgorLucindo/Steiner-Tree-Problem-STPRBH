/*
 * STPRBHSepFlow.cpp
 *
 *  Created on: Mar 28, 2014
 *      Author: markus
 */

#include "STPRBHSepFlow.h"
#include "ogdf/basic/Stack.h"
#include "utility/ProgramOptions.h"

STPRBHSepFlow::STPRBHSepFlow(DiGraph* _G, IloEnv _env, IloNumVarArray _x, IloNumVarArray _y, bool _integer):
G(_G), nNodes(G->G.numberOfNodes()),
nArcs(G->G.numberOfEdges()),
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
env(_env), x(_x), y(_y),
myCuts(vector<IloRange>()),
myPurgeableCuts(vector<IloRange>()), myFlow(0), capacities(0), cut(0),
epsInt(1e-3),epsOpt(1e-6), epsCreep (params.creepflow), threshold(params.septhreshold),
incObj(std::numeric_limits<double>::max())
{
	//cerr << "STPRBHSepFlow" << endl;
	initialize();
}

STPRBHSepFlow::~STPRBHSepFlow()
{
	if(myFlow)
		delete myFlow;
	myFlow = 0;

	if(capacities) {
		delete[] capacities;
	}
	capacities = 0;

	if(cut)
		delete[] cut;
	cut = 0;
}



inline void STPRBHSepFlow::initialize()
{
	arcs = std::list< std::pair<u_int, u_int> >();

	edge e;
	forall_edges(e,G->G)
	{
		arcs.push_back(make_pair(e->source()->index(), e->target()->index()));
	}

	myFlow = new Maxflow(nNodes, nArcs, arcs);
	capacities = new double[nArcs];
	cut = new int[nNodes];

	Terminals=G->Terminals.nodes();



}

STPRBHSepFlow::STPRBHSepFlow(const STPRBHSepFlow& _copy):
		G(_copy.G),
		nNodes(G->G.numberOfNodes()),
		nArcs(G->G.numberOfEdges()),
		nTerminals(Terminals.size()),
		myFlow(0), capacities(0), cut(0),
		nHoplink(0),
		nHopend(0),
		nNodearccut(0),
		nFlowbalance(0),
		nHopflowbalance(0),
		nOtahoplink(0),
		nEtahoplink(0),
		nCcuts(0),
		nGhoplink(0),
		env(_copy.env), x(_copy.x), y(_copy.y),
		threshold(params.septhreshold),
		myCuts(vector<IloRange>()),
		myPurgeableCuts(vector<IloRange>()),
		incObj(std::numeric_limits<double>::max())
{
	//cerr << "STPRBHSepFlowCopy" << endl;
	initialize();
}

vector<IloRange> STPRBHSepFlow::getCuts()
{
	return myCuts;
}

vector<IloRange> STPRBHSepFlow::getPurgeableCuts()
{
	return myPurgeableCuts;
}

int STPRBHSepFlow::separation(const IloNumArray& _x_lp,const IloNumArray& _y_lp, double _incObj, HashArray2D<int, node, double> _hyval)
{
	incObj=_incObj;
	myCuts.clear();
	myPurgeableCuts.clear();
	hyval=_hyval;
	x_lp=_x_lp;
	y_lp=_y_lp;

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

	if(params.sepcut==1)
	{
		nCcuts=separateCut();
		nCuts+=nCcuts;
	}

	return nCuts;
}

int STPRBHSepFlow::separateCut()
{
	int nCuts=0;
	for (int i = 0; i < nArcs; i++) {
		capacities[i] = x_lp[i]+epsCreep;
	}

	//find all terminals, which can be reached from the root node when considering arcs with x_lp>=limit
	//this way, we may not have to do maxflow to all terminals, which should offer a good speed-up
	//can also be used to speed up the other separations
	const double limit = 1.0-epsInt;
	node v, w;
	edge e;
	NodeArray<bool> found(G->G, false);
	Stack<node> stack;

	found[G->rootNode] = true;
	stack.push(G->rootNode);
	while (!stack.empty()) {
		v = stack.pop();
		// iii: outgoing
		forall_adj_edges(e, v) {
			w = e->opposite(v);
			//cerr<<x_lp[G->x_id[e]]<<endl;
			if(e->source() == v && x_lp[G->x_id[e]] >= limit) {
				if(!found[w]) {
					found[w] = true;
					stack.push(w);
					//cerr<<e->target()<<G->myNodeTerminal[e->target()]<<endl;
				}
			}
		}
	}

	double flow = 0;
	double tolerance=threshold;

	Terminals.permute();
	for(node t: Terminals)
	{

		if(found[t])
		{
			//cerr<<"terminal reachable, separation not necessary for "<<t<<endl;
			continue;
		}


		if (y_lp[G->y_id[t]] < tolerance)
			continue;

		if(nCuts>params.nestedcuts)
			break;


		//cerr<<"node"<<i<<endl;
		for(int k=0;k<(params.nestedcuts-nCuts);k++)
		{
			myFlow->update(G->y_id[G->rootNode],G->y_id[t],capacities);
			flow=myFlow->min_cut(y_lp[G->y_id[t]]+2*(nArcs+1)*(epsOpt+epsCreep),cut);

			if (flow+epsInt<y_lp[G->y_id[t]])
			{
				//cut[node]=1 for source side, 2 for sink side, and 0, if both sides are possible
				//forwardcut=source (root) side cut, backcut=sink (terminal)

				//sum of prizes not in forwardcut/backcut
				double sumNotInF=0;
				double sumNotInB=0;
				bool adds=false;
				node m;
				node rhs=t;

				forall_nodes(m,G->G)
				{

					if(cut[G->y_id[m]]==1)
					{
						sumNotInF+=G->myNodeWeights[m];
						sumNotInB+=G->myNodeWeights[m];
					}
					if(cut[G->y_id[m]]==0)
					{
						//we have at least one node, which can be on both sides, thus forwardcut!=backcut
						adds=true;
						sumNotInB+=G->myNodeWeights[m];
					}

				}

				IloExpr myCut(env);
				IloExpr myCut2(env);

				forall_edges(e,G->G)
				{
					if(cut[G->y_id[e->source()]]==1 && cut[G->y_id[e->target()]]!=1)
					{
						myCut+=G->xvar[e];
					}

					if(cut[G->y_id[e->source()]]!=2 && cut[G->y_id[e->target()]]==2)
					{
						myCut2+=G->xvar[e];

						//set it to one for nested
						//we delibaretly do this only for the backcut
						//note that if there is forwardcut==backcut, this if will be entered
						capacities[G->x_id[e]]=1;
					}
				}

				if(params.forwardcuts || !adds) {

					//try to lift
					if(sumNotInF>incObj)
						myCut -= G->yvar[rhs];
					else
					{
						myCut-=1;
					}
					nCuts++;
					myCuts.push_back(IloRange(env, 0, myCut, IloInfinity));
				}

				if(adds && params.backcuts) {
					//try to lift
					if(sumNotInB>incObj)
						myCut2 -= G->yvar[rhs];
					else
					{
						myCut2 -= 1;
					}
					nCuts++;
					myCuts.push_back(IloRange(env, 0, myCut2, IloInfinity));
				}

				myCut.end();
				myCut2.end();
			}
			else
			{
				found[t]=true;
				stack.push(t);
				//add all nodes reachable from terminal we cut to to found
				//we only have to do it at the end, since in the loop we are only concerned with one terminal
				while (!stack.empty()) {
					v = stack.pop();
					forall_adj_edges(e, v) {
						w = e->opposite(v);
						if(	e->source() == v && x_lp[G->x_id[e]] >= limit) {
							if(!found[w]) {
								found[w] = true;
								stack.push(w);
							}
						}
					}
				}
				break;
			}
		}

	}

	return nCuts;
}

int STPRBHSepFlow::separateGHopLink()
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
			//take smaller one of y_s^{i-1} and y_t^i or nothing, if one of the variables do not exist
			//the latter case it the downlifting similar to the one for hoplink etc.
			double sourceH=-1.0;
			double targetH=-1.0;
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

		if(lhs>rhs+epsInt)
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

int STPRBHSepFlow::separateNodeArcCut()
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

				if(toAdd+epsOpt>=x_lp[G->x_id[e]])
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

			if(lhs+epsInt<rhs)
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

int STPRBHSepFlow::separateHopEnd()
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


		if(sum>epsInt)
		{
			myCuts.push_back(IloRange(env, -IloInfinity, myHopArc, 0));
			nCuts++;
		}
		myHopArc.end();
	}

	return nCuts;
}

int STPRBHSepFlow::separateHopLink()
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
				sum+=hyval(k,e->source());
				myHopArc+=G->hyvar(k,e->source());
			}


			if(sum>epsInt)
			{
				//cout<<myHopArc<<endl;
				myCuts.push_back(IloRange(env, -IloInfinity, myHopArc, 0));
				nCuts++;
			}
			myHopArc.end();

		}
	}

	return nCuts;
}

int STPRBHSepFlow::separateFlowBalanceHop()
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
				if(toAdd>x_lp[G->x_id[e]]+epsOpt)
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
			if(lhs+epsInt<rhs)
			{
				//cerr<<lhs<<" "<<rhs<<endl;
				//cerr<<flowBalance<<endl;
				myPurgeableCuts.push_back(IloRange(env, -IloInfinity, flowBalance, 0));
				nCuts++;
			}
			flowBalance.end();
		}
	}

	return nCuts;
}

int STPRBHSepFlow::separateFlowBalance()
{
	int nCuts=0;
	node v;
	edge e;
	forall_nodes(v,G->G)
	{
		double lhs= y_lp[G->y_id[v]];
		cerr<<lhs<<endl;
		if(lhs>epsInt && !G->myNodeTerminal[v])
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
			if(outgoing+epsInt<lhs)
			{
				myPurgeableCuts.push_back(IloRange(env, -IloInfinity, flowBalance, 0));
				nCuts++;
			}
			flowBalance.end();
		}
	}
	return nCuts;
}

int STPRBHSepFlow::separateOddTwoHopLink()
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

		if(lhs>rhs+epsInt)
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

int STPRBHSepFlow::separateEvenTwoHopLink()
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

		if(lhs>rhs+epsInt)
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


int STPRBHSepFlow::separateCover()
{
	int nCuts=0;
	vector<CoverElements> myCoverVector;
	edge e;
	forall_edges(e,G->G)
	{
		double w=(1-x_lp[G->x_id[e]])/G->myArcWeights[e];
		CoverElements entry=CoverElements(e,w);
		myCoverVector.push_back(entry);
	}

	sort(myCoverVector.begin(),myCoverVector.end());

	List<edge> myCover;
	EdgeArray<bool> coverEdges(G->G, false);
	EdgeArray<bool> removedEdges(G->G, false);
	double b=G->budget;
	double aStar=b;
	double inCover=0;
	double sumCover=0;
	int elements=0;
	double maxWeight=-1;
	bool found=false;
	do
	{
		found=false;
		ListIterator<edge> toDel=nullptr;
		maxWeight=-1;
		for(unsigned i=0;i<myCoverVector.size();i++)
		{
			if(G->myArcWeights[myCoverVector[i].e]>aStar)
				continue;
			if(coverEdges[myCoverVector[i].e])
				continue;
			if(removedEdges[myCoverVector[i].e])
				continue;
			myCover.pushBack(myCoverVector[i].e);
			inCover+=G->myArcWeights[myCoverVector[i].e];
			sumCover+=x_lp[G->x_id[myCoverVector[i].e]];
			elements++;
			coverEdges[myCoverVector[i].e]=true;
			found=true;
			//cerr<<inCover<<" "<<sumCover<<" "<<elements<<" "<<myCoverVector[i].weight<<endl;
			//if(i==myCoverVector.size())
			//	found=false;
			if(inCover>b)
				break;
		}
		if(!found)
			break;

		double sumCoverE=sumCover;

		IloExpr cover(env);

		//int counter=0;
		for(ListIterator<edge> it=myCover.begin();it!=myCover.end();it++)
		{
			//cerr<<(*it)<<" "<<myCover.size()<<" "<<counter<<endl;
			cover+=G->xvar[*it];
			if(G->myArcWeights[*it]>maxWeight && coverEdges[*it])
			{
				maxWeight=G->myArcWeights[*it];
				toDel=it;
			}
			//counter++;
		}

		aStar=maxWeight;

		forall_edges(e, G->G)
		{
			if(G->myArcWeights[e]>=aStar && (!coverEdges[e] || removedEdges[e]))
			{
				sumCoverE+=x_lp[G->x_id[e]];
				cover += G->xvar[e];
				//myCover.pushBack(e);
			}
		}

		//cerr<<sumCoverE<<" "<<(elements-1)<<endl;

		if(sumCoverE>elements-1+1e03)
		{
			myPurgeableCuts.push_back(IloRange(env,-IloInfinity, cover, elements-1));
			nCuts++;
			//cerr<<cover<<endl;
		}
		cover.end();

		//cerr<<(*toDel)<<endl;
		coverEdges[*toDel]=false;
		removedEdges[*toDel]=true;
		inCover-=aStar;
		elements--;
		sumCover-=x_lp[G->x_id[*toDel]];
		myCover.del(toDel);
		//cerr<<sumCover<<" "<<aStar<<" "<<(elements-1)<<" "<<inCover<<" "<<elements<<" "<<myCover.size()<<endl;

	}while(found);

	return nCuts;
}
