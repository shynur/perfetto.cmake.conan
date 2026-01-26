SHELL := /bin/bash -O globstar

.PHONY: conan
conan: install-cmake fetch-perfetto-git-tags
	rm -rf ~/.conan/data/Perfetto/*/shynur/dev
	conan create . shynur/dev `[ shynur = \`whoami\` ] && echo '' --profile:build conan.build.ini --profile:host conan.host.ini`
	if [ shynur = `whoami` ]; then conan upload --parallel -c --force --all -r my Perfetto/\*@shynur/dev; fi

.PHONY: install
install: build
	rm -rf install
	cmake --install build --prefix install

.PHONY: build
build: install-cmake fetch-perfetto-git-tags
	rm -rf build
	cmake -B build -S . --preset dev
	cmake --build build --parallel

.PHONY: fetch-perfetto-git-tags
fetch-perfetto-git-tags:
	cd perfetto;                                       \
	if ! [ git describe --tags --abbrev=0 ]; then      \
		git fetch tags;                            \
	fi

.PHONY: install-cmake
install-cmake:
	./contrib/install-cmake.bash

.PHONY: clean
clean:
	rm -rf {.,test_package}/build/ install/
	rm -f test_package/CMakeUserPresets.json
