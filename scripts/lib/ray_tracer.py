from dataclasses import dataclass


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

    def __ne__(self, __value: object) -> bool:
        return not (self == __value)


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

        if self.vup != Vec(0, 1, 0):
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
            desc += f": albedo={self.albedo} {alpha_key}={self.alpha:.3f}"

        return desc


@dataclass
class Surface:
    type_: str
    props: dict

    def __str__(self) -> str:
        desc = f"{self.type_}:"

        if self.type_ == "triangle":
            desc += f" {self.props['p1']} {self.props['p2']} {self.props['p3']} material={self.props['material']}"
        else:
            for k in self.props.keys():
                v = self.props[k]
                if type(v) == float:
                    desc += f" {k}={v:.3f}"
                else:
                    desc += f" {k}={v}"

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
