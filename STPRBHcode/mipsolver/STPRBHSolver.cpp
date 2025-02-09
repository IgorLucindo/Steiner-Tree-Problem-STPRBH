/*
 * STPRBHSolver.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: markus
 */

#include "STPRBHSolver.h"
#include <thread>
#include "utility/Timer.hpp"
#include "mipsolver/STPRBHSepFlow.h"
#include "mipsolver/STPRBHLazy.h"
#include "mipsolver/STPRBHUser.h"
#include <sstream>
#include <boost/filesystem.hpp>

STPRBHSolver::STPRBHSolver(STPInstance* _inst):
	env(IloEnv()),
	myModel(IloModel(env)),
	cplex(IloCplex(myModel)),
	inst(_inst),
	G(&(inst->basicG)),
	incumbent(new Solution(G)),
	lazySol(new Solution(G)),
	nNodes(G->G.numberOfNodes()),
	nArcs (G->G.numberOfEdges()),
	nTerminals(G->Terminals.size()),
	nodeID(0),
	x(IloNumVarArray(env, nArcs, 0, 1, IloNumVar::Bool)),
	y(IloNumVarArray(env, nNodes, 0, 1, IloNumVar::Bool)),
	x_lp(IloNumArray(env, nArcs)),
	y_lp(IloNumArray(env, nNodes)),
	timeBest(-1),
	rootBound(std::numeric_limits<double>::max()),
	oldObj(0),
	initialized(false),
	lazyImproved(false),
	lbOld(std::numeric_limits<double>::max()),
	lbCurrent( std::numeric_limits<double>::max()),
	nHoplink(0),
	nHopend(0),
	nNodearccut(0),
	nFlowbalance(0),
	nHopflowbalance(0),
	nOtahoplink(0),
	nEtahoplink(0),
	nCcuts(0),
	nGhoplink(0),
	bestBound(0)
{
	initialized = true;
}

STPRBHSolver::~STPRBHSolver()
{

	if(initialized) {
		lazy.end();
		user.end();
		heur.end();
		env.end();
	}

	delete incumbent;
	delete lazySol;
}

void STPRBHSolver::addMIPStart(Solution* s, const char* name)
{
	IloNumVarArray vars(env);
	IloNumArray vals(env);

	edge e;
	forall_edges(e, G->G) {
		vars.add(G->xvar[e]);
		vals.add(s->getXv(e));
		//if(s->getXv(e)>0.5)
		//	cerr<<e<<" "<<s->getXv(e)<<endl;
	}

	node n;
	forall_nodes(n, G->G) {
		vars.add(G->yvar[n]);
		vals.add(s->getYv(n));
		//if(s->getYv(n)>0.5)
		//	cerr<<n<<" "<<s->getYv(n)<<endl;
	}

	forall_nodes(n,G->G)
	{
		if(n==G->rootNode)
			continue;
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(!G->hyvar.isDefined(j,n))
				continue;

			vars.add(G->hyvar(j,n));
			vals.add(s->getHyv(j,n));

			//if(s->getHyv(j,n)>0.5)
			//	cerr<<j<<" "<<n<<" "<<s->getHyv(j,n)<<endl;

			//if(val>0.5)
			//	cerr<<j<<" "<<n<<" "<<val<<endl;
			//if(y_lp[G->y_id[n]]>0.5)
			//	cerr<<j<<" "<<n<<" "<<val<<endl;
		}
	}

	if (name == nullptr) {
		cplex.addMIPStart(vars, vals, IloCplex::MIPStartAuto, "myMIPStart");
	} else {
		cplex.addMIPStart(vars, vals, IloCplex::MIPStartAuto, name);
	}

	vars.end();
	vals.end();
}


bool STPRBHSolver::solve()
{

	prepareDIMACSOutputBegin();

	Timer modelTimer(true);

	createModel();

	modelTimer.stop();
	modelTimer.registerTime(C_INITIALIZATION);

	Timer solveTimer(true);

	if(params.lprelaxation)
		solveLPRelaxation();
	else
		solveBaC();
	solveTimer.stop();

	solveTimer.registerTime(C_CPLEX);

	prepareDIMACSOutputEnd();

	return true;
}

