#!/bin/sh

glslangValidator -S vert -V100 --vn cube_vert -o cube.vert.h cube.vert.glsl
glslangValidator -S frag -V100 --vn cube_frag -o cube.frag.h cube.frag.glsl
