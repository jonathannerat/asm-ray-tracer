from math import sqrt
from lib.ray_tracer import Material, Surface, Vec
from random import choice, choices, random as frand

MATERIAL_TYPES = ["lambertian", "metal", "dielectric"]
MATERIAL_WEIGHTS = [0.5, 0.35, 0.15]
OBJECT_TYPES = ["box", "sphere", "tetra"]


def random_material() -> Material:
    mtype = choices(MATERIAL_TYPES, MATERIAL_WEIGHTS)[0]
    min_alpha = 1 if mtype == "dielectric" else 0
    max_alpha = 1.5 if mtype == "dielectric" else 1

    return Material(
        mtype,
        Vec(frand(), frand(), frand()),
        min_alpha + frand() * (max_alpha - min_alpha),
    )


def generate_surface(otype: str, at: Vec, mat: int, max_size: float):
    otype = choice(OBJECT_TYPES) if otype == "random" else otype
    side = frand() * (max_size - 0.1) + 0.1
    props = {}

    props["material"] = mat
    if otype == "box":
        props["pmin"] = Vec(at.x - side / 2, at.y, at.z - side / 2)
        props["pmax"] = Vec(at.x + side / 2, at.y + side, at.z + side / 2)
    elif otype == "sphere":
        props["center"] = Vec(at.x, at.y + side / 2, at.z)
        props["radius"] = side / 2
    else:  # tetra
        points = []
        triangles = []
        half_h = sqrt(3) * side / 4

        points.append(Vec(at.x, at.y, at.z + half_h))
        points.append(Vec(at.x - side / 2, at.y, at.z - half_h))
        points.append(Vec(at.x + side / 2, at.y, at.z - half_h))
        points.append(Vec(at.x, at.y + side, at.z))

        triangles.append(
            Surface(
                "triangle",
                {"p1": points[0], "p2": points[2], "p3": points[1], "material": mat},
            )
        )

        triangles.append(
            Surface(
                "triangle",
                {"p1": points[3], "p2": points[1], "p3": points[2], "material": mat},
            )
        )

        triangles.append(
            Surface(
                "triangle",
                {"p1": points[3], "p2": points[2], "p3": points[0], "material": mat},
            )
        )

        triangles.append(
            Surface(
                "triangle",
                {"p1": points[3], "p2": points[0], "p3": points[1], "material": mat},
            )
        )

        return triangles

    return Surface(otype, props)
