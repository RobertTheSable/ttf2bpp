from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout

class Ttf2BppConan(ConanFile):
    name = "ttf2bpp"
    version = "0.3.0"
    license = "MIT License"
    settings = "os", "compiler", "build_type", "arch"
    requires = "yaml-cpp/0.6.3", "cxxopts/2.2.0"
    options = {"use_system_png": [True, False], "use_system_xml": [True, False], "use_system_icu": [True, False], "use_system_freetype": [True, False], "use_boxer": [True, False]}
    default_options = {"use_system_png": False, "use_system_xml": False, "use_system_icu": False, "use_system_freetype": False, "use_boxer": False}
    
    def layout(self):    
        cmake_layout(self)
        
    def requirements(self):
        if not self.options.use_system_png:
            self.requires("pngpp/0.2.10")
            self.requires("libpng/1.6.40")
        if not self.options.use_system_xml:
            self.requires("expat/2.5.0")
        if not self.options.use_system_icu:
            self.requires("icu/72.1")
        if not self.options.use_system_freetype:
            self.requires("freetype/2.13.2")
        if self.options.use_boxer:
            self.requires("boxer/1.0.0")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()
        tc = CMakeToolchain(self)
        if not self.options.use_system_xml:
            tc.variables["expat_DIR"] = "${CMAKE_CURRENT_LIST_DIR}"
        if self.options.use_boxer:
            tc.variables["TTF_USE_BOXER"] = "On"
        if not self.options.use_system_icu:
            icu = self.dependencies["icu"]
            if icu.options.get_safe("data_packaging") in ["files", "archive"]:
                tc.variables["ICU_DATA_FILE"] = self.dependencies["icu"].runenv_info.vars(self).get("ICU_DATA")
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install() 