void STPRBHSolver::prepareDIMACSOutputBegin()
{
	dimacsf<<"SECTION Comment"<<endl;
	dimacsf<<"Name "<<inst->Name<<endl;
	dimacsf<<"Problem STPRBH"<<endl;
	dimacsf<<"Program ViennaNodehopper"<<endl;
	dimacsf<<"Version 2.0.0408beta"<<endl;

	dimacsf<<"End"<<endl<<endl;

	dimacsf<<"SECTION Solutions"<<endl;
}

void STPRBHSolver::prepareDIMACSOutputEnd()
{
	dimacsf<<"End"<<endl<<endl;

	dimacsf<<"SECTION Run"<<endl;
	dimacsf<<"Threads "<<params.threads<<endl;
	dimacsf<<"Time "<<Timer::total.elapsed().getSeconds()<<endl;
	dimacsf<<"Dual "<<cplex.getBestObjValue() <<endl;
	dimacsf<<"Primal "<<cplex.getObjValue()<<endl;
	dimacsf<<"End"<<endl<<endl;

	dimacsf<<"SECTION Finalsolution"<<endl;
	dimacsf<<incumbent->printDimacs();
	dimacsf<<"End"<<endl;
}

Solution STPRBHSolver::getSolution()
{
	return *incumbent;
}

bool STPRBHSolver::solveHeur()
{
	STPRBHHeuristics myHeur(G);
	myHeur.PrimI(incumbent);

	dimacsf<<"Solution "<<Timer::total.elapsed().getSeconds()<<" "<<incumbent->getObjective()<<endl;

	return true;
}

bool STPRBHSolver::solveLPRelaxation()
{
	cplex.extract(myModel);

	if(params.cplexexportmodels)
		cplex.exportModel("stprbh.lp");

	STPRBHSepFlow mySeparation=STPRBHSepFlow(G,env,x, y);
	int nCuts=0;

	double boundPrev=std::numeric_limits<double>::max();

	do
	{

		//+3 seconds to have some buffer to account for reading etc in total timer
		long time=params.timelimit-Timer::total.elapsed().getSeconds()+3;
		if(time<0)
			break;
		cplex.setParam(IloCplex::TiLim, time);
		cplex.solve();

		cplex.getValues(x,x_lp);
		cplex.getValues(y,y_lp);
		HashArray2D<int, node, double> yh_lp=HashArray2D<int, node, double>();
		node v;
		forall_nodes(v, G->G) {
			if(v==G->rootNode)
				continue;
			for(int j=1;j<=G->hoplimit;j++)
			{
				if(G->hyvar.isDefined(j,v))
					yh_lp(j,v)=cplex.getValue(G->hyvar(j,v));
			}
		}

		if (cplex.getStatus() == IloAlgorithm::Optimal)
		{
			nCuts=mySeparation.separation(x_lp,y_lp,0,yh_lp);
			//cout<<cplex.getBestObjValue()<<" "<<cplex.getObjValue()<<endl;
			bestBound=cplex.getObjValue();

			cout<<boundPrev<<" "<<bestBound<<endl;
			if(params.septailoff>0.0 && boundPrev-bestBound<params.septailoff)
			{
				cout<<"Tailing off, thus stop"<<endl;
				break;
			}

			boundPrev=bestBound;
			if(nCuts>0)
			{
				vector<IloRange> myCuts=mySeparation.getCuts();
				for(IloRange cut:myCuts)
				{
					myModel.add(cut);
				}

				vector<IloRange> myPurgeableCuts=mySeparation.getPurgeableCuts();
				for(IloRange cut:myPurgeableCuts)
				{
					myModel.add(cut);
				}

				nHoplink+=mySeparation.getNHoplink();
				nHopend+=mySeparation.getNHopend();
				nNodearccut+=mySeparation.getNNodearccut();
				nFlowbalance+=mySeparation.getNFlowbalance();
				nHopflowbalance+=mySeparation.getNHopflowbalance();
				nOtahoplink+=mySeparation.getNOtahoplink();
				nEtahoplink+=mySeparation.getNEtahoplink();
				nCcuts+=mySeparation.getNCcuts();
				nGhoplink+=mySeparation.getNGhoplink();
			}
		}
		else
		{
			cout<<"ERROR: CPLEX could not solve all LP-relaxations, due to "<<cplex.getStatus()<<endl;
			break;
		}
	}while(nCuts>0);

	setStatistics();

	if (params.writesol)
		writeSolution();

	return true;

}

