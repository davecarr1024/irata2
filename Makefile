.PHONY: all configure build test docs clean coverage format coverage-configure coverage-build

BUILD_DIR ?= build
JOBS ?= $(shell nproc)

all: build

format:
	cmake -B $(BUILD_DIR) -DBUILD_TESTING=ON
	cmake --build $(BUILD_DIR) --target fmt --parallel $(JOBS)

configure: format
	cmake -B $(BUILD_DIR) -DBUILD_TESTING=ON

build: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure -j $(JOBS)

docs: build
	cmake --build $(BUILD_DIR) --target docs --parallel $(JOBS)

coverage-configure:
	cmake -B $(BUILD_DIR) -DENABLE_COVERAGE=ON -DBUILD_TESTING=ON

coverage-build: coverage-configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

coverage: coverage-build
	ctest --test-dir $(BUILD_DIR) --output-on-failure -j $(JOBS)
	./scripts/coverage.sh $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
