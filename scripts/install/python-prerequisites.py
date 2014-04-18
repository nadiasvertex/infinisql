#!/usr/bin/env python3
import pip

cmd = ["install", "--upgrade"]
libs = ["pyzmq", "psutil", "lmdb", "tornado", "parsedatetime"]

pip.main(cmd + libs)

