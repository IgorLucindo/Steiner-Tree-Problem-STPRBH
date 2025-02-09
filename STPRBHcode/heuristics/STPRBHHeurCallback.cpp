/*
 *  \brief
 *  \details
 */

#include "STPRBHHeurCallback.h"
#include "utility/Timer.hpp"


STPRBHHeurCallback::STPRBHHeurCallback(IloEnv _env, STPRBHSolver* _solver) : IloCplex::HeuristicCallbackI(_env),
	env(_env), 
	solver(_solver),
	G(solver->G),
	heur(new STPRBHHeuristics(G)),
	inc(solver->incumbent)
{
	initialize();
}

STPRBHHeurCallback::STPRBHHeurCallback(const STPRBHHeurCallback& _copy) : 	IloCplex::HeuristicCallbackI(_copy.getEnv()),
	env(_copy.env), 
	solver(_copy.solver),
	G(solver->G),
	heur(new STPRBHHeuristics(G)),
	inc(solver->incumbent)
{
	initialize();
}

STPRBHHeurCallback::~STPRBHHeurCallback()
{
	delete heur;
}

inline void STPRBHHeurCallback::initialize()
{
	nNodes = G->G.numberOfNodes();
	nArcs = G->G.numberOfEdges();

	this->x = solver->x;
	this->y = solver->y;
}

void STPRBHHeurCallback::callHeur()
{

	IloNumArray x_lp = IloNumArray(env, nArcs);
	IloNumArray y_lp = IloNumArray(env, nNodes);
	getValues(x_lp, solver->x);
	getValues(y_lp, solver->y);

	vector<double> y_h(nNodes);
	vector<double> x_h(nArcs);

	node v;
	edge e;

	EdgeArray<double> weights=EdgeArray<double>(G->G);
	NodeArray<double> nodeWeights(G->G,1);

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


	//call the heuristic
	heur->PrimSTPRBH(mySolution, weights,nodeWeights, hyval, getBestObjValue());

	//cerr<<mySolution->getObjective()<<endl;



	//check, if we have found a better solution with either the heuristic here, or already in the lazy constraint callback
	//(since due to the way CPLEX works, we only can add heuristic solutions now)
	//some thread-safety stuff is done, and the solution is also written into the DIMACS file
	bool better=false;

	if((mySolution->getObjective()-1e-3 > getIncumbentObjValue() || !hasIncumbent()) )
		better=true;

	boost::unique_lock<decltype(solver->incMutex)> lock(solver->incMutex, boost::defer_lock);
	{
		lock.lock();
		if((solver->lazySol->getObjective()-1e-3 > getIncumbentObjValue() || !hasIncumbent()) && solver->lazySol->getFeas())
		{
			//cerr<<"jhere"<<" "<<solver->lazySol->getObjective()<<" "<<getIncumbentObjValue()<<endl;
			better=true;
		}

		lock.unlock();
	}

	if(better)
	{
		boost::unique_lock<decltype(solver->incMutex)> lock(solver->incMutex, boost::defer_lock);
		{
			lock.lock();
			if(solver->lazySol->getFeas() && solver->lazySol->getObjective()>mySolution->getObjective())
			{
				//cerr<<"jhere"<<" "<<solver->lazySol->getObjective()<<endl;
				(*inc)=*solver->lazySol;
			}
			else
			{
				//cerr<<"jhere"<<" "<<solver->lazySol->getObjective()<<endl;
				(*inc)=*mySolution;
				solver->dimacsf<<"Solution "<<Timer::total.elapsed().getSeconds()<<" "<<inc->getObjective()<<endl;
				solver->solf<<inc->print()<<endl;

				if (!params.outputfile3.empty())
				{
				ofstream os(params.outputfile3);
				os<<inc->print()<<endl;
				os.close();
				}
			}
			cout << "primal " << setprecision(10) << (*inc).getObjective() << " > " << getIncumbentObjValue() << endl;
			lock.unlock();
		}


		IloNumArray solVal(env);
		IloNumVarArray solVar(env);

		double obj = 0.0;
		forall_nodes(v, G->G)
		{

			if(inc->getYv(v) > 0.5) {
				obj += G->myNodeWeights[v];
			}

			solVar.add(G->yvar[v]);
			solVal.add(inc->getYv(v));
//			if(mySolution->getYv(v)>0.5)
//				cerr<<v<<" "<<inc->getYv(v)<<endl;

		}

		forall_edges(e, G->G)
		{

			solVar.add(G->xvar[e]);
			solVal.add(inc->getXv(e));

//			if(inc->getXv(e)>0.5)
//				cerr<<e<<" "<<inc->getXv(e)<<endl;
		}

		node n;
		forall_nodes(n,G->G)
		{
			if(n==G->rootNode)
				continue;
			for(int j=1;j<=G->hoplimit;j++)
			{
				if(!G->hyvar.isDefined(j,n))
					continue;

				solVar.add(G->hyvar(j,n));
				solVal.add(inc->getHyv(j,n));
//				if(inc->getHyv(j,n)>0.5)
//					cerr<<j<<" "<<n<<" "<<inc->getHyv(j,n)<<endl;

				//if(val>0.5)
				//	cerr<<j<<" "<<n<<" "<<val<<endl;
				//if(y_lp[G->y_id[n]]>0.5)
				//	cerr<<j<<" "<<n<<" "<<val<<endl;
			}
		}

		//cerr<<obj<<endl;
		setSolution(solVar, solVal, obj);
		solVal.end();
		solVar.end();
	}

	delete mySolution;


	//cerr<<mySolution->print()<<endl;

	x_lp.end();
	y_lp.end();
}

void STPRBHHeurCallback::main()
{
	inc = solver->incumbent;
	callHeur();
}
