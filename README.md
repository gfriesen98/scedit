# scedit

simple painting tool for drawing on screenshots using raylib

i dont know how to write c so wip

## usage

`$ scedit -i path/to/input/file -o path/to/save/file`

**note** this doesnt actually work like this yet

see `screenshot.sh` for scripting usage examples

## build

depends on

- raylib
- xclip

1. ensure dependencies are installed
2. clone this repository
3. `cd scedit`
4. `make`
    - to clean build files, run `make clear`

build file `scedit` will be generated in `./build`
