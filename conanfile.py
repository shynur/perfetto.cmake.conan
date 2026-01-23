'''
Use with Conan v1.x
'''

import os, sys, subprocess
import conan, conan.tools.cmake, conan.tools.files

class PerfettoConan(conan.ConanFile):
    print(f'RUNNING {__file__} @ {os.getcwd()}', file=sys.stderr)

    name = 'Perfetto'
    url = 'https://github.com/shynur/perfetto.cmake.conan'

    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'shared': [True, False],
        'fPIC': [True, False],
    }
    default_options = {
        'shared': False,  # Alan 给 SDK 加了一个 weak 函数及调用, 必须用 obj 格式以允许覆盖它的定义.
        'fPIC': True,
    }

    subprocess.run(
        ['bash', '-c', 'if [ -d perfetto ]; then cd perfetto; git config --global --add safe.directory "$PWD"; git describe --tags --abbrev=0 >VERSION; fi'],
        cwd='.',
        stdout=subprocess.PIPE,
        universal_newlines=True
    )
    exports_sources = (
        'CMakeLists.txt',
        'cmake/*',
        'perfetto/*',
        'polyfill/*',
        'patches/*',
    )

    def set_version(self):
        print(f'RUNNING {__file__} @ {os.getcwd()}, in set_version', file=sys.stderr)

        subprocess.run(
            ['bash', '-c', 'git config --global --add safe.directory "$PWD"'],
            cwd='perfetto',
        )
        perfetto_git_tag = subprocess.run(
            ['git', 'describe', '--tags', '--abbrev=0'],
            cwd='perfetto',
            stdout=subprocess.PIPE,
            universal_newlines=True
        )
        self.version = perfetto_git_tag.stdout.strip()
        print(f'Perfetto {self.version}', file=sys.stderr)

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
