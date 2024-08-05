# scedit

simple painting tool for drawing on screenshots using raylib

i dont know how to write c so wip

## usage

```bash
Usage: ./scedit -i <input_file_path> -o <output_file_path> [-bclmsxh]

Options:
    [stdin] -i --input       : Input file path
                        You can also pipe image into scedit
                        Example: 'cat img.png | scedit -o myimg.png'
    -o --output              : Output file path (required)
    -b --brushsize           : Brush size (default '5')
    -c --colour              : Brush colour or hex colour code. Default 'red'
                        Example: 'scedit -c white'
                                 'scedit -c fcba03'
    -l --listcolours         : List all builtin colours
    -m --monitor             : Display to open on (default 0)
    -s --saveonexit          : Enable save+copy on exit (Default disabled)
    -x --clipboardonexit     : Enable copy on exit (Default disabled)
    -h --help                : Prints this help message

GUI Usage:
    L Click         : Draw
    R Click         : Erase area
    Scroll up/down  : Change brush size
    Backspace       : Clear all
    Ctrl+C          : Copy to clipboard
    Ctrl+S          : Save to output path and copy
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
