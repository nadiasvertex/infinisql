/*
 * nodeid.h
 *
 *  Created on: Feb 2, 2014
 *      Author: Christopher Nelson
 */

#ifndef INFINISQLNODEID_H_
#define INFINISQLNODEID_H_

namespace infinisql {
namespace configuration {

/** Defines a strong type for the node id. Keeps us from accidentally accepting
 * values that aren't really node ids. Also lets us change the underlying type
 * of a node id if we need to do that in the future.
 */
class nodeid {
	int node_id;

	nodeid(const int node_id):node_id(node_id) {};
public:
	nodeid(const nodeid &id)=default;
	int to_int() const { return node_id; }
	static nodeid from_int(const int node_id) {
		return nodeid(node_id);
	}
};

} // end namespace configuration
} // end namespace infinisql

#endif /* INFINISQLNODEID_H_ */
