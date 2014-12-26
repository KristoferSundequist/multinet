/*
 * evolution.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: magnanim
 */

#include <stdio.h>
#include <vector>
#include <iostream>
#include "evolution.h"

void evolve(MultiplexNetwork &mnet,
		long num_of_steps,
		std::vector<int> num_new_vertexes_per_step,
		std::vector<double> pr_internal_event,
		std::vector<evolution_strategy> strategy,
		std::vector<double> pr_external_event,
		std::vector<std::vector<double> > dependency) {

	Random rand;

	// init of support data structures
	std::vector<std::set<entity_id> > free_identities(mnet.getNumNetworks());
	std::set<entity_id> ids = mnet.getGlobalIdentities();

	//std::cout << "Init identities" << std::endl;
	for (int n=0; n<mnet.getNumNetworks(); n++) {
		std::set<entity_id> to_copy(ids.begin(),ids.end());
		free_identities[n] = to_copy;
	}

	for (long i=0; i<num_of_steps; i++) {
		for (int n=0; n<mnet.getNumNetworks(); n++) {

			// Add new vertexes
			//std::cout << "Add vertexes to " << n << ": ";
			for (int new_v=0; new_v<num_new_vertexes_per_step[n]; new_v++) {
				if (free_identities[n].empty())
					break;
				entity_id gid = rand.getElement(free_identities[n]);
				free_identities[n].erase(gid);
				vertex_id vertex = mnet.getNetwork(n).addVertex();
				mnet.mapIdentity(gid,vertex,n);
				//std::cout << ".";
			}
			//std::cout << std::endl;

			std::set<vertex_id> vertexes = mnet.getNetwork(n).getVertexes();
			for (vertex_id vertex: vertexes) {
				if (rand.test(pr_internal_event[n])) {
					//std::cout << "Iternal event for vertex " << vertex << std::endl;
					switch (strategy[n]) {
						case EVOLUTION_DEGREE:
							//std::cout << "Getting target vertex " << std::endl;
							vertex_id target = choice_degree(rand, mnet, n);
							if (target==-1) break;
							//std::cout << "Inserting edge to " << target << std::endl;
							if (!mnet.getNetwork(n).containsEdge(vertex,target))
								mnet.getNetwork(n).addEdge(vertex,target);
							break;
					}
				}
				if (rand.test(pr_external_event[n])) {
					//std::cout << "External event for vertex " << vertex << std::endl;
								// choose a network from which to import an edge: first find the candidates:
								std::set<network_id> candidates;
								for (int i=0; i<dependency[n].size(); i++) {
									if (rand.test(dependency[n][i]))
										candidates.insert(i);
								}
								// then pick uniformly at random one of the candidates
								network_id import = rand.getElement(candidates);
								//std::cout << "external event from " << import << std::endl;
								// finally we choose an edge uniformly at random from this network and we insert it into the target
								std::set<edge_id> edges = mnet.getNetwork(import).getEdges();
								std::set<edge_id> edges_between_common_ids;
								for (edge_id e: edges) {
									entity_id gid1 = mnet.getGlobalIdentity(e.v1,import);
									entity_id gid2 = mnet.getGlobalIdentity(e.v2,import);
									if (!mnet.containsVertex(gid1,n) || !mnet.containsVertex(gid2,n))
										continue;
									vertex_id lid1 = mnet.getVertexId(gid1,n);
									vertex_id lid2 = mnet.getVertexId(gid2,n);
									if (!mnet.getNetwork(n).containsEdge(lid1,lid2)) {
										edge_id eid(lid1,lid2,false);
										edges_between_common_ids.insert(eid);
									}
								}
								if (edges_between_common_ids.empty()) continue;
								edge_id edge_to_be_imported = rand.getElement(edges_between_common_ids);
								// check if these identities are already present in the target network (if not, insert them)
								mnet.getNetwork(n).addEdge(edge_to_be_imported.v1, edge_to_be_imported.v2);
							}
						}
			}

	}
}

