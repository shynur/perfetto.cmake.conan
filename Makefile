SHELL := /bin/bash -O globstar

.PHONY: install
install: build
	rm -rf install
	cmake --install build --prefix install

.PHONY: build
build:
	rm -rf build
	cmake -B build -DBUILD_SHARED_LIBS=ON
	cmake --build build

.PHONY: clean
clean:
	rm -rf build/ install/
