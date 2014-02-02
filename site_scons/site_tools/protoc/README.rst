Generating Protocol Buffers files with SCons
============================================

This SCons builder will allow you to generate Google Protocol Buffers .pb.cc, .pb.h, and _pb2.py files using
SCons from a .proto source file.

It doesn't currently support Java, but that should be easy to add.

Setup
=====

Clone this repository (with the name protoc) to the site_scons/site_tools directory of your project to use
the tool for just that project or clone to your ~/site_scons/site_tools directory (still named protoc) for
use with all your projects.

Usage
=====

Here is a sample SConstruct Scott used (some MSVC/Windows-specific stuff, but the Protoc builder is
OS-agnostic).  Note that you must make sure the protoc executable is available in your PATH or it won't be
able to execute it::

    import os

    env = Environment(
        ENV = {'PATH' : os.environ['PATH']},
        CPPPATH = ['C:\wc\protobuf-trunk\src' ],
        CPPFLAGS = ['/EHsc', '/MDd'],
        tools=['default', 'protoc']
        )

    proto_files = env.Protoc(
        [],
        "config.proto",
        PROTOCPROTOPATH=['.',r'C:\wc\protobuf-trunk\src',],
        PROTOCPYTHONOUTDIR='python', # set to None to not generate python
        PROTOCOUTDIR = 'build', # defaults to same directory as .proto
        # PROTOCCPPOUTFLAGS = "dllexport_decl=PROTOCONFIG_EXPORT:", too
    )

    env.Program('test', ['test.cpp', proto_files[0]],
        LIBPATH=[r'C:\wc\protobuf-trunk\vsprojects\Debug'],
        LIBS=['libprotobuf.lib'],
        )
