#! /usr/bin/python3
import os
import shutil
import sys

from glob import glob
from urllib.parse import urlparse

distro_folder = os.path.abspath(os.path.join(os.path.split(__file__)[0], "../../"))
lib_folder = os.path.join(distro_folder, "lib")

def copy_lib(from_path):
   if not os.path.exists(lib_folder):
      os.mkdir(lib_folder)
   lib_name = os.path.split(from_path)[1]
   target = os.path.join(lib_folder, lib_name)
   shutil.copyfile(from_path, target)
   print("Installing library: %s -> %s" %(from_path, target))
   
def find_folder_with_file(start_path, find_filename):
   for root, dirs, files in os.walk(start_path):
      if find_filename in files:
         return root
      
   return None

def tbb(archive_path): 
   archive_folder, archive_name = os.path.split(archive_path)
   os.system('tar -C "%s" -xzf "%s"' % (archive_folder, archive_path))
   os.chdir(os.path.join(archive_folder, archive_name.replace("_src.tgz", "")))
   os.system('make tbb')
   tbb_lib_path = find_folder_with_file(os.getcwd(), "libtbb.so")
   copy_lib(os.path.join(tbb_lib_path, "libtbb.so"))
   copy_lib(os.path.join(tbb_lib_path, "libtbb.so.2"))
   
#===============================================================================
packages = [("https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb42_20131118oss_src.tgz",
             tbb)]

#===============================================================================
tmp_folder = os.environ.get("TEMP", "/tmp") 
for pkg in packages:
   url, builder = pkg
   parsed_url = urlparse(url)
   file_name = os.path.split(parsed_url.path)[1]
   archive_path = os.path.join(tmp_folder, file_name)
   fetch = 'wget -c -O "%s" "%s"' % (archive_path, url)
   os.system(fetch)
   curdir = os.getcwd()
   builder(archive_path)
   os.chdir(curdir)
