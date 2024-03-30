import sys
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


@dataclass
class Config:
    width: int = 100
    height: int = 100
    rows: int = 5
    cols: int = 5
    scenes: int = 1
    objtype: str = 'random'


def parse_config(args: list[str]):
    config = Config()
    i = 1

    while i < len(args):
        if args[i] == "-r":
            res_name = args[i + 1]

            if res_name not in RESOLUTIONS:
                print("invalid resolution")
                sys.exit(1)

            res = RESOLUTIONS[res_name]
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
        elif args[i] == "-o":
            config.objtype = args[i + 1]

        i += 2

    return config
