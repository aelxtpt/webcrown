from conan import ConanFile
from conan.tools.build import build_jobs

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "date/*:header_only": True,
        "refl-cpp/*:header-only": True,
        "boost/*:without_python": True, 
        "boost/*:without_test": True,
        "boost/*:without_atomic": False,
        "boost/*:without_chrono": False,
        "boost/*:without_container": False,
        "boost/*:without_context": True,
        "boost/*:without_coroutine": True,
        "boost/*:without_date_time": False,
        "boost/*:without_fiber": True,
        "boost/*:without_filesystem": True,
        "boost/*:without_graph": True,
        "boost/*:without_graph_parallel": True,
        "boost/*:without_iostreams": True,
        "boost/*:without_json": True,
        "boost/*:without_locale": True,
        "boost/*:without_log": True,
        "boost/*:without_math": True,
        "boost/*:without_mpi": True,
        "boost/*:without_nowide": True,
        "boost/*:without_program_options": True,
        "boost/*:without_random": True,
        "boost/*:without_regex": True,
        "boost/*:without_serialization": True,
        "boost/*:without_stackstrace": True,
        "boost/*:without_system": False,
        "boost/*:without_thread": False,
        "boost/*:without_timer": True,
        "boost/*:without_wave": True,
    }

    def requirements(self):
        self.requires("asio/1.24.0")
        self.requires("date/3.0.1")
        self.requires("nlohmann_json/3.11.3")
        self.requires("libpqxx/7.6.1")
        self.requires("refl-cpp/0.12.4")
        self.requires("argon2/20190702")
        self.requires("fmt/10.0.0")
        self.requires("inja/3.4.0")
        self.requires("boost/1.81.0")

    def build_requirements(self):
        self.tool_requires("cmake/3.19.8")