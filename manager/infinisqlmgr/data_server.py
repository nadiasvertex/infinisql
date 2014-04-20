__author__ = 'Christopher Nelson'

import logging
import os
import signal
import sys
import urllib.request
import urllib.parse

from infinisqlmgr import common, engine
from infinisqlmgr.management import msg

def start_data_server(config):
   logging.basicConfig(level=logging.DEBUG if config.args.debug else logging.INFO,
                       format=common.DEFAULT_LOG_FORMAT)
   node_id = config.args.node_id
   node_ip, node_port = node_id.split(":")
   mt = config.config["management"]
   management_ip = node_ip
   configuration_port = mt["configuration_port"]

   if node_port in (configuration_port, mt["management_port"]):
      logging.error("Unable to start a database engine on port %s because it "
                    "conflicts with the management server.")
      return

   logging.info("Trying to start new database engine at %s", node_id)

   url = urllib.parse.urlunparse(('http', "%s:%s" % (node_ip, configuration_port), "/dbe/start/%s/%s/" % (node_ip, node_port), None, None, None))
   logging.debug("request=%s", url)
   with urllib.request.urlopen(url) as f:
      pass


def stop_data_server(config):
   logging.basicConfig(level=logging.DEBUG if config.args.debug else logging.INFO,
                      format=common.DEFAULT_LOG_FORMAT)
   node_id = config.args.node_id
   node_ip, node_port = node_id.split(":")
   mt = config.config["management"]
   management_ip = node_ip
   configuration_port = mt["configuration_port"]

   logging.info("Trying to stop existing database engine at %s", node_id)

   url = urllib.parse.urlunparse(('http', "%s:%s" % (node_ip, configuration_port), "/dbe/stop/%s/%s/" % (node_ip, node_port), None, None, None))
   logging.debug("request=%s", url)
   with urllib.request.urlopen(url) as f:
      pass


def show_data_server(args):
    logging.basicConfig(level=logging.DEBUG if config.args.debug else logging.INFO,
                       format=common.DEFAULT_LOG_FORMAT)

def list_data_servers(args):
    logging.basicConfig(level=logging.DEBUG if config.args.debug else logging.INFO,
                       format=common.DEFAULT_LOG_FORMAT)

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

