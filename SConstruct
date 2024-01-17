#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

opts = Variables([], ARGUMENTS)

opts.Add("target_name", "Name of the library to be built by SCons", "libgdmpt")
if env["platform"] == "windows":
    toolchains = ["vs2017winxp", "vs2017winxpansi", "vs2019win10", "vs2019win10uwp", "vs2019win7", "vs2019win81", "vs2022win10", "vs2022win10clang", "vs2022win10uwp", "vs2022win7", "vs2022win81"]
    opts.Add(EnumVariable("vs_toolchain", "Visual Studio version and the OS version used to build libopenmpt", "vs2022win10", toolchains))
opts.Update(env)

env.Append(CPPPATH=["src/", "openmpt"])

if env["platform"] == "windows":
    suffix = "Debug"
    if env["target"] == "template_release":
        suffix = "Release"
        # OpenMPT enables "whole program optimization" (/GL) on release builds
        env.Append(LINKFLAGS=["/LTCG"])
    else:
        # Override to prevent LNK2038
        env.Append(CCFLAGS=["/MTd"])
        # Override to prevent LNK4075
        env.Append(LINKFLAGS=["/OPT:NOREF"])

    env.Append(LIBPATH=["openmpt/build/lib/{}/{}/{}".format(env["vs_toolchain"], env["arch"], suffix)])
    env.Append(LIBS=["libopenmpt-small", "openmpt-minimp3", "openmpt-miniz", "openmpt-stb_vorbis"])
else:
    env.Append(LIBS=["openmpt"])

sources = Glob("src/*.cpp")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "project/bin/{}.{}.{}.framework/{}.{}.{}".format(
            env["target_name"], env["platform"], env["target"], env["target_name"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "project/bin/{}{}{}".format(env["target_name"], env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
