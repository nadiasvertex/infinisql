__author__ = 'Christopher Nelson'

import logging
import os
import signal

import psutil
import zmq

from infinisqlmgr.engine import state

class Configuration(object):
    def __init__(self, node_id, config):
        self.node_id = node_id
        self.dist_dir = config.dist_dir
        self.config = config
        self.infinisql = config.get("data engine", "infinisqld")
        self.log_file = config.get("data engine", "log_file")

        self.configuration_ip = config.get("data engine", "configuration_ip")
        self.configuration_port = config.get("data engine", "configuration_port")
        self.pid = None

        self.ctx = zmq.Context.instance()
        self.socket = None
        self.poller = zmq.Poller()

        self.state = state.ConfigurationState(self)
        self.receive_handler = None

    def _connect(self):
        """
        Connects to the database engine's management port.
        :return: None
        """
        if self.configuration_ip == "*":
            addresses = [interface[0].ip.compressed
                         for interface in self.config.interfaces().values()]
            ip = addresses[0]
        else:
           ip = self.configuration_ip

        logging.debug("connecting to database engine management port (%s:%s)", self.configuration_ip, self.configuration_port)
        self.socket = self.ctx.socket(zmq.REQ)

        self.socket.connect("tcp://%s:%s" % (ip, self.configuration_port))
        self.poller.register(self.socket, zmq.POLLIN)

    def set_next_handler(self, handler):
        self.receive_handler = handler

    def process(self):
        """
        Processes engine events.
        :return: None
        """
        events = self.poller.poll(timeout=100)
        for sock, event in events:
            if self.receive_handler is None:
                continue
            handler = self.receive_handler
            self.receive_handler = None
            handler(sock)

    def start(self):
        """
        Starts a database engine.
        :return: None
        """
        if self.pid is not None:
            logging.debug("database engine already started")
            return

        log_dir = os.path.dirname(self.log_file)
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)

        # -m <management ip:port> -n <nodeid> -l <log path/file>
        self.pid = os.spawnl(os.P_NOWAIT, self.infinisql, self.infinisql,
                              '-m', '%s:%s' % (self.configuration_ip, self.configuration_port),
                              '-n', str(self.node_id),
                              '-l', self.log_file)

        logging.info("Started database engine pid=%s", self.pid)
        self._connect()

    def stop(self):
        """
        Stops a database engine.
        :return: None
        """
        if self.pid is None:
            logging.debug("database engine already stopped")
            return

        logging.debug("disconnecting from database engine management command port")
        self.poller.unregister(self.socket)
        self.socket.close()
        self.socket = None

        logging.info("Stopping database engine pid=%s node=%s", self.pid, str(self.node_id))
        process = psutil.Process(self.pid)
        process.terminate()

        gone, alive = psutil.wait_procs([process], timeout=5)
        for p in alive:
          logging.warn("Unable to gently terminate database engine process, resorting to more severe measures.")
          p.kill()

        logging.info("Stopped database engine.")
        self.pid = None


