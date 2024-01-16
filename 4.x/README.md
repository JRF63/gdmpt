## Building

Compilation is done purely through SCons.

```sh
git submodule update --init --recursive
cd 4.x
scons target=template_debug && scons target=template_release
```