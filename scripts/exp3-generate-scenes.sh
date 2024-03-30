#!/bin/sh

targetdir="./scenes/exp3_obj_types"

mkdir -p "$targetdir"

for obj in box sphere tetra; do
    for i in $(seq 1 10); do
        ./scripts/generate_scenes.py -s 1 -x "$i" -y "$i" -o "$obj"
        target="$(printf "%s/%s_%02dx%02d" "$targetdir" "$obj" "$i" "$i")"
        mv ./scenes/generated/scene0 "$target"
    done
done
