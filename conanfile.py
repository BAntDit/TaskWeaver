
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class TaskWeaverRecipe(ConanFile):
    name = "taskweaver"
    version = "0.1.0.0"

    settings = "build_type"

    def requirements(self):
        self.requires("gtest/[~1.16]")
        self.requires("metrix/[~1.5]")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()