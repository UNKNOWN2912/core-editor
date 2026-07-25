#!/bin/bash

argumentCount="$#"

if ((argumentCount == 0)); then
    echo "usage: ./compile_shader <shader_name>";
    exit
fi

vertexFile="$1.vert"
fragmentFile="$1.frag"
geometryFile="$1.geom"
computeFile="$1.comp"

if [[ -f "$vertexFile" ]]; then
    glslc "$1.vert" -o "$1.vert.spv"
fi
if [[ -f "$fragmentFile" ]]; then
    glslc "$1.frag" -o "$1.frag.spv"
fi
if [[ -f "$geometryFile" ]]; then
    glslc "$1.geom" -o "$1.geom.spv"
fi
if [[ -f "$computeFile" ]]; then
    glslc "$1.comp" -o "$1.comp.spv"
fi