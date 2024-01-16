# GDMPT

GDNative/GDExtension project using libopenmpt.

Running the default scene will play `bananasplit.mod` from [furrykef/GDMPT](https://github.com/furrykef/GDMPT/blob/97159c16f26a8f0c4ce25b54d920191987415fb8/project/bananasplit.mod).

## Overview

The root folder of the repository contains `SCsub` - a Godot 3.x/4.x agnostic build script for libopenmpt.

The 3.x/4.x folders contains the build instructions for the GDNative/GDExtension. Compilation requires a C/C++ compiler and a Python interpreter with SCons. Currently tested only for Windows and Linux, both using the x64 arch.