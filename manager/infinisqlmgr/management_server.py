__author__ = 'Christopher Nelson'

import datetime
import logging
import os
import psutil
import signal
import time

from infinisqlmgr import common, management

def start_management_server(config):
    from infinisqlmgr.management import util

    logging.basicConfig(level=logging.DEBUG if config.args.debug else logging.INFO,
                       format=common.DEFAULT_LOG_FORMAT)

    cluster_name = config.get("management", "cluster_name")
    existing_pid = util.get_pid(config.dist_dir, cluster_name)
    if existing_pid is not None:
        logging.error("A management process appears to exist already. You should run the 'manager stop' command first "
                      "to make sure the existing process has stopped.")
        return 1

    logging.debug("forking management server")
    pid = os.fork()

    if pid != 0:
        util.write_pid(config.dist_dir, cluster_name, pid)
        logging.info("Parent start_management_server() finished")
        return 0

    common.configure_logging(config)
    logging.debug("creating management process")
    management_server = management.Controller(config)
    logging.debug("starting management process")
    return management_server.run()


def stop_management_server(config):
    from infinisqlmgr.management import util

    logging.basicConfig(level=logging.DEBUG if config.args.debug else logging.INFO,
                       format=common.DEFAULT_LOG_FORMAT)

    cluster_name = config.get("management", "cluster_name")

    existing_pid = util.get_pid(config.dist_dir, cluster_name)
    if existing_pid is not None:
       try:
          process = psutil.Process(existing_pid)
       except psutil.NoSuchProcess:
          logging.info("Management process is already stopped.")
       else:
          created_at = datetime.datetime.fromtimestamp(process.create_time()).strftime("%Y-%m-%d %H:%M:%S")
          logging.info("Trying to stop the existing process at pid %d started at %s", existing_pid, created_at)
          process.terminate()

          gone, alive = psutil.wait_procs([process], timeout=5)
          for p in alive:
             logging.warn("Unable to gently terminate management process, resorting to more severe measures.")
             p.kill()

    # Make sure that the pid file is gone, even if it's empty.
    if util.exists(config.dist_dir, cluster_name):
        run_path = util.get_run_path(config.dist_dir, cluster_name)
        logging.debug("deleting run file at: %s", run_path)
        os.unlink(run_path)

    logging.info("Stopped management process for cluster: %s" % cluster_name)


def restart_management_server(config):
    stop_management_server(config)
    time.sleep(1)
    start_management_server(config)

def add_args(sub_parsers):
    mgr_parser = sub_parsers.add_parser('manager', help='Options for controlling a management process')
    mgr_parser.add_argument('--no-background', dest='daemonize', action='store_false',
                              default=True,
                              help='Do not run the manager in the background. Useful for debugging. (default is off)')
    mgr_parser.add_argument('--cluster-name', dest='cluster_name',
                              default="default_cluster",
                              help='Set the cluster name to join. If the cluster does not exist it will be created. '
                                   '(default is %(default)s)')

    ss_parsers = mgr_parser.add_subparsers()
    start_parser = ss_parsers.add_parser('start', help='Start a management process')
    mgr_parser.add_argument('--listen-interface', dest='management_interface',
                              default="*",
                              help='Set the interface to listen on.'
                                   '(default is %(default)s)')
    start_parser.set_defaults(func=start_management_server)

    stop_parser = ss_parsers.add_parser('stop', help='Stop a management process')
    stop_parser.set_defaults(func=stop_management_server)

    restart_parser = ss_parsers.add_parser('restart', help='Restart a management process')
    restart_parser.set_defaults(func=restart_management_server)

