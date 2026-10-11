#!/bin/sh
# Generates version.cpp from version.cpp.in, baking in the current
# timestamp. Used by TiledPatternMaker.pro (qmake/Qt Creator build) on
# Linux/macOS -- mirrors what gen_version.cmake does for the CMake build.
#
# usage: gen_version.sh <version.cpp.in> <version.cpp>
IN="$1"
OUT="$2"
TS=$(date '+%Y-%m-%d %H:%M:%S')
sed "s/@BUILD_TIMESTAMP@/$TS/" "$IN" > "$OUT"