void STPRBHSolver::createModel()
{
	createVariables();
	createNACoupling();
	createHopCoupling();
	createRootOutgoing();
	createRootLayerOneHopCoupling();
	createBudget();
	if(params.sephoplink==0)
		createHopLink();
	if(params.sephopend==0)
		createHopEnd();
	if(params.sepetahoplink==0)
		createEthalink();
	if(params.sepotahoplink==0)
		createOthalink();
	if(params.sepflow==0)
		createFlowConservation();
	if(params.sepgsecsizetwo)
		createGSECSizeTwo();
	createObjective();

	setCPLEXParameters();
}


bool STPRBHSolver::solveBaC()
{

	if(params.heurisitc)
	{
		solveHeur();
	}

	lazy = cplex.use(new (env)STPRBHLazy(env, this));
	if(params.usercut)
		user = cplex.use(new (env)STPRBHUser(env, this));
	if(params.heurisitc)
		heur = cplex.use(new (env)STPRBHHeurCallback(env, this));

	cplex.extract(myModel);

	if(params.cplexexportmodels)
		cplex.exportModel("stprbh.lp");

	if (incumbent != nullptr)
		addMIPStart(incumbent, "ILP MIPStart");


	//first, solve only root node to get the root bound for output
	cplex.setParam(IloCplex::NodeLim,1);

	cplex.solve();
	rootBound=cplex.getBestObjValue();

	//if not solved to optimality in root, continue

	//+3 seconds to have some buffer to account for reading etc in total timer
	long time=params.timelimit-Timer::total.elapsed().getSeconds()+3;
	if(!params.onlyroot && cplex.getStatus() != IloAlgorithm::Optimal && time>0)
	{
		cplex.setParam(IloCplex::TiLim, time);
		cplex.setParam(IloCplex::NodeLim,9999999999);
		cplex.solve();
	}

	if (cplex.getStatus() == IloAlgorithm::Feasible || cplex.getStatus() == IloAlgorithm::Optimal) {
		timeBest=incumbent->getFindtime();
		bestBound=cplex.getBestObjValue();
		setSolution();
		setStatistics();

		if (params.writesol)
			writeSolution();

		return true;
	}

	return false;
}


void STPRBHSolver::writeSolution()
{
	if (params.writesol) {
		std::string filename(boost::filesystem::path(params.file).stem().string());
		stringstream solname;
		solname << filename << "-" << incumbent->getObjective() << ".sol";
		LOG(debug) << "write solution " << solname.str() << endl;
		cplex.writeSolution(solname.str().c_str());
	}
}


void STPRBHSolver::setStatistics()
{

	std::string filename(boost::filesystem::path(params.file).stem().string());
	stringstream solname;

	statf << filename <<",";
	statf <<Timer::total.elapsed().getSeconds()<<",";
	statf <<timeBest<<",";
	streamsize defaultPre=statf.precision();
	statf.precision(10);
	statf << cplex.getObjValue() <<",";
	statf << bestBound <<",";
	statf << rootBound <<",";
	statf.precision(defaultPre);
	statf << cplex.getNnodes() <<",";
	statf << cplex.getNcuts(IloCplex::CutUser) <<",";
	statf << cplex.getNcuts(IloCplex::CutFrac) <<",";
	statf << cplex.getNcuts(IloCplex::CutZeroHalf) <<",";
	statf << cplex.getNcuts(IloCplex::CutCover) <<",";
	statf <<  nHoplink<<",";
	statf <<  nHopend<<",";
	statf <<  nNodearccut<<",";
	statf <<  nFlowbalance<<",";
	statf <<  nHopflowbalance<<",";
	statf <<  nOtahoplink<<",";
	statf <<  nEtahoplink<<",";
	statf <<  nCcuts<<",";
	statf <<  nGhoplink<<",";
	statf << cplex.getStatus() <<",";
	statf << params.sepflow << ",";
	statf << params.sepflowhop << ",";
	statf << params.sepnodearccut << ",";
	statf << params.sepotahoplink << ",";
	statf << params.sepetahoplink << ",";
	statf << params.sephoplink << ",";
	statf << params.sephopend << ",";
	statf << params.sepghoplink << ",";
	statf << params.sepcover << ",";
	statf << params.sepgsecsizetwo << ",";
	statf << params.sepcut << ",";
	statf << params.septhreshold << ",";
	statf << params.septailoff << ",";
	statf << params.creepflow << ",";
	statf << params.nestedcuts << ",";
	statf << params.backcuts << ",";
	statf << params.forwardcuts;
}

