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

**note** ideal usage means this is a floating window. for window manager users you may want to set a rule to set scedit to be a floating window

i3wm example:

`for_window [class="scedit"] floating enable`

## build

### dependencies

- gcc
- raylib
- xclip (for clipboard access)
- notify-send (for feedback on saving etc)

### building

1. ensure dependencies are installed via your package manager
    - you'll have to figure out what the package names are for your package manager
2. clone this repo: `git clone https://github.com/gfriesen98/scedit.git`
3. naviage to repo: `cd scedit`
4. build by running: `make`
    - to clean build files, run `make clear`
5. copy build to /usr/local/bin: `cp ./build/scedit /usr/local/bin`
