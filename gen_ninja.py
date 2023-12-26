import sys
import os

from ninja_syntax import Writer as NinjaWriter

DEBUG = "DEBUG" in os.environ
ASM_TRACER = "src/asm/tracer.nasm"
BUILD_DIR = "build"
OBJ_EXT = ".o"


def main():
    w = NinjaWriter(sys.stdout)
    w.variable("cflags", compiler_flags("CFLAGS", "-Wall -Wextra"))
    w.variable("asmflags", compiler_flags("ASMFLAGS", "-f elf64 -Wall -Isrc/asm"))
    w.variable("ldflags", compiler_flags("LDFLAGS", "-lm -no-pie"))

    w.rule("cc", "gcc -MD -MF $out.d $cflags -c -o $out $in", "gcc $in")
    w.variable("depfile", "$out.d", 1)
    w.variable("deps", "gcc", 1)

    w.rule("ld", "gcc -o $out $in $ldflags", "ld $out")
    w.rule("asm", "nasm -o $out $asmflags $in", "nasm $in")
    w.rule("regen_ninja", f"sh -c 'if command -v dotenv >/dev/null; then wrapper=\"dotenv run\"; fi; $$wrapper {sys.executable} $in > $out'")

    w.build("build.ninja", "regen_ninja", __file__, implicit=[".env"])
    objpaths = build_c_files(w)
    objpaths.append(build_asm_tracer(w))

    w.build("ray_tracer", "ld", objpaths)
    w.default("ray_tracer")


def compiler_flags(flags_env: str, extend: str) -> str:
    """Extend `flags_env` with `extend`, optionally adding -g if DEBUG is defined

    Args:
        flags_env: Environment variable to extend
        extend: Additional string to append
    """
    return f"{os.getenv(flags_env, '')} {'-g' if DEBUG else ''} {extend}"


def get_objpath(srcpath: str) -> str:
    """Get object path for `srcpath`, in BUILD_DIR, with OBJ_EXT

    Args:
        srcpath: Path of the source file
    """
    components = srcpath.split("/")
    filename = components[-1]
    filename = filename[: filename.rfind(".")] + OBJ_EXT

    components[0] = BUILD_DIR
    components[-1] = filename

    return os.path.join(*components)


def list_files_with_ending(
    topdir: str, ending: str, res: list[str] | None = None
) -> list[str]:
    """Get a list of files inside `topdir` whose name ends with `ending`

    Args:
        topdir: Directory to search files in
        ending: Eding that filenames have to match to be included
        res: List in which we should add the files
    """
    res = [] if res is None else res

    for dirname, subdirs, filenames in os.walk(topdir):
        for filename in filter(lambda f: f.endswith(ending), filenames):
            path = os.path.join(dirname, filename)
            res.append(path)

        for subdir in subdirs:
            list_files_with_ending(subdir, ending, res)

    return res


def build_c_files(w: NinjaWriter, topdir: str = "src") -> list[str]:
    """Include build rules in `w` for all .c files in `topdir`

    Args:
        topdir: Directory to search .c files in

    Returns:
        List of object files of every C file
    """
    objpaths = []
    for srcpath in list_files_with_ending(topdir, ".c"):
        objpath = get_objpath(srcpath)
        w.build(objpath, "cc", srcpath)
        objpaths.append(objpath)

    return objpaths


def build_asm_tracer(w: NinjaWriter) -> str:
    """Include build rule for the ASM implementation of tracer

    Returns:
        Path to the object file of the ASM tracer
    """
    nasm_files = list_files_with_ending("src/asm", ".nasm")
    objpath = get_objpath(ASM_TRACER)

    w.build(
        objpath,
        "asm",
        ASM_TRACER,
        implicit=nasm_files,
    )

    return objpath


if __name__ == "__main__":
    main()
