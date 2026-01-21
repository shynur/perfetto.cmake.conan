'''
Use with Conan v1.x
'''

import os, sys
import conan, conan.tools.cmake, conan.tools.files

class QuickJSConan(conan.ConanFile):
    print(f'RUNNING {__file__} @ {os.getcwd()}', file=sys.stderr)

    name = 'Perfetto'
    url = 'https://github.com/shynur/perfetto.cmake.conan'

    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'shared': [True, False],
        'fPIC': [True, False],
    }
    default_options = {
        'shared': True,
        'fPIC': True,
    }

    exports_sources = (
        'CMakeLists.txt',
        'cmake/*',
        'src/*',
    )

    def set_version(self):
        print(f'RUNNING {__file__} @ {os.getcwd()}', file=sys.stderr)
        self.version = '1'

    def config_options(self):
        if self.settings.os == 'Windows':
            del self.options.fPIC

    def layout(self):
        conan.tools.cmake.cmake_layout(self)

    def generate(self):
        tc = conan.tools.cmake.CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = conan.tools.cmake.CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = conan.tools.cmake.CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property('cmake_file_name', 'Perfetto')

        self.cpp_info.components['perfetto'].set_property('cmake_target_name', 'Perfetto::perfetto')
        self.cpp_info.components['perfetto'].libs = ['perfetto']
