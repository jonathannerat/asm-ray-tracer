#!/usr/bin/env python3

import os
import sys
from random import random as frand, choices as rand_choices, choice as rand_choice
from dataclasses import dataclass


RESOLUTIONS = {
    "small": [100, 100],
    "medium": [200, 200],
    "large": [400, 400],
    "vga": [640, 480],
    "sd": [720, 480],
    "hd": [1280, 720],
    "fullhd": [1920, 1080],
}

MATERIAL_TYPES = ["lambertian", "metal", "dielectric", "light"]
MATERIAL_WEIGHTS = [0.5, 0.35, 0.1, 0.05]

OBJECT_TYPES = ["box", "sphere"]
SCENES_DIR = os.path.join("scenes", "generated")


@dataclass
class Vec:
    x: float
    y: float
    z: float

    def __str__(self) -> str:
        return f"{self.x:.3f},{self.y:.3f},{self.z:.3f}"

    def __eq__(self, __value: object) -> bool:
        if not isinstance(__value, Vec):
            return False

        return self.x == __value.x and self.y == __value.y and self.z == __value.z


@dataclass
class Output:
    width: int
    height: int
    spp: int
    depth: int

    def __str__(self) -> str:
        return f"output: width={self.width} height={self.height} spp={self.spp} depth={self.depth}"


class Camera:
    from_: Vec
    to: Vec
    vup: Vec
    vfov: float
    aperture: float

    def __init__(self, from_, to, vup=Vec(0, 1, 0), vfov=45, aperture=2) -> None:
        self.from_ = from_
        self.to = to
        self.vup = vup
        self.vfov = vfov
        self.aperture = aperture

    def __str__(self) -> str:
        desc = f"camera: from={self.from_} to={self.to}"

        if not (self.vup == Vec(0, 1, 0)):
            desc += " vup=" + str(self.vup)

        if self.vfov != 45:
            desc += " vfov=" + str(self.vfov)

        if self.aperture != 2:
            desc += " aperture=" + str(self.aperture)

        return desc


@dataclass
class Material:
    type_: str
    albedo: Vec
    alpha: float = 0

    def __str__(self) -> str:
        desc = f"{self.type_}"

        if self.type_ == "light" or self.type_ == "lambertian":
            desc += "=" + str(self.albedo)
        else:
            alpha_key = "fuzz" if self.type_ == "metal" else "ir"
            desc += f": albedo={self.albedo} {alpha_key}={self.alpha}"

        return desc


@dataclass
class Surface:
    type_: str
    props: dict

    def __str__(self) -> str:
        desc = f"{self.type_}:"

        for k in self.props.keys():
            if type(self.props[k]) == float:
                desc += f" {k}={self.props[k]:.3f}"
            else:
                desc += f" {k}={self.props[k]}"

        return desc


@dataclass
class GeneratedScene:
    output: Output
    camera: Camera
    materials: list[Material]
    objects: list[Surface]

    def __str__(self) -> str:
        desc = str(self.output) + "\n"
        desc += str(self.camera) + "\n"
        desc += "materials:\n"
        for m in self.materials:
            desc += "    " + str(m) + "\n"
        desc += "objects:\n"
        for o in self.objects:
            desc += "    " + str(o) + "\n"
        return desc


@dataclass
class Config:
    width: int = 100
    height: int = 100
    rows: int = 5
    cols: int = 5
    scenes: int = 1


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
        elif args[i] == "-s":
            config.scenes = int(args[i + 1])

        i += 2

    return config


def generate_materials(config: Config):
    n = config.rows * config.cols
    res = []

    for _ in range(n):
        mtype = rand_choices(MATERIAL_TYPES, MATERIAL_WEIGHTS)[0]
        res.append(Material(mtype, Vec(frand(), frand(), frand()), frand()))

    return res


def generate_objects(config: Config):
    res = []
    mat_index = 0

    for y in range(0, config.rows):
        for x in range(0, config.cols):
            otype = rand_choice(OBJECT_TYPES)
            side = frand() * 0.7 + 0.2
            cx = x + 0.5 - side / 2
            cz = y + 0.5 - side / 2

            if otype == "box":
                res.append(
                    Surface(
                        otype,
                        {
                            "pmin": Vec(cx, 0, cz),
                            "pmax": Vec(cx + side, side, cz + side),
                            "material": mat_index,
                        },
                    )
                )
            else:
                res.append(
                    Surface(
                        otype,
                        {
                            "center": Vec(x + 0.5, side / 2, x + 0.5),
                            "radius": side / 2,
                            "material": mat_index,
                        },
                    )
                )

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
                    "origin": Vec(0, 8, 0),
                    "normal": Vec(0, -1, 0),
                    "material": len(materials) - 2,
                },
            )
        )
        objects.append(Surface("plane", {"material": len(materials) - 1}))
        scene = GeneratedScene(
            Output(config.width, config.height, 50, 20),
            Camera(Vec(-5, 5, -5), Vec(config.cols / 2, 1, config.rows / 2)),
            materials,
            objects,
        )

        with open(scene_file, "w") as f:
            f.write(str(scene))


if __name__ == "__main__":
    main()
