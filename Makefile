.PHONY: all configure build test docs clean coverage format coverage-configure coverage-build asteroids

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

asteroids:
	cmake -B $(BUILD_DIR) -DBUILD_TESTING=ON -DIRATA2_ENABLE_SDL=ON
	cmake --build $(BUILD_DIR) --target irata2_asm irata2_demo --parallel $(JOBS)
	mkdir -p $(BUILD_DIR)/demos
	./$(BUILD_DIR)/assembler/irata2_asm demos/asteroids.asm $(BUILD_DIR)/demos/asteroids.bin $(BUILD_DIR)/demos/asteroids.json
	./$(BUILD_DIR)/frontend/irata2_demo --rom $(BUILD_DIR)/demos/asteroids.bin --fps 30 --scale 3
