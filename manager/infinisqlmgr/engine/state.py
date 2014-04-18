__author__ = 'Christopher Nelson'

import logging

from io import BytesIO

class ConfigurationState(object):
    def __init__(self, controller):
        self.controller = controller
        self.actors = {}
        self.replica_members = {}