void STPRBHSolver::setSolution()
{
	cplex.getValues(x_lp, x);
	cplex.getValues(y_lp, y);

	edge e;
	forall_edges(e,G->G)
	{
		if(x_lp[G->x_id[e]]>0.5)
			x_lp[G->x_id[e]]=1;
		else
			x_lp[G->x_id[e]]=0;
	}


	node n;
	forall_nodes(n,G->G)
	{
		if(y_lp[G->y_id[n]]>0.5)
			y_lp[G->y_id[n]]=1;
		else
			y_lp[G->y_id[n]]=0;
	}


	HashArray2D<int, node, double> hyv;

	forall_nodes(n,G->G)
	{
		if(n==G->rootNode)
			continue;
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(!G->hyvar.isDefined(j,n))
				continue;
			hyv(j,n)=cplex.getValue(G->hyvar(j,n));
			//if(hyv(j,n)>0.5)
			//	cerr<<j<<" "<<n<<endl;
			//if(val>0.5)
			//	cerr<<j<<" "<<n<<" "<<val<<endl;
			//if(y_lp[G->y_id[n]]>0.5)
			//	cerr<<j<<" "<<n<<" "<<val<<endl;
		}
	}

	incumbent->createSolution(x_lp, y_lp, hyv,Timer::total.elapsed().getSeconds(), cplex.getBestObjValue());
	solf<<incumbent->print();
}

//Coupling Constraints
void STPRBHSolver::createNACoupling()
{
	node v;
	edge e;
	forall_nodes(v, G->G) {
		if (v == G->rootNode) {
			continue;
		}

		IloExpr myCoupl(env);
		forall_adj_edges(e, v) {
			if (e->source() == v)
				continue;
			myCoupl += G->xvar[e];
		}
		stringstream myname;
		myname << "coupl_" << v->index();
		myCoupl -= G->yvar[v];

		myModel.add(myCoupl == 0).setName(myname.str().c_str());
		myCoupl.end();
	}
}

void STPRBHSolver::createHopCoupling()
{
	node v;
	forall_nodes(v, G->G) {
		if (v == G->rootNode) {
			continue;
		}

		IloExpr myCoupl(env);
		for(int j=1;j<=G->hoplimit;j++)
		{
			if(G->hyvar.isDefined(j,v))
				myCoupl+=G->hyvar(j,v);
		}
		stringstream myname;
		myname << "hopcoupl_" << v->index();
		myCoupl -= G->yvar[v];

		//cerr<<myCoupl<<endl;
		myModel.add(myCoupl == 0).setName(myname.str().c_str());
		myCoupl.end();
	}
}

void STPRBHSolver::createRootLayerOneHopCoupling()
{
	edge e;
	forall_adj_edges(e,G->rootNode)
	{
		if(e->target()==G->rootNode)
			continue;
		IloExpr myHopArc(env);
		myHopArc+=G->xvar[e];
		//cerr<<e<<endl;
		myHopArc-=G->hyvar(1,e->target());
		stringstream myname;
		myname << "hoproot_"<<e->target()->index() <<"h1";
		myModel.add(myHopArc ==0).setName(myname.str().c_str());
		myHopArc.end();
	}
}

void STPRBHSolver::createRootOutgoing()
{
	edge e;
	//root outgoing>=1
	if(G->rootNode->outdeg()>1)
	{
		IloExpr myOut(env);
		forall_adj_edges(e,G->rootNode)
		{
			if(e->source()==G->rootNode)
				myOut +=G->xvar[e];
		}
		stringstream myname;
		myname << "routgoing";
		myModel.add(myOut >= 1).setName(myname.str().c_str());
		myOut.end();
	}
}

