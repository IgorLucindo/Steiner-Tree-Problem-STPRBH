/*
 * STPRBHLazy.cpp
 *
 *  Created on: Aug 24, 2012
 *      Author: markus
 */


#include "STPRBHLazy.h"
#include "utility/Timer.hpp"
#include <limits>


STPRBHLazy::STPRBHLazy(IloEnv _env, STPRBHSolver* _solver):
IloCplex::LazyConstraintCallbackI(_env),
env(_env), solver(_solver),
incObj(std::numeric_limits<double>::max())
{
	//cerr << "lazyConstructor" << endl;
	initialize();
}

STPRBHLazy::~STPRBHLazy()
{
	//cerr<<"deleteInLazy "<<separatorFlow<<endl;
	delete separatorCon;
	delete heur;

}

inline void STPRBHLazy::initialize()
{
	G=solver->G;
	heur=new STPRBHHeuristics(G);
	nNodes=G->G.numberOfNodes();
	nArcs =G->G.numberOfEdges();
	nTerminals=G->Terminals.size();

	this->x = solver->x;
	this->y = solver->y;

	separatorCon = new STPRBHSepCon(solver->G, _env, solver->x, solver->y, true);
}

STPRBHLazy::STPRBHLazy(const STPRBHLazy& _copy):
				IloCplex::LazyConstraintCallbackI(_copy.getEnv()),
				env(_copy.env),
				solver(_copy.solver),
				incObj(std::numeric_limits<double>::max())
{
	//cerr << "lazyCopyConstructor" << endl;
	initialize();
}

void STPRBHLazy::main()
{

	Timer lT(true);

	IloNumArray x_lp = IloNumArray(env, nArcs);
	IloNumArray y_lp = IloNumArray(env, nNodes);
	getValues(x_lp, x);
	getValues(y_lp, y);

	//cerr<<getObjValue()<<endl;


	//cerr<<"callSeparation"<<endl;

	HashArray2D<int, node, double> yh_lp=HashArray2D<int, node, double>();
	node v;
	forall_nodes(v, G->G) {
		if(v==G->rootNode)
			continue;
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(G->hyvar.isDefined(j,v))
				yh_lp(j,v)=getValue(G->hyvar(j,v));
		}
	}


	double incObj=0;
	if(hasIncumbent())
		incObj=getIncumbentObjValue();

	if(separatorCon->separation(x_lp, y_lp, incObj, yh_lp)>0)
	{
		vector<IloRange> myCuts=separatorCon->getCuts();

		//cerr<<"lazy "<<myCuts.size()<<endl;
		for(u_int i = 0; i < myCuts.size(); ++i) {
			add(myCuts.at(i));
			myCuts.at(i).end();
		}

		vector<IloRange> myPurgeableCuts=separatorCon->getPurgeableCuts();
		//cerr<<"lazy purge "<<myPurgeableCuts.size()<<endl;

		for(u_int i = 0; i < myPurgeableCuts.size(); ++i) {
			add(myPurgeableCuts.at(i),IloCplex::UseCutPurge);
			myPurgeableCuts.at(i).end();
		}

		solver->nHoplink+=separatorCon->getNHoplink();
		solver->nHopend+=separatorCon->getNHopend();
		solver->nNodearccut+=separatorCon->getNNodearccut();
		solver->nFlowbalance+=separatorCon->getNFlowbalance();
		solver->nHopflowbalance+=separatorCon->getNHopflowbalance();
		solver->nOtahoplink+=separatorCon->getNOtahoplink();
		solver->nEtahoplink+=separatorCon->getNEtahoplink();
		solver->nCcuts+=separatorCon->getNCcuts();
		solver->nGhoplink+=separatorCon->getNGhoplink();
	}



	Solution mySol=Solution(G);
	vector<double> y_lp2=vector<double>(G->G.numberOfNodes(),0);
	vector<double> x_lp2=vector<double>(G->G.numberOfEdges(),0);

	for(int i=0;i<G->G.numberOfNodes();i++)
		y_lp2[i]=y_lp[i];
	for(int i=0;i<G->G.numberOfEdges();i++)
		x_lp2[i]=x_lp[i];

	mySol.createSolution(x_lp2,y_lp2,Timer::total.elapsed().getSeconds(),incObj);

	//cerr<<mySol.getObjective()<<endl;

	EdgeArray<double> weights=EdgeArray<double>(G->G);
	NodeArray<double> nodeWeights(G->G,1);

	edge e;
	forall_edges(e, G->G)
	{
		weights[e] = G->myArcWeights[e] * (1 - x_lp[G->x_id[e]]);
	}

	Solution* mySolution=new Solution(G);

	HashArray2D<int, node, double> hyval=HashArray2D<int, node, double>();

	forall_nodes(v, G->G) {
		nodeWeights[v]=y_lp[G->y_id[v]];
		if(v==G->rootNode)
			continue;
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(G->hyvar.isDefined(j,v))
				hyval(j,v)=getValue(G->hyvar(j,v));
		}
	}


	heur->PrimSTPRBH(mySolution, weights,nodeWeights, hyval, getBestObjValue());
	//cerr<<"laaazy "<<mySolution->getObjective()<<endl;
	//cerr<<mySolution->getObjective()<<endl;

	if(mySolution->getObjective()>mySol.getObjective() || !mySol.getFeas())
	{
		mySol=*mySolution;
	}

	delete mySolution;


	boost::unique_lock<decltype(solver->incMutex)> lock(solver->incMutex, boost::defer_lock);
	{
		lock.lock();
		if(mySol.getFeas() && mySol.getObjective()>incObj && mySol.getObjective()>solver->lazySol->getObjective())
		{
			solver->lazyImproved=true;
			(*solver->lazySol)=mySol;
			solver->dimacsf<<"Solution "<<Timer::total.elapsed().getSeconds()<<" "<<mySol.getObjective()<<endl;
			solver->solf<<mySol.print()<<endl;

			if (!params.outputfile3.empty())
			{
			    ofstream os(params.outputfile3);
			    os<<mySol.print()<<endl;
			    os.close();
			}
		}

		lock.unlock();
	}
	x_lp.end();
	y_lp.end();


	//cerr << "number of added cuts lazy "<<myCuts.size() << endl;


	lT.stop();
	lT.registerTime(C_MIP_LCallback);

}
