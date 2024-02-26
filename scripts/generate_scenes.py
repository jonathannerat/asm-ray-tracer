#!/usr/bin/env python3

import math
import os
import sys
from random import random as frand, choices as rand_choices, choice as rand_choice
from dataclasses import dataclass


RESOLUTIONS = {
    "small": [100, 100],
    "medium": [200, 200],
    "large": [400, 400],
}

MATERIAL_TYPES = ["lambertian", "metal", "dielectric", "light"]
MATERIAL_WEIGHTS = [0.5, 0.35, 0.1, 0.05]

OBJECT_TYPES = ["box", "sphere"]


def main():
    config = parse_config(sys.argv)

    if not os.path.isdir("scenes/gen"):
        os.mkdir("scenes/gen")

    for i in range(config.rows):
        scene_file = f"scenes/gen/scene{i}"

        with open(scene_file, "w") as f:
            f.write(
                f"output: width={config.width} height={config.height} spp=50 depth=20\n"
            )
            f.write("camera: from=3,3,3 to=0,1,0\n")

            num_objects = config.rows * config.cols

            # materials
            f.write("materials:\n")
            f.write("    light=1,1,1\n")
            f.write("    lambertian=.3,.1,.7\n")
            for i in range(num_objects):
                mtype = rand_choices(MATERIAL_TYPES, MATERIAL_WEIGHTS)[0]
                f.write("    " + mtype)
                [r, g, b] = [frand() for _ in range(3)]
                if mtype == "light" or mtype == "lambertian":
                    f.write(f"={r:.3f},{g:.3f},{b:.3f}\n")
                else:
                    alpha = frand()
                    f.write(f": albedo={r:.3f},{g:.3f},{b:.3f} ")
                    if mtype == "metal":
                        f.write(f" fuzz={alpha:.3f}\n")
                    else:
                        f.write(f" ir={alpha:.3f}\n")

            f.write("objects:\n")
            f.write("    plane: origin=0,6,0 normal=0,-1,0 material=0\n")
            f.write("    plane: material=1\n")
            mat_index = 0
            for y in range(math.floor(-config.rows / 2), math.floor(config.rows / 2)):
                for x in range(
                    math.floor(-config.cols / 2), math.floor(config.cols / 2)
                ):
                    otype = rand_choice(OBJECT_TYPES)
                    cx = x + 0.7 * frand()
                    cz = y + 0.7 * frand()

                    if otype == "box":
                        halfside = frand() * 0.5 + 0.1
                        pmin = f"{cx-halfside:.3f},0,{cz-halfside:.3f}"
                        pmax = f"{cx+halfside:.3f},{2*halfside:.3f},{cz+halfside:.3f}"
                        f.write(
                            f"    box: pmin={pmin} pmax={pmax} material={mat_index+2}\n"
                        )
                    else:
                        radius = frand() * 0.5 + 0.1
                        f.write(
                            f"    sphere: center={cx:.3f},{radius:.3f},{cz:.3f} radius={radius:.3f} material={mat_index+2}\n"
                        )

                    mat_index += 1


@dataclass
class Config:
    width: int = 100
    height: int = 100
    rows: int = 5
    cols: int = 5


def parse_config(args: list[str]):
    config = Config()
    i = 1

    while i < len(args):
        if args[i] == "-r":
            res = RESOLUTIONS[args[i + 1]]
            config.width = res[0]
            config.height = res[1]
        elif args[i] == "-w":
            config.width = int(args[i + 1])
        elif args[i] == "-h":
            config.height = int(args[i + 1])
        elif args[i] == "-x":
            config.cols = int(args[i + 1])
        elif args[i] == "-y":
            config.rows = int(args[i + 1])

        i += 2

    return config


if __name__ == "__main__":
    main()
