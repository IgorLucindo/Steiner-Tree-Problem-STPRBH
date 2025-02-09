/**
 * \file
 * \author Martin Luipersbeck
 * \date 2012-12-06
 */

#ifndef STPINSTANCE_HPP_
#define STPINSTANCE_HPP_


#include <ogdf/basic/Graph.h>
#include <ogdf/basic/NodeArray.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/NodeSet.h>

#include "instance/DiGraph.h"

typedef double weight;

typedef long int id;


/**
 * \brief Instance class. Contains an object of class DiGraph, and some additional information.
 * Preprocessing and creation of the DiGraph is done here.
 */
class STPInstance
{
public:
	ogdf::Graph G;

	DiGraph basicG;

	/**
	 * \brief creates a directed graph out of an undirected instance. While doing so also incorporates some
	 * final preprocessing steps.
	 *
	 * \param root The root node
	 */
	bool createDiGraphs(ogdf::node root);


	double budget;
	int hoplimit;

	/// Terminals
	ogdf::NodeSet Terminals;

	/// Edge weights
	ogdf::EdgeArray<weight> Weight;

	/// Node prizes
	ogdf::NodeArray<weight> Prize;

	std::string Description;
	std::string Name;
	std::string Date;

	bool isInteger = true;

	/// This is the ID the node got in the corresponding STP file
	ogdf::NodeArray<id> NodeId;

	/// This is the ID of the Edge in the corresponding STP file
	/// unused, unless for reconstruction purposes, it is just an
	/// indication of the order in the corresponding STP file
	ogdf::EdgeArray<id> EdgeId;

	/// This is an internal Id, unique to instantiation
	id Id;

	/// Root node as defined in the STP file,
	ogdf::node Root = nullptr;

	STPInstance ();
	STPInstance (const STPInstance& inst);
	virtual ~STPInstance ();

	bool operator == (const STPInstance& rhs) const;


	/// \brief checks consistency, connectivity and simplicity
	bool valid () const;

	inline bool isTerminal (ogdf::node v) const
	{
		return Terminals.isMember(v);
	}

private:
	NodeArray<int> layerStart;
	NodeArray<int> layerEnd;

	NodeArray<int> rootCost;

	/**
	 * \brief preprocessing routine, as described in the paper. Some steps are also done in the method createDiGraphs
	 *
	 * \param root The root node
	 */
	bool preprocessing(node root);

};

#endif // STPINSTANCE_HPP_
