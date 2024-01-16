#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

target_name = "libgdmpt"

env.Append(CPPPATH=["src/", "libopenmpt/inc"])
env.Append(LIBPATH=["libopenmpt/lib/{}/{}".format(env["platform"], env["arch"])])
if env["platform"] == "windows":
    env.Append(LIBS=["libopenmpt"])
else:
    env.Append(LIBS=["openmpt"])

sources = Glob("src/*.cpp")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "project/bin/{}.{}.{}.framework/libgdexample.{}.{}".format(
            target_name, env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "project/bin/{}{}{}".format(target_name, env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
