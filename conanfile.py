# Conan recipe for building the SvcWrapper library.
# Copyright (c) LASERVORM GmbH 2023

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import copy
from conans.errors import ConanException, ConanInvalidConfiguration
from pathlib import Path
from re import finditer

class SvcWrapperConan(ConanFile):
    name = "SvcWrapper"
    license = "MIT"
    author = "Dominic Ernst <dominic.ernst@laservorm.com>"
    homepage = "https://github.com/LASERVORM/SvcWrapper"
    url = "https://github.com/LASERVORM/SvcWrapper"
    description = "Library for wrapping any C/C++ application into a Windows service."
    topics = ("service", "windows")
    generators = "VirtualBuildEnv"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "cmake/*"
    exports = "README.md", "LICENSE"

    def set_version(self):
        versionFile = Path(self.recipe_folder).joinpath("CMakeLists.txt")
        if not versionFile.exists() or not versionFile.is_file():
            raise ConanException(f"Missing file to read version from: {versionFile.name}")
        vNums = {}
        with open(versionFile.as_posix()) as fh:
            matches = finditer(r"set\(LIB_VERSION_(MAJOR|MINOR|PATCH)\s([0-9]+)\)", fh.read())
            vNums = {m.group(1): m.group(2) for m in matches}
        if not all(k in vNums for k in ("MAJOR", "MINOR", "PATCH")):
            raise ConanException(f"Failed to read package version from {versionFile.name}!")
        self.version = f"{vNums['MAJOR']}.{vNums['MINOR']}.{vNums['PATCH']}"
    
    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["SVCWRAPPER_EXAMPLE"] = False
        tc.generate()
    
    def build(self):
        cm = CMake(self)
        cm.configure()
        cm.build()
    
    def package(self):
        cm = CMake(self)
        cm.install()
