__author__ = 'Christopher Nelson'

import logging
import msgpack
import os
import signal
import sys
import zmq

from infinisqlmgr import common, engine
from infinisqlmgr.management import msg

def start_data_server(config):
   logging.basicConfig(level=logging.DEBUG if config.args.debug else logging.INFO,
                       format=common.DEFAULT_LOG_FORMAT)
   node_id = config.args.node_id
   node_ip, node_port = node_id.split(":")
   cluster_name = config.get("management", "cluster_name")
   mt = config.config["management"]
   management_ip = node_ip
   management_port = mt["management_port"]

   addresses = [(node_ip, int(management_port))]

   if management_port == node_port:
      logging.error("Unable to start a database engine on port %s because it "
                    "conflicts with the management server.")
      return

   logging.info("Trying to start new database engine at %s", node_id)

   ctx = zmq.Context()
   socket = ctx.socket(zmq.REQ)
   poller = zmq.Poller()
   poller.register(socket, zmq.POLLOUT)
   for address in addresses:
      logging.debug("connecting to management server (%s:%s)", address[0], address[1])
      socket.connect("tcp://%s:%s" % (address[0], address[1]))
      logging.debug("polling for connection")
      events = poller.poll(1000)
      if not events:
         logging.warn("unable to connect to management server on (%s:%s)", address[0], address[1])
      logging.debug("sending start command")
      socket.send(msgpack.packb((msg.START_DATA_ENGINE, node_id)))
      logging.debug("closing socket")
      poller.unregister(socket)
      socket.close()
      logging.debug("completed.")
      sys.exit(0)
      return
   else:
      logging.error("Unable to connect to the management server on any available address.")

def stop_data_server(args):
    common.configure_logging(config)

def show_data_server(args):
    common.configure_logging(config)

def list_data_servers(args):
    common.configure_logging(config)

def add_args(sub_parsers):
    mgr_parser = sub_parsers.add_parser('dbe', help='Options for controlling database engine processes')
    mgr_parser.add_argument('--node', dest='node_id',
                              default=None, required=True,
                              help='The node to control. This in the form "ip:port".')

    ss_parsers = mgr_parser.add_subparsers()
    start_parser = ss_parsers.add_parser('start', help='Start a data engine process')
    start_parser.add_argument('--config-file', dest='config_file',
                              default=None,
                              help='Sets the configuration file to use to configure the database engine.'
                                   '(default is %(default)s)')
    start_parser.set_defaults(func=start_data_server)

    stop_parser = ss_parsers.add_parser('stop', help='Stop a data engine process')
    stop_parser.set_defaults(func=stop_data_server)

    list_parser = ss_parsers.add_parser('list', help='List active data engine processes')
    list_parser.set_defaults(func=list_data_servers)

    show_parser = ss_parsers.add_parser('show', help='Show information about a data engine process')
    show_parser.set_defaults(func=show_data_server)

