
from conan import ConanFile

class TaskWeaverRecipe(ConanFile):
    settings = "build_type"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("gtest/[~1.16]")