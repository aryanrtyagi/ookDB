
import os
from pathlib import Path

from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ROOT = Path(__file__).resolve().parent

source_file = ROOT / "cpp_files.txt"

sources = [
    line.strip().strip('"')
    for line in source_file.read_text(encoding="utf-8-sig").splitlines()
    if line.strip()
]

# The REPL has its own main() and is not part of the Python extension.
sources = [
    source for source in sources
    if not source.replace("\\", "/").endswith("src/cli/repl/repl.cpp")
]

sources.append("bindings/python/bindings.cpp")

# Keep the same C++17 standard on both platforms.
compile_args = (
    ["/std:c++17", "/O2"]
    if os.name == "nt"
    else ["-std=c++17", "-O2"]
)

extension = Pybind11Extension(
    "arydb",
    sources=sources,
    include_dirs=[
        str(ROOT),
        str(ROOT / "src"),
    ],
    cxx_std=17,
    extra_compile_args=compile_args,
)

setup(
    name="arydb",
    version="0.1.0",
    description="AryDB C++ Database Engine Python Bindings",
    ext_modules=[extension],
    cmdclass={"build_ext": build_ext},
)
