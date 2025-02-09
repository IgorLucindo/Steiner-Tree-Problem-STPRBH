/*
 * Solution.h
 *
 *  Created on: Mar 30, 2014
 *      Author: markus
 */

#ifndef SOLUTION_H_
#define SOLUTION_H_

#include "DiGraph.h"


/**
 * \brief Class to store (feasible) solutions. Contains feasibility check.
 */
class Solution
{
	DiGraph* G;
	Graph sol;

	int nNodes;
	int nArcs;
	int nArcsRoot;

	EdgeArray<double> xv;
	NodeArray<double> yv;
	HashArray2D<int, node, double> hyv;

	double obj;

	double findtime;
	double ub;
	double budget;
	int hops;


	bool feas;
	int constructionID;


	bool integer;
	bool created;

public:
	Solution();

	Solution(DiGraph* G);
	virtual ~Solution();

	Solution& operator=(const Solution& s)
	{
		//cerr<<"here"<<endl;
		G = s.G;
		xv= s.xv;

		yv = s.yv;
		hyv= s.hyv;
		//cout << "COPY " << obj << " " << s.obj << endl;
		obj = s.obj;
		ub = s.ub;

		findtime=s.findtime;
		budget=s.budget;
		hops=s.hops;

		sol=s.sol;

		nNodes=s.nNodes;
		nArcs=s.nArcs;
		nArcsRoot=s.nArcsRoot;

		root = s.root;
		//cout << "FEAS " << feas << " " << s.feas << endl;
		feas = s.feas;

		return *this;
	}


	int cntSol;


	double computeObjective();

	/**
	 * \brief tries to create solution without being given the y^h-vars, does feasibility checks
	 *
	 * It creates the tree induced by the given values of y and x variables and sets the y^h-variables accordingly.
	 * If the resulting tree execeeds the hop-limit, the solution is of course infeasible.
	 * This is used in the LazyCons-Callback to repair a given (x,y,y^h) with potentially wrong y^h-vars.
	 */
	int createSolution(const vector<double>& x_lp, const vector<double>& y_lp, double findtime, double ub=-1);

	/**
	 * \brief creates solution, does feasibility checks
	 */
	void createSolution(const IloNumArray& x_lp, const IloNumArray& y_lp, HashArray2D<int, node, double> hyvar, double findtime, double ub=1);

	void writeGML();
	void writeAuxFile();
	bool readSolution();
	void clear();

	/**
	 * \brief prints the solution as arclist (format used in the two Fu and Hao-Papers
	 */
	std::string print();
	/**
	 * \brief prints the solution using DIMACS format.
	 */
	std::string printDimacs();


	bool isFeasible() const
	{
		return feas;
	}

	double getObjective() const {
		if (!feas)
			return -1;
		return obj;
	}

	double getBudget() const {
		return budget;
	}

	double getFindtime() const {
		return findtime;
	}

	int getHops() const {
		return hops;
	}

	int getFeas() const {
		return feas;
	}

	double getUb() const {
		return ub;
	}

	double getYv(node n) const {
		return yv[n];
	}

	double getXv(edge e) const{
		return xv[e];
	}

	double getHyv(int layer, node n) {
		if(!G->hyvar.isDefined(layer,n))
			return -1;
		//cerr<<layer<<" "<<n<<" "<<hyv(layer,n)<<endl;
		return hyv(layer,n);
	}

	node root;
};

#endif /* SOLUTION_H_ */
