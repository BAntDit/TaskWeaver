
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class TaskWeaverRecipe(ConanFile):
    name = "taskweaver"
    version = "0.3.0.0"

    settings = "os", "build_type"

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

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "TaskWeaver")
        self.cpp_info.set_property("cmake_target_name", "TaskWeaver::TaskWeaver")

        self.cpp_info.libs = ["TaskWeaver"]
        self.cpp_info.requires = ["metrix"]

        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]