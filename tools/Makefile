# Thin wrapper around CMake, meant to be run from any directory:
#   make -f tools/Makefile build
# Variables can be overridden, e.g. BUILD_DIR=out/a make -f tools/Makefile build

ROOT      := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
BUILD_DIR ?= $(ROOT)/build
JOBS      ?= $(shell nproc 2>/dev/null || echo 4)
SAN_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer -g

.PHONY: all configure build test sanitize labs clean

all: build

configure:
	cmake -S $(ROOT)/code -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=RelWithDebInfo

build: configure
	cmake --build $(BUILD_DIR) -j $(JOBS)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

sanitize:
	cmake -S $(ROOT)/code -B $(BUILD_DIR)-san -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_C_FLAGS="$(SAN_FLAGS)" -DCMAKE_CXX_FLAGS="$(SAN_FLAGS)"
	cmake --build $(BUILD_DIR)-san -j $(JOBS)
	ctest --test-dir $(BUILD_DIR)-san --output-on-failure

labs:
	bash $(ROOT)/tools/run_labs.sh

clean:
	rm -rf $(BUILD_DIR) $(BUILD_DIR)-san
