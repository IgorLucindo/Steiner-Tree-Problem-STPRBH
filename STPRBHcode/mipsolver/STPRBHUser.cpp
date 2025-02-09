/*
 * STPRBHUser.cpp
 *
 *  Created on: Aug 24, 2012
 *      Author: markus
 */


#include <queue>
#include "STPRBHUser.h"
#include "utility/Timer.hpp"



STPRBHUser::STPRBHUser(IloEnv _env, STPRBHSolver *_solver):
IloCplex::UserCutCallbackI(_env),
env(_env),
solver(_solver)
{
	initialize();
}

STPRBHUser::~STPRBHUser()
{

	if(separator) {
		//cerr<<"deleteInUser" << separator<<endl;
		delete separator;
	}
	separator = 0;
}

inline void STPRBHUser::initialize()
{

	G = solver->G;
	nNodes=G->G.numberOfNodes();
	nArcs =G->G.numberOfEdges();
	nTerminals=G->Terminals.size();


	incObj=0;

	this->x = solver->x;
	this->y = solver->y;

	separator = new STPRBHSepFlow(solver->G, _env, solver->x, solver->y, false);
}

STPRBHUser::STPRBHUser(const STPRBHUser& _copy):
		IloCplex::UserCutCallbackI(_copy.getEnv()),
		env(_copy.env),
		solver(_copy.solver)
{
	initialize();
}

void STPRBHUser::main()
{

	if(params.septailoff>0)
	{
		//to be on the safe side, if we use more than 1 thread.
		boost::unique_lock<decltype(solver->incMutex)> lock(solver->incMutex, boost::defer_lock);
		{
			lock.lock();
			{
				solver->lbOld = solver->lbCurrent;
				solver->lbCurrent = getBestObjValue();
				if (solver->lbOld < std::numeric_limits<double>::max()) {

					double improve = 0;

					improve=(solver->lbOld-solver->lbCurrent);

					if (improve < params.septailoff) {
						cerr<<"tailing off, only improved "<<improve<<", thus break"<<endl;
						return;
					}
				}
			}
			lock.unlock();
		}
	}


	Timer uT(true);

	IloNumArray x_lp = IloNumArray(env, nArcs);
	IloNumArray y_lp = IloNumArray(env, nNodes);
	getValues(x_lp, x);
	getValues(y_lp, y);

	if(hasIncumbent())
		incObj=getIncumbentObjValue();


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


	if(separator->separation(x_lp, y_lp, incObj, yh_lp)>0)
	{
		vector<IloRange> myCuts =separator->getCuts();
		//cerr << "number of added cuts user" << " " << myCuts.size() << endl;
		for(u_int i = 0; i < myCuts.size(); ++i) {
			//cerr<<myCuts.at(i)<<endl;
			add(myCuts.at(i),IloCplex::UseCutForce);
			myCuts.at(i).end();
		}

		vector<IloRange> myPurgeableCuts =separator->getPurgeableCuts();
		//cerr << "number of added purgeable cuts user" << " " << myPurgeableCuts.size() << endl;
		for(u_int i = 0; i < myPurgeableCuts.size(); ++i) {
			//cerr<<myPurgeableCuts.at(i)<<endl;
			add(myPurgeableCuts.at(i),IloCplex::UseCutPurge);
			myPurgeableCuts.at(i).end();
		}
		solver->nHoplink+=separator->getNHoplink();
		solver->nHopend+=separator->getNHopend();
		solver->nNodearccut+=separator->getNNodearccut();
		solver->nFlowbalance+=separator->getNFlowbalance();
		solver->nHopflowbalance+=separator->getNHopflowbalance();
		solver->nOtahoplink+=separator->getNOtahoplink();
		solver->nEtahoplink+=separator->getNEtahoplink();
		solver->nCcuts+=separator->getNCcuts();
		solver->nGhoplink+=separator->getNGhoplink();

	}

	x_lp.end();
	y_lp.end();

	uT.stop();
	uT.registerTime(C_MIP_UCallback);

}
