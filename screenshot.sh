#!/bin/bash

# Example usage of 'scedit'
#
# Uses 'maim' to take a mouse selection screenshot and
# opens it with 'scedit'
#
# Save the screenshot in a date organized file structure
# On 'scedit' exit, copy to clipboard and save to designated location

path=$HOME/images/screenshots/$(date +%Y)/$(date +%B)
filename=$(date +"%Y-%m-%d-%H:%M").png

[[ ! -d $path ]] && mkdir -p $path

maim -u -s | ./build/scedit -o $path/$filename --clipboardonexit --brushsize 5 --colour fc03b1
