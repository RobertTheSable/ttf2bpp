from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout

class Ttf2BppConan(ConanFile):
    name = "ttf2bpp"
    version = "0.1.0"
    license = "MIT License"
    settings = "os", "compiler", "build_type", "arch"
    requires = "yaml-cpp/0.6.3", "cxxopts/2.2.0"
    options = {"use_system_png": [True, False], "use_system_xml": [True, False], "use_system_boost": [True, False], "use_system_icu": [True, False], "use_system_freetype": [True, False]}
    default_options = {"use_system_png": False, "use_system_xml": False, "use_system_boost": False, "use_system_icu": False, "use_system_freetype": False}
    
    def layout(self):    
        cmake_layout(self)
        
    def requirements(self):
        if not self.options.use_system_png:
            self.requires("pngpp/0.2.10")
            self.requires("libpng/1.6.39")
        if not self.options.use_system_xml:
            self.requires("expat/2.5.0")
        if not self.options.use_system_boost:
            self.requires("boost/1.81.0")
        if not self.options.use_system_icu:
            self.requires("icu/72.1")
        if not self.options.use_system_freetype:
            self.requires("freetype/2.11.1")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()
        tc = CMakeToolchain(self)
        if not self.options.use_system_xml:
            tc.variables["expat_DIR"] = "${CMAKE_CURRENT_LIST_DIR}"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install() 
