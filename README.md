# GDMPT

GDExtension project using libopenmpt.

Running the default scene will play `bananasplit.mod` from [furrykef/GDMPT](https://github.com/furrykef/GDMPT/blob/97159c16f26a8f0c4ce25b54d920191987415fb8/project/bananasplit.mod).

## Building

Make sure to update the submodules:

```sh
git submodule upate --init --recursive
```

Required tools
- [SCons](https://scons.org/doc/production/HTML/scons-user/ch01s02.html)
- [MSBuild](https://visualstudio.microsoft.com/downloads/?q=build+tools). Either Visual Studio or just the Build Tools. Only for Windows builds.

### Windows

First build libopenmpt-small.sln.

```sh
# Debug
msbuild libopenmpt-small.sln -t:rebuild -p:Platform=x64,Configuration=Debug

# Release
msbuild libopenmpt-small.sln -t:rebuild -p:Platform=x64,Configuration=Release
```

Then run scons. Replace `vs2022win10` on the following commands with the version of Visual Studio and Windows you used to build libopenmpt.

```sh
# Debug
scons platform=windows vs_toolchain=vs2022win10

# Release
scons platform=windows vs_toolchain=vs2022win10 target=template_release
```

### Linux

libopenmpt is available in most [Linux distros](https://wiki.openmpt.org/Libopenmpt#Distribution_packages).

In Ubuntu for example:

```sh
sudo apt install libopenmpt-dev
```

Then run scons.

```sh
# Debug
scons platform=linux

# Release
scons platform=linux target=template_release
```
