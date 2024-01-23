from conan import ConanFile
from conan.tools.build import build_jobs

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "date/*:header_only": True,
        "refl-cpp/*:header-only": True,
    }

    def requirements(self):
        self.requires("asio/1.24.0")
        self.requires("date/3.0.1")
        self.requires("nlohmann_json/3.11.2")
        self.requires("libpqxx/7.6.1")
        self.requires("refl-cpp/0.12.4")
        self.requires("argon2/20190702")
        self.requires("fmt/10.0.0")

    def build_requirements(self):
        self.tool_requires("cmake/3.19.8")