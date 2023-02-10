# Conan recipe for building the SvcWrapper library test package.
# Copyright (c) LASERVORM GmbH 2023

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake

class SvcWrapperTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "VirtualBuildEnv", "VirtualRunEnv", "CMakeToolchain", "cmake_paths"

    def build(self):
        cm = CMake(self)
        cm.configure()
        cm.build()
    
    def test(self):
        cm = CMake(self)
        cm.test()