void STPRBHSolver::createBudget()
{
	edge e;
	//budgetlimit
	IloExpr myBudget(env);
	forall_edges(e,G->G)
	{
		myBudget +=(G->myArcWeights[e])*G->xvar[e];
	}
	stringstream myname2;
	myname2 << "budget";
	myModel.add(myBudget <= G->budget).setName(myname2.str().c_str());
	myBudget.end();
}

void STPRBHSolver::createFlowConservation()
{
	node v;
	edge e;
	forall_nodes(v,G->G)
	{
		if(!G->myNodeTerminal[v])
		{
			IloExpr flowBalance(env);

			flowBalance += G->yvar[v];

			forall_adj_edges(e,v)
			{
				if(e->source()==v)
					flowBalance -= G->xvar[e];
			}
			stringstream myname3;
			myname3 << "flowbalance "<<v;
			if(v->outdeg()!=1)
				myModel.add(flowBalance <= 0).setName(myname3.str().c_str());
			else
				myModel.add(flowBalance == 0).setName(myname3.str().c_str());
			flowBalance.end();
		}
	}
}

void STPRBHSolver::createGSECSizeTwo()
{
	edge e;
	forall_edges(e, G->G)
	{
		if(e->source()!=G->rootNode)
		{
			edge f = G->getOppositeArc(e);
			if(f!=nullptr)
				myModel.add(G->xvar[e] + G->xvar[f] <= G->yvar[e->target()]);
			else
			{
				if(e->target()!=G->rootNode)
					myModel.add(G->xvar[e] <= G->yvar[e->source()]);
			}
		}
	}
}

void STPRBHSolver::createHopLink()
{
	edge e;
	forall_edges(e, G->G) {
		if(e->source()==G->rootNode || e->target()==G->rootNode)
			continue;

		//HopLink, note that the lhs is taken as j-1
		int endLayer=min(G->layerEnd[e->source()]+1,G->layerEnd[e->target()]);
		for(int j=G->layer[e->source()]+1;j<=endLayer;++j)
		{

			IloExpr myHopArc(env);

			myHopArc+=G->hyvar(j-1,e->source());


			//lifting
			//int start=max(G->layer[e->source()],G->layerEnd[e->target()]);
			for(int k=G->layerEnd[e->target()] ;k<=G->layerEnd[e->source()];++k)
			{
				//if(!G->hyvar.isDefined(k,e->source()))
				//	continue;
				myHopArc+=G->hyvar(k,e->source());
			}

			myHopArc+=G->xvar[e];

			myHopArc-=G->yvar[e->source()];
			//if(G->hyvar.isDefined(j,e->target()))
			myHopArc-=G->hyvar(j,e->target());

			stringstream myname;
			myname << "hoplink_" << e->source()->index()<<","<<e->target()->index() <<"h"<<j;
			myModel.add(myHopArc <= 0).setName(myname.str().c_str());
			//cerr<<myHopArc<<endl;
			myHopArc.end();
		}
	}
}


void STPRBHSolver::createHopEnd()
{
	edge e;
	forall_edges(e, G->G)
	{
		if(e->source()==G->rootNode || e->target()==G->rootNode || G->layerEnd[e->target()] >G->layerEnd[e->source()])
			continue;

		IloExpr myHopArc(env);


		for(int k=G->layerEnd[e->target()] ;k<=G->layerEnd[e->source()];++k)
		{
			myHopArc+=G->hyvar(k,e->source());
		}

		myHopArc+=G->xvar[e];

		myHopArc-=G->yvar[e->source()];

		stringstream myname;
		myname << "hopend_" << e->source()->index()<<","<<e->target()->index();
		myModel.add(myHopArc <= 0).setName(myname.str().c_str());
		//cerr<<myHopArc<<endl;
		myHopArc.end();
	}
}



void STPRBHSolver::createObjective()
{
	IloExpr objective(env);
	node v;
	forall_nodes(v,G->G)
	{

		objective+=(G->myNodeWeights[v])*(G->yvar[v]);
	}
	myModel.add(IloMaximize(env, objective));
	objective.end();
}

