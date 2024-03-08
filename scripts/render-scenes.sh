#!/bin/sh

SCENES_DIR="${1:-scenes}"
OUT_DIR="${2:-images}"

[ -d "$SCENES_DIR" ] || ( >&2 echo "invalid scenes dir" && exit 1 )
[ -d "$OUT_DIR" ] || mkdir -p "$OUT_DIR"

for f in $(find "$SCENES_DIR" -type f | sort); do
    scene="${f##*/}"
    out="$OUT_DIR/$scene"
    echo "Rendering $scene with C tracer into $out-c.png"
    ./ray_tracer -c -s "$f" | convert ppm:- "$out-c.png"
    echo "Rendering $scene with ASM tracer into $out-asm.png"
    ./ray_tracer -a -s "$f" | convert ppm:- "$out-asm.png"
done
