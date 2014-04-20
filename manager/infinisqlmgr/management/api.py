from os import uname

__author__ = 'Christopher Nelson'

import json
import socket
import time

import parsedatetime
import tornado.web

import infinisqlmgr


class ManagementNodeListHandler(tornado.web.RequestHandler):
    def get(self):
        instance = infinisqlmgr.management.Controller.instance

        self.write(json.dumps({
            "cluster_name"         : instance.cluster_name,
            "peak_cluster_size"    : instance.peak_cluster_size,
            "current_cluster_size" : instance.current_cluster_size,
            "nodes"  : ["%s:%s" % (n[0], n[1]) for n in instance.get_nodes()],
            "leader" : instance.leader_node_id
        }))


class ManagementNodeDetailsHandler(tornado.web.RequestHandler):
    def get(self, node_name):
        instance = infinisqlmgr.management.Controller.instance
        parts = node_name.split(":")
        node_id = (parts[0], int(parts[1]))
        if not instance.is_node_known(node_id):
            self.write(json.dumps({
                "cluster_name"         : instance.cluster_name,
                "node"                 : node_name,
                "exists"               : False
            }))
            return


        self.write(json.dumps({
            "cluster_name"         : instance.cluster_name,
            "node"                 : node_name,
            "exists"               : True,
            "leader"               : instance.leader_node_id == node_id,
            "local"                : instance.node_id == node_id,
        }))


class MetricsHandler(tornado.web.RequestHandler):
    def process_metric(self, operation, values):
        values = [v for v in values if v is not None]
        if operation == "min":
            return min(values)
        elif operation == "max":
            return max(values)
        if operation == "avg":
            return sum(values) / len(values)

    def get(self, metric_name, operation):
        instance = infinisqlmgr.management.Controller.instance
        health = instance.get_health()
        metric = health.lookup(metric_name)
        if metric is None:
            self.write(json.dumps({
                "metric" : metric_name,
                "values" : []
             }))

        c = parsedatetime.Calendar()
        from_time = time.mktime(c.parse(self.get_argument("from", "24 hours ago"))[0])
        until_time = time.mktime(c.parse(self.get_argument("until", "now"))[0])

        result = metric.fetch(from_time, until_time)

        self.write(json.dumps({
            "metric"     : metric_name,
            "from"       : result[0][0],
            "until"      : result[0][1],
            "resolution" : result[0][2],
            "values"     : result[1] if operation == "values" else self.process_metric(operation, result[1])
         }))


class MetricsListHandler(tornado.web.RequestHandler):
    def get(self):
        instance = infinisqlmgr.management.Controller.instance
        health = instance.get_health()
        self.write(json.dumps(health.get_metric_names()))


class DatabaseEngineHandler(tornado.web.RequestHandler):
    def get(self, operation, node_ip, node_port):
        instance = infinisqlmgr.management.Controller.instance
        dbe_node_id = (socket.gethostbyname(node_ip), node_port)
        if operation == "start":
           instance.start_engine(dbe_node_id)
        elif operation == "stop":
           if not instance.stop_engine(dbe_node_id):
              self.set_status(400, "Unknown database engine.")
              self.write(json.dumps({"error" : "Database engine %s is not running." % dbe_node_id}))
        else:
           self.set_status(400, "Unknown operation.")
           self.write(json.dumps({"error" : "Operation %s is not supported" % operation}))


handlers = [
    (r"/nodes/management/([-a-zA-Z0-9.]+:\d+)/", ManagementNodeDetailsHandler),
    (r"/nodes/management/", ManagementNodeListHandler),
    (r"/metrics/([a-zaA-Z0-9.]+)/(avg|min|max|values)/", MetricsHandler),
    (r"/metrics/", MetricsListHandler),
    (r"/dbe/(start|stop)/([a-zaA-Z0-9.]+)/(\d+)/", DatabaseEngineHandler)
]
