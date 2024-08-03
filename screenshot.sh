#!/bin/bash

# Example usage of 'scedit'
# Uses 'maim' to take a screenshot with mouse
# selection then opens it in 'scedit'
#
# TODO: 
# - eventually flesh this out when scedit has better scripting support
# 	scedit uses /tmp/screen.png hardcoded

temppath="/tmp/screen.png"
path="$HOME/images/screenshots/$(date +%Y)/$(date +%B)"
filename=$(date +"%Y-%m-%d-%H:%M").png

maim -u -s $temppath || exit 1
./build/sceditor
