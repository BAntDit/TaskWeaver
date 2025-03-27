
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class TaskWeaverRecipe(ConanFile):
    name = "taskweaver"
    version = "0.3.0.0"
    license = "This project is licensed under the MIT License. Copyright (c) 2025 Anton Sedov Henrikhovich"
    author = "Anton Sedov Henrikhovich"
    url = "https://github.com/BAntDit/TaskWeaver"
    description = "A multithreaded task scheduler utilizing lock-free task-stealing deques."

    settings = "os", "build_type"

    exports_sources = "CMakeLists.txt", "*.cmake", ".clang-format", ".md", "src/*.h", "tests/*", "cmake/*"

    def requirements(self):
        self.requires("gtest/[~1.16]")
        self.requires("metrix/[~1.5]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.10]")

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

        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]