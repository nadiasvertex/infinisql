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
	virtual Response on_add_node()=0;
	virtual Response on_remove_node()=0;
	virtual Response on_stop_node()=0;
	virtual Response on_set_node_type()=0;

	virtual Response on_partition()=0;
	virtual Response on_add_partition()=0;
	virtual Response on_delete_partition()=0;
	virtual Response on_new_partition()=0;
	virtual Response on_persist_partition()=0;
	virtual Response on_replicate_partition()=0;
	virtual Response on_stop_partition()=0;
	virtual Response on_sync_partition()=0;

	virtual Response on_add_partition_group()=0;

	virtual Response on_start()=0;
	virtual Response on_stop()=0;

	virtual Response on_transaction_log()=0;
	virtual Response on_stop_transaction_log()=0;
	virtual Response on_notify_when_log_committed()=0;
	virtual Response on_notify_when_log_sent()=0;

	virtual Response on_start_transactions()=0;
	virtual Response on_finish_and_abort_transactions()=0;
	virtual Response on_finish_transactions()=0;

	virtual Response on_set_sync()=0;
	virtual Response on_stop_sync()=0;

	virtual Response on_change_manager()=0;
	virtual Response on_change_replica()=0;

	virtual Response on_delete_version()=0;
	virtual Response on_get_config()=0;
	virtual Response on_node_problem()=0;
	virtual Response on_relayout()=0;
};


} // end namespace configuration
} // end namespace infinisql


#endif /* INFINISQL_CONFIGURATION_LISTENER_H_ */
