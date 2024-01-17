# GDMPT

GDExtension project using libopenmpt.

Running the default scene will play `bananasplit.mod` from [furrykef/GDMPT](https://github.com/furrykef/GDMPT/blob/97159c16f26a8f0c4ce25b54d920191987415fb8/project/bananasplit.mod).

## Building

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