bool STPRBHSolver::setCPLEXParameters()
{

	if (params.threads == 0)
		cplex.setParam(IloCplex::Threads, thread::hardware_concurrency());
	else
		cplex.setParam(IloCplex::Threads, params.threads);

	if(params.cplexoutput == 0) {
		cplex.setOut(env.getNullStream());
		cplex.setWarning(env.getNullStream());
	} else {
		cplex.setParam(IloCplex::MIPInterval, 0);
		cplex.setParam(IloCplex::MIPDisplay, params.cplexoutput);
	}

	if(params.seed!=-1)
		cplex.setParam(IloCplex::RandomSeed,params.seed);


	switch (params.rootAlgorithm) {
	case ALG_Primal:
		cplex.setParam(IloCplex::RootAlg, IloCplex::Primal);
		break;
	case ALG_Dual:
		cplex.setParam(IloCplex::RootAlg, IloCplex::Dual);
		break;
	case ALG_Concurrent:
		cplex.setParam(IloCplex::RootAlg, IloCplex::Concurrent);
		break;
	case ALG_Sifting:
		cplex.setParam(IloCplex::RootAlg, IloCplex::Sifting);
		break;
	case ALG_Barrier:
		cplex.setParam(IloCplex::RootAlg, IloCplex::Barrier);
		break;
	case ALG_Network:
		cplex.setParam(IloCplex::RootAlg, IloCplex::Network);
		break;
	case ALG_Auto:
		cplex.setParam(IloCplex::RootAlg, IloCplex::AutoAlg);
		break;
	default:
		throw invalid_argument("unknown root algorithm");
		break;
	}


	switch (params.nodeAlgorithm) {
	case ALG_Primal:
		cplex.setParam(IloCplex::NodeAlg, IloCplex::Primal);
		break;
	case ALG_Dual:
		cplex.setParam(IloCplex::NodeAlg, IloCplex::Dual);
		break;
	case ALG_Concurrent:
		cplex.setParam(IloCplex::NodeAlg, IloCplex::Concurrent);
		break;
	case ALG_Sifting:
		cplex.setParam(IloCplex::NodeAlg, IloCplex::Sifting);
		break;
	case ALG_Barrier:
		cplex.setParam(IloCplex::NodeAlg, IloCplex::Barrier);
		break;
	case ALG_Network:
		cplex.setParam(IloCplex::NodeAlg, IloCplex::Network);
		break;
	case ALG_Auto:
		cplex.setParam(IloCplex::NodeAlg, IloCplex::AutoAlg);
		break;
	default:
		throw invalid_argument("unknown node algorithm");
		break;
	}
	cplex.setParam(IloCplex::DPriInd, 2);

	cplex.setParam(IloCplex::FracCuts, params.cplexcuts);
	cplex.setParam(IloCplex::ZeroHalfCuts, params.cplexcuts);
	cplex.setParam(IloCplex::Covers,  params.cplexcuts);

	// branch-cut heuristic frequency
	cplex.setParam(IloCplex::HeurFreq, 1);
	cplex.setParam(IloCplex::RINSHeur, 1);
	cplex.setParam(IloCplex::LBHeur, 1);


	cplex.setParam(IloCplex::PreLinear, 1);
	cplex.setParam(IloCplex::Reduce, 1);

	if (params.timelimit > 0) {
		cplex.setParam(IloCplex::TiLim, params.timelimit);
	}

	cplex.setParam(IloCplex::EpGap, params.gap);

	if (inst->isInteger)
		cplex.setParam(IloCplex::EpAGap, 1 - 0.001);


	// branch on node variables first
	node n;
	forall_nodes(n, G->G) {
		cplex.setPriority(G->yvar[n], G->myNodeWeights[n]+G->hoplimit+2);
	}

	// next, on y_h-variables
	node v;
	forall_nodes(v, G->G) {

		for(int j=1;j<=G->hoplimit;j++)
		{
			if(G->hyvar.isDefined(j,v))
				cplex.setPriority(G->hyvar(j,v),G->hoplimit-j+1);
		}
	}

	return true;
}

