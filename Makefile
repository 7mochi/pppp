BUILD_DIR   := build
NPROC := $(shell nproc 2>/dev/null || echo 4)

.PHONY: all clean rebuild test configure format format-check check-tidy

all: build

configure:
	@echo "Configuring pppp:"
	cmake --preset dev

build: configure
	@echo "Building pppp:"
	cmake --build $(BUILD_DIR) -j$(NPROC)

test: build
	@echo "Running tests:"
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build test

format: configure
	cmake --build $(BUILD_DIR) --target format

format-check: configure
	cmake --build $(BUILD_DIR) --target format-check

check-tidy: configure
	cmake --build $(BUILD_DIR) --target check-tidy || true
