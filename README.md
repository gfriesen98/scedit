# scedit

simple painting tool for drawing on screenshots using raylib

i dont know how to write c so wip

## usage

```bash
Usage: scedit -i <input_file_path> [-o <output_file_path>] [-sch]
Options:
    -i      : Input file path
            You can also pipe an image into scedit
            Example: cat img.png | scedit -o output.png
    -o      : Output file path
    -s      : Enable save/clipboard on exit
    -c      : Enable clipboard on exit
    -h      : Prints this menu

GUI Usage:
    L Click     : Draw
    R Click     : Erase
    Backspace   : Clear all
    Ctrl+C      : Copy to clipboard
    Ctrl+S      : Save to output path and copy
```

### examples

Loading an image file with a designated output file path

`$ scedit -i path/to/input/file.png -o path/to/save/file.png`

Loading an image from a pipe with a designated output file path

`$ cat /tmp/img.png | scedit -o myimg.png`

see `screenshot.sh` for scripting usage examples

## build

depends on

- raylib
- xclip
- notify-send (for feedback on saving etc)

1. ensure dependencies are installed
2. clone this repository
3. `cd scedit`
4. `make`
    - to clean build files, run `make clear`

build file `scedit` will be generated in `./build`
