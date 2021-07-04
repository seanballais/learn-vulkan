#!/bin/sh

echo -n "Compiling vertex and fragment shaders... "
glslc shaders/vertex.vert -o shaders/vertex.spv
glslc shaders/fragment.frag -o shaders/fragment.spv
echo "Done!"