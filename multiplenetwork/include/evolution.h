/**
 * This module defines a generic network co-evolution process.
 *
 * The function "evolve" takes a multiplex network as input and at every step
 * updates each of its networks taking one of the following actions:
 * 1) no action (the network remains unchanged - used to set different evolution speeds)
 * 2) internal evolution (the network evolves according to some internal dynamics)
 * 3) external evolution (the network imports vertexes and edges from another)
 */

#ifndef EVOLUTION_H_
#define EVOLUTION_H_

#include <set>
#include <vector>
#include "evolution.h"
#include "datastructures.h"
#include "utils.h"


typedef int evolution_strategy;

const int EVOLUTION_DEGREE=0;

/**********************************************************************/
/** edge closure strategy *********************************************/
/**********************************************************************/
entity_id choice_uniform(Random rand, MultiplexNetwork& mnet);
vertex_id choice_uniform(Random rand, MultiplexNetwork& mnet, network_id net);
vertex_id choice_common_friends(Random rand, MultiplexNetwork& mnet, network_id net, vertex_id vertex);
vertex_id choice_degree(Random rand, MultiplexNetwork& mnet, network_id net);


/**********************************************************************/
/** Evolution models **************************************************/
/**********************************************************************/
class EvolutionModel {
public:
	virtual ~EvolutionModel() = 0;
	virtual void evolution_step(MultiplexNetwork& mnet, network_id net) = 0;
	virtual void evolution_step(MultiplexNetwork& mnet, network_id net, std::set<global_vertex_id>& new_vertexes, std::set<global_edge_id>& new_edges) = 0;
	virtual void init_step(MultiplexNetwork& mnet, network_id net) = 0;
protected:
	Random rand;
};

/**
 * @brief Grows a network by first creating a complete graph with m0 vertexes, then adding a new vertex at a time and connecting it to m other vertexes chosen with a probability proportional to their degree.
 **/
class BAEvolutionModel : public EvolutionModel {
	int m0, m;
public:
	BAEvolutionModel(int m0, int m);
	~BAEvolutionModel();
	void init_step(MultiplexNetwork& mnet, network_id net);
	void evolution_step(MultiplexNetwork& mnet, network_id net);
	void evolution_step(MultiplexNetwork& mnet, network_id net, std::set<global_vertex_id>& new_vertexes, std::set<global_edge_id>& new_edges);
};

/**
 * @brief Grows a network by first creating all the vertexes and then at every step choosing two (uniform probability) to connect with an edge.
 **/
class UniformEvolutionModel : public EvolutionModel {
	int m0;
public:
	UniformEvolutionModel(int m0);
	~UniformEvolutionModel();
	void evolution_step(MultiplexNetwork& mnet, network_id net);
	void evolution_step(MultiplexNetwork& mnet, network_id net, std::set<global_vertex_id>& new_vertexes, std::set<global_edge_id>& new_edges);
	void init_step(MultiplexNetwork& mnet, network_id net);
};

/**********************************************************************/
/** Evolution method **************************************************/
/**********************************************************************/
/**
 * @brief Grows the input multiplex network.
 * @param mnet MultiplexNetwork to grow
 * @param num_of_steps number of evolution steps
 * @param pr_no_event[] for each network, the probability that an evolution step does not change the network
 * @param pr_internal_event[] for each network, the probability that if something happens this is an internal evolution according to the evolution_model[] parameter
 * @param dependency[][] The (i,j) element of this matrix indicates the probability that, given an external evolution event, network i will consider network j as a potential candidate to import edges from
 * @param evolution_model[] for each network, a specification of how the network should evolve when an internal event happens
 **/
void evolve_edge_import(MultiplexNetwork &mnet,
		long num_of_steps,
		std::vector<double> pr_no_event,
		std::vector<double> pr_internal_event,
		std::vector<std::vector<double> > dependency,
		std::vector<EvolutionModel*> evolution_model);

void evolve_edge_copy(MultiplexNetwork &mnet,
		long num_of_steps,
		std::vector<double> pr_no_event,
		std::vector<double> pr_internal_event,
		std::vector<std::vector<double> > dependency,
		std::vector<EvolutionModel*> evolution_model);

void evolve(MultiplexNetwork &mnet,
		long num_of_steps,
		std::vector<int> num_new_vertexes_per_step,
		std::vector<double> pr_internal_event,
		std::vector<evolution_strategy> strategy,
		std::vector<double> pr_external_event,
		std::vector<std::vector<double> > dependency);

#endif /* EVOLUTION_H_ */
