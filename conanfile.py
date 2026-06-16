from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import collect_libs
from collections import namedtuple

class LibBiomEvalConan(ConanFile):
    name = 'biomeval'
    license = 'BSD 2-clause'
    url = 'https://github.com/usnistgov/libbiomeval'
    description = "The NIST Image Group's Biometric Evaluation Framework is a set of C++ classes, error codes, and design patterns used to create a common environment to provide logging, data management, error handling, and other functionality that is needed for many applications used in the testing of biometric software."
    settings = 'os', 'compiler', 'arch', 'build_type'
    package_type = "library"
    version = "12.0"

    exports_sources = "*"

    options = {
        'fPIC': [True, False],
        'shared': [True, False],
        'with_ffmpeg': [True, False], #True	Build sources that require FFMPEG	Unavailable when FORCE_STATIC_DEPENDENCIES is ON
        'with_hwloc': [True, False], #True Build sources that require libhwloc	
        'with_mpi': [True, False], #True Build sources that require OpenMPI	
        'with_pcsc': [True, False], #True Build sources that require PCSC
        'with_libdb': [True, False], #True Build sources that require libdb
        'build_for_wasm': [True, False] #Use WASM exceptions (instead of JavaScript exceptions)
    }
    default_options = {
        'fPIC': True,
        'shared': False,
        'with_ffmpeg': False,
        'with_hwloc': False,
        'with_mpi': False,
        'with_pcsc': False,
        'with_libdb': False,
        'build_for_wasm': True
    }
    
    _BiomevalComponent = namedtuple(
        "_BiomevalComponent",
        ("option", "requires", "external_requires", "system_libs")
    )

    _component_tree = {
        "core": _BiomevalComponent(
            None,
            [],
            ["openssl::openssl"],
            [],
        ),
        "io": _BiomevalComponent(
            None,
            ["biomeval_core"],
            ["zlib::zlib"],
            [],
        ),
        "recordstore": _BiomevalComponent(
            None,
            ["biomeval_io"],
            ["sqlite3::sqlite3", "zlib::zlib"],
            [],
        ),
        "image": _BiomevalComponent(
            None,
            ["biomeval_core"],
            [
                "openjpeg::openjpeg",
                "libjpeg-turbo::libjpeg-turbo",
                "libpng::libpng",
                "libtiff::libtiff",
                "zlib::zlib",
            ],
            [],
        ),
        "nbis": _BiomevalComponent(None, ["biomeval_core"], [], []),
        "video": _BiomevalComponent("with_ffmpeg", ["biomeval_core"], ["ffmpeg::ffmpeg"], []),
        "device": _BiomevalComponent("with_pcsc", ["biomeval_core"], [], []),
        "mpibase": _BiomevalComponent("with_mpi", ["biomeval_core"], ["openmpi::openmpi"], []),
        "mpidistributor": _BiomevalComponent("with_mpi", ["biomeval_mpibase"], ["openmpi::openmpi"], []),
        "mpireceiver": _BiomevalComponent("with_mpi", ["biomeval_mpibase"], ["openmpi::openmpi"], []),
        "system": _BiomevalComponent("with_hwloc", ["biomeval_core"], ["hwloc::hwloc"], []),
    }

    def requirements(self):
        if self.options.with_ffmpeg:
            self.requires("ffmpeg/[>=7.0.1]")
        if self.options.with_mpi:
            if self.settings.os != "Windows":
                self.requires("openmpi/[>=4.1.6]")
            else:
                self.output.warn("OpenMPI support is not available on Windows. Disabling MPI.")
                # Alternate not provided in conan yet https://github.com/Microsoft/Microsoft-MPI
        if self.options.with_libdb and not self.options.build_for_wasm:
            if self.settings.os != "Windows":
                self.requires("libdb/[>=5.3.28]")
            else:
                self.output.warn("Berkeley DB support is not available on Windows. Disabling libdb.")
                # Conan recipe broken on windows, only builds legacy VC10-
        if self.settings.os != "Windows" and self.options.with_hwloc:
            self.requires("hwloc/[>=2]")
        if self.settings.os == 'Linux' and self.options.with_pcsc:
            self.requires("libpcsclite/system")

        self.requires("openjpeg/[>=2]")
        self.requires("libjpeg-turbo/[>=3]")
        self.requires("libpng/1.6.47")
        self.requires("libtiff/[>=4.7.0]")
        self.requires("openssl/[~3]")
        self.requires("sqlite3/[>=3.45 <4]")
        self.requires("zlib/[>=1.3.0 <2]")

    def build_requirements(self):
        if self.options.with_ffmpeg:
            self.tool_requires("pkgconf/2.0.3")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        self.options["libtiff"].jpeg = "libjpeg-turbo"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables['CMAKE_BUILD_TYPE'] = self.settings.build_type
        tc.variables['BUILD_SHARED_LIBS'] = self.options.shared
        # if self.settings.os == "Windows" and self.settings.arch == "x86":
        #     tc.variables['BUILD_BIOMEVAL_32BIT'] = 'ON'
        # FORCE_STATIC_DEPENDENCIES  DBUILD_SHARED_LIBS=OFF 
        tc.variables['BUILD_BIOMEVAL_TESTS'] = 'OFF'
        tc.variables['WITH_FFMPEG'] = self.options.with_ffmpeg and self.options.shared
        tc.variables['WITH_HWLOC'] = self.options.with_hwloc
        tc.variables['WITH_MPI'] = self.options.with_mpi
        tc.variables['WITH_PCSC'] = self.options.with_pcsc
        tc.variables['WITH_LIBDB'] = self.options.with_libdb
        tc.variables['BUILD_FOR_WASM'] = self.options.build_for_wasm  # avoid (RECORDSTORE) and need for berkley db
        tc.variables['CMAKE_FIND_PACKAGE_PREFER_CONFIG'] = 'ON'
        tc.generate() 

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def _is_component_enabled(self, component_name, component):
        if component.option and not self.options.get_safe(component.option, False):
            return False
        if component_name == "recordstore":
            return not self.options.build_for_wasm
        if component_name == "video":
            return bool(self.options.with_ffmpeg and self.options.shared)
        if component_name in {"mpibase", "mpidistributor", "mpireceiver", "system", "device"}:
            return self.settings.os != "Windows"
        return True

    # def package(self):
    #     if self.settings.os == 'Windows':
    #         package_lib_dir = os.path.join(self.package_folder, 'lib')
    #         os.rename(os.path.join(package_lib_dir, 'fido2_static.lib'), os.path.join(package_lib_dir, 'fido2.lib'))

    def package_info(self):
        self.cpp_info.set_property('cmake_file_name', 'biomeval')
        self.cpp_info.set_property('cmake_target_name', 'biomeval::biomeval')

        package_libs = collect_libs(self)

        for component_name, component in self._component_tree.items():
            if not self._is_component_enabled(component_name, component):
                continue

            conan_component = f'biomeval_{component_name}'
            component_info = self.cpp_info.components[conan_component]
            component_info.set_property('cmake_target_name', f'biomeval::{component_name}')
            component_info.requires = list(component.requires) + list(component.external_requires)
            component_info.system_libs = list(component.system_libs)

            if component_name == 'recordstore' and self.options.with_libdb and self.settings.os != 'Windows' and not self.options.build_for_wasm:
                component_info.requires.append('libdb::libdb')

            if component_name == 'core':
                component_info.libs = package_libs
                if self.settings.os == 'Windows':
                    component_info.system_libs.append('ws2_32')
                    if not self.options.shared:
                        component_info.system_libs.append('crypt32')
            elif component_name == 'device' and self.settings.os == 'Linux':
                component_info.system_libs.append('pcsclite')
        # if self.settings.os == 'Windows':
        #     self.cpp_info.libs.append('hid.lib')