void evolve_edge_import(MultiplexNetwork &mnet,
		long num_of_steps,
		std::vector<double> pr_no_event,
		std::vector<double> pr_internal_event,
		std::vector<std::vector<double> > dependency,
		std::vector<EvolutionModel*> evolution_model) {

		Random rand;

		// check size
		for (int i=0; i<mnet.getNumNetworks(); i++) {
			evolution_model[i]->init_step(mnet, i);
		}

		for (long i=0; i<num_of_steps; i++) {
			//if (i%100==0) std::cout << "Step " << i << std::endl;
			for (int n=0; n<mnet.getNumNetworks(); n++) {
				//std::cout << "Network " << n << ": ";
				if (rand.test(pr_no_event[n])) {
					//std::cout << "no event" << std::endl;
					continue;
				}
				if (rand.test(pr_internal_event[n])) {
					//std::cout << "internal event" << std::endl;
					evolution_model[n]->evolution_step(mnet, n);
					continue;
				}
				else {
					// choose a network from which to import an edge: first find the candidates:
					std::set<network_id> candidates;
					for (int i=0; i<dependency[n].size(); i++) {
						if (rand.test(dependency[n][i]))
							candidates.insert(i);
					}
					// then pick uniformly at random one of the candidates
					network_id import = rand.getElement(candidates);
					//std::cout << "external event from " << import << std::endl;
					// finally we choose an edge uniformly at random from this network and we insert it into the target
					std::set<edge_id> edges = mnet.getNetwork(import).getEdges();
					edge_id edge_to_be_imported = rand.getElement(edges);
					entity_id id1 = mnet.getGlobalIdentity(edge_to_be_imported.v1, import);
					entity_id id2 = mnet.getGlobalIdentity(edge_to_be_imported.v2, import);
					// check if these identities are already present in the target network (if not, insert them)
					vertex_id vertex1, vertex2;
					if (!mnet.containsVertex(id1,n)) {
						if (mnet.getNetwork(n).isNamed())
							vertex1 = mnet.getNetwork(n).addVertex(mnet.getGlobalName(id1));
						else vertex1 = mnet.getNetwork(n).addVertex();
						mnet.mapIdentity(id1, vertex1, n);
					}
					else vertex1 = mnet.getVertexId(id1,n);
					if (!mnet.containsVertex(id2,n)) {
						if (mnet.getNetwork(n).isNamed())
							vertex2 = mnet.getNetwork(n).addVertex(mnet.getGlobalName(id2));
						else vertex2 = mnet.getNetwork(n).addVertex();
						mnet.mapIdentity(id2, vertex2, n);
					}
					else vertex2 = mnet.getVertexId(id2,n);
					if (!mnet.getNetwork(n).containsEdge(vertex1, vertex2))
						mnet.getNetwork(n).addEdge(vertex1, vertex2);
				}
			}
		}
}


void evolve_edge_copy(MultiplexNetwork &mnet,
		long num_of_steps,
		std::vector<double> pr_no_event,
		std::vector<double> pr_internal_event,
		std::vector<std::vector<double> > dependency,
		std::vector<EvolutionModel*> evolution_model) {

		Random rand;

		// check size
		for (int i=0; i<mnet.getNumNetworks(); i++) {
			evolution_model[i]->init_step(mnet, i);
		}

		for (long i=0; i<num_of_steps; i++) {
			//if (i%100==0) std::cout << "Step " << i << std::endl;
			for (int n=0; n<mnet.getNumNetworks(); n++) {
				//std::cout << "Network " << n << ": ";
				if (rand.test(pr_no_event[n])) {
					//std::cout << "no event" << std::endl;
					continue;
				}
				if (rand.test(pr_internal_event[n])) {
					//std::cout << "internal event " << n << std::endl;
					std::set<global_vertex_id> new_vertexes;
					std::set<global_edge_id> new_edges;
					evolution_model[n]->evolution_step(mnet, n, new_vertexes, new_edges);

					// The newly inserted vertexes and edges can be copied to other networks
					for (int i=0; i<dependency.size(); i++) {
						if (rand.test(dependency[i][n])) {
							//std::cout << dependency[i][n] << " copy to " << i << std::endl;
							/* copy vertexes
							for (global_vertex_id gvid: new_vertexes) {
								global_identity gid = mnet.getGlobalIdentity(gvid.vid,gvid.network);
							if (!mnet.containsVertex(gid,i)) {
								vertex_id vertex;
								if (mnet.getNetwork(i).isNamed())
										vertex = mnet.getNetwork(i).addVertex(mnet.getGlobalName(gid));
								else
									vertex = mnet.getNetwork(i).addVertex();
								mnet.mapIdentity(gid, vertex, i);
							}
							}*/
							// copy edges
							for (global_edge_id geid: new_edges) {
								entity_id gid1 = mnet.getGlobalIdentity(geid.v1,geid.network);
								entity_id gid2 = mnet.getGlobalIdentity(geid.v2,geid.network);
								vertex_id lid1, lid2;
								if (!mnet.containsVertex(gid1,i)) { // not necessary
									if (mnet.getNetwork(i).isNamed())
										lid1 = mnet.getNetwork(i).addVertex(mnet.getGlobalName(gid1));
									else
										lid1 = mnet.getNetwork(i).addVertex();
									mnet.mapIdentity(gid1, lid1, i);
								}
								else lid1 = mnet.getVertexId(gid1,i);
								if (!mnet.containsVertex(gid2,i)) {
									if (mnet.getNetwork(i).isNamed())
										lid2 = mnet.getNetwork(i).addVertex(mnet.getGlobalName(gid2));
									else
										lid2 = mnet.getNetwork(i).addVertex();
									mnet.mapIdentity(gid2, lid2, i);
								}
								else lid2 = mnet.getVertexId(gid2,i);
								if (!mnet.getNetwork(i).containsEdge(lid1,lid2)) {
									mnet.getNetwork(i).addEdge(lid1,lid2);
									//std::cout << "E " << lid1 << " " << lid2 << std::endl;
								}
							}
						}
					}
				}
			}
		}
}


