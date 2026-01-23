SHELL := /bin/bash -O globstar

.PHONY: conan
conan:
	rm -rf ~/.conan/data/Perfetto/*/shynur/dev
	conan create . shynur/dev --profile:build conan.build.ini --profile:host conan.host.ini
	[ shynur = `whoami` ] && conan upload --parallel -c --force --all -r my Perfetto/\*@shynur/dev

.PHONY: install
install: build
	rm -rf install
	cmake --install build --prefix install

.PHONY: build
build:
	rm -rf build
	cmake -B build -S . --preset dev
	cmake --build build --parallel

.PHONY: clean
clean:
	rm -rf {.,test_package}/build/ install/
	rm -f test_package/CMakeUserPresets.json
