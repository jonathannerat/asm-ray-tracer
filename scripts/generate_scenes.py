#!/usr/bin/env python3

import os
import sys
from random import random as frand
from lib.generation import random_material, generate_surface
from lib.config import Config, parse_config
from lib.ray_tracer import Vec, Camera, Output, Material, Surface, GeneratedScene


SCENES_DIR = os.path.join("scenes", "generated")


def generate_materials(config: Config):
    n = config.rows * config.cols
    res = []

    for _ in range(n):
        res.append(random_material())

    return res


def generate_objects(config: Config):
    res = []
    mat_index = 0
    zcenter = config.rows / 2
    xcenter = config.cols / 2

    for z in range(0, config.rows):
        for x in range(0, config.cols):
            center = Vec(x - xcenter + 0.5, frand() * 0.3, z - zcenter + 0.5)
            surfaces = generate_surface(config.objtype, center, mat_index, 0.7)


            if isinstance(surfaces, Surface):
                res.append(surfaces)
            else:
                res = res + surfaces
            mat_index += 1

    return res


def main():
    config = parse_config(sys.argv)

    if not os.path.isdir(SCENES_DIR):
        os.mkdir(SCENES_DIR)

    for i in range(config.scenes):
        scene_file = os.path.join(SCENES_DIR, "scene" + str(i))

        materials = generate_materials(config)
        materials.append(Material("light", Vec(1, 1, 1)))
        materials.append(Material("lambertian", Vec(0.4, 0.4, 0.4)))

        objects = generate_objects(config)
        objects.append(
            Surface(
                "plane",
                {
                    "origin": Vec(0, 4, 0),
                    "normal": Vec(0, -1, 0),
                    "material": len(materials) - 2,
                },
            )
        )
        objects.append(Surface("plane", {"material": len(materials) - 1}))
        scene = GeneratedScene(
            Output(config.width, config.height, 50, 25),
            Camera(
                Vec(-config.cols / 2 - 0.5, 2, -config.rows / 2 - 0.5), Vec(0, 0, 0)
            ),
            materials,
            objects,
        )

        with open(scene_file, "w") as f:
            f.write(str(scene))


if __name__ == "__main__":
    main()
