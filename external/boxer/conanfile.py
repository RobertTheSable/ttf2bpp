from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout

class Ttf2BppConan(ConanFile):
    name = "boxer"
    version = "1.0.0"
    url = "https://github.com/aaronmjacobs/Boxer"
    license = "MIT License"
    settings = "os", "compiler", "build_type", "arch"
    
    def layout(self):    
        cmake_layout(self)

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install() 
