/*
 * configuration_listener.h
 *
 *  Created on: Feb 18, 2014
 *      Author: christopher
 */

#ifndef INFINISQL_CONFIGURATION_LISTENER_H_
#define INFINISQL_CONFIGURATION_LISTENER_H_

#include "configuration.pb.h"

namespace infinisql {
namespace configuration {

class listener {
public:
	virtual Response on_add_node(const Request& r)=0;
	virtual Response on_remove_node(const Request& r)=0;
	virtual Response on_stop_node(const Request& r)=0;
	virtual Response on_set_node_type(const Request& r)=0;

	virtual Response on_partition(const Request& r)=0;
	virtual Response on_add_partition(const Request& r)=0;
	virtual Response on_delete_partition(const Request& r)=0;
	virtual Response on_new_partition(const Request& r)=0;
	virtual Response on_persist_partition(const Request& r)=0;
	virtual Response on_replicate_partition(const Request& r)=0;
	virtual Response on_start_partition(const Request& r)=0;
	virtual Response on_stop_partition(const Request& r)=0;
	virtual Response on_sync_partition(const Request& r)=0;

	virtual Response on_add_partition_group(const Request& r)=0;

	virtual Response on_start(const Request& r)=0;
	virtual Response on_stop(const Request& r)=0;

	virtual Response on_transaction_log(const Request& r)=0;
	virtual Response on_start_transaction_log(const Request& r)=0;
	virtual Response on_stop_transaction_log(const Request& r)=0;
	virtual Response on_notify_when_log_committed(const Request& r)=0;
	virtual Response on_notify_when_log_sent(const Request& r)=0;

	virtual Response on_start_transactions(const Request& r)=0;
	virtual Response on_finish_and_abort_transactions(const Request& r)=0;
	virtual Response on_finish_transactions(const Request& r)=0;

	virtual Response on_set_sync(const Request& r)=0;
	virtual Response on_stop_sync(const Request& r)=0;

	virtual Response on_change_manager(const Request& r)=0;
	virtual Response on_change_replica(const Request& r)=0;

	virtual Response on_delete_version(const Request& r)=0;
	virtual Response on_get_config(const Request& r)=0;
	virtual Response on_node_problem(const Request& r)=0;
	virtual Response on_relayout(const Request& r)=0;
};


} // end namespace configuration
} // end namespace infinisql


#endif /* INFINISQL_CONFIGURATION_LISTENER_H_ */