void STPRBHSolver::createVariables()
{
	int i = 0;
	edge e;
	node v;

	forall_nodes(v, G->G) {
		G->y_id[v] = i;
		stringstream aname;
		if (v!= G->rootNode)
			aname << "y_" << v->index();
		else
			aname << "y_root";
		y[i].setName(aname.str().c_str());
		//cerr<<v<<" "<<G->myNodeTerminal[v]<<G->is_terminal(i)<<endl;
		if (v==G->rootNode) {
			y[i].setLB(1);
		}

		myModel.add(y[i]);
		G->yvar[v] = y[i];
		i++;
	}

	if(params.lprelaxation)
	{
		myModel.add(IloConversion(env,x,ILOFLOAT));
	}

	i = 0;
	forall_edges(e, G->G) {

		G->x_id[e] = i;

		stringstream ss;
		if (e->source() != G->rootNode)
			ss << "x_" << e->source() << "," << e->target();
		else
			ss << "x_root," << e->target();
		x[i].setName(ss.str().c_str());
		G->xvar[e] = x[i];


		myModel.add(G->xvar[e]);

		i++;
	}

	if(params.lprelaxation)
	{
		myModel.add(IloConversion(env,y,ILOFLOAT));
	}


	i=0;
	forall_nodes(v, G->G) {

		if(v==G->rootNode)
			continue;

		for(int j=1;j<=G->hoplimit;j++)
		{
			if(G->layer[v]>j)
				continue;

			if(G->layerEnd[v]<j)
				continue;

			stringstream aname;
			aname << "y_"<<v->index()<<"h"<<j;

			G->hyvar(j,v)=IloBoolVar(env, aname.str().c_str());
			if(params.lprelaxation)
				myModel.add(IloConversion(env,G->hyvar(j,v),ILOFLOAT));

			myModel.add(G->hyvar(j,v));
		}
	}

}

void STPRBHSolver::createEthalink() {
	edge e;
	forall_edges(e, G->G)
	{
		if (e->source()->index() > e->target()->index())
			continue;
		if(e->source()==G->rootNode)
			continue;
		if(e->target()==G->rootNode)
			continue;

		IloExpr jumpE(env);
		IloExpr jumpO(env);

		jumpE += G->xvar[e];
		jumpO += G->xvar[e];
		edge f = G->getOppositeArc(e);
		if(f==nullptr)
			continue;

		jumpE += G->xvar[f];
		jumpO += G->xvar[f];


		node s = e->source();
		node t = e->target();

		for (int i = 2; i <= G->hoplimit; i=i+2)
		{
			if (G->hyvar.isDefined(i, s))
			{
				if (G->hyvar.isDefined(i-1, t) || G->hyvar.isDefined(i+1, t))
				{
					jumpE -= G->hyvar(i, s);
				}
			}
			if (G->hyvar.isDefined(i, t))
			{
				if (G->hyvar.isDefined(i-1, s) || G->hyvar.isDefined(i+1, s))
				{
					jumpE -= G->hyvar(i, t);
				}
			}
		}

		stringstream myname4;
		myname4 << "Ethalink" << e->source()->index()<<","<<e->target()->index();
		myModel.add(jumpE <= 0).setName(myname4.str().c_str());

		jumpE.end();
	}


}

void STPRBHSolver::createOthalink() {
	edge e;
	forall_edges(e, G->G)
	{
		if (e->source()->index() > e->target()->index())
			continue;
		if(e->source()==G->rootNode)
			continue;
		if(e->target()==G->rootNode)
			continue;

		IloExpr jumpO(env);

		jumpO += G->xvar[e];
		edge f = G->getOppositeArc(e);
		if(f!=nullptr)
		{
			jumpO += G->xvar[f];
		}

		node s = e->source();
		node t = e->target();

		for (int i = 1; i <= G->hoplimit; i=i+2)
		{
			if (G->hyvar.isDefined(i, s))
			{
				if (G->hyvar.isDefined(i-1, t) || G->hyvar.isDefined(i+1, t))
				{
					jumpO -= G->hyvar(i, s);
				}
			}
			if (G->hyvar.isDefined(i, t))
			{
				if (G->hyvar.isDefined(i-1, s) || G->hyvar.isDefined(i+1, s))
				{
					jumpO -= G->hyvar(i, t);
				}

			}
		}

		stringstream myname4;
		myname4 << "Othalink-" << e->source()->index()<<","<<e->target()->index();
		myModel.add(jumpO <= 0).setName(myname4.str().c_str());

		jumpO.end();
	}


}
