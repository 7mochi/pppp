BUILD_DIR   := build
NPROC := $(shell nproc 2>/dev/null || echo 4)

.PHONY: all clean rebuild test configure format check-format check-tidy build-windows-2k build-windows-xp-x64

all: build

configure:
	@echo "Configuring pppp:"
	cmake --preset dev

build: configure
	@echo "Building pppp:"
	cmake --build $(BUILD_DIR) -j$(NPROC)

build-windows-2k:
	@echo "Run this in 'nix develop .#legacyWindows':"
	cmake --preset win2k
	cmake --build --preset win2k

build-windows-xp-x64:
	@echo "Run this in 'nix develop .#legacyWindows':"
	cmake --preset winxp-x64
	cmake --build --preset winxp-x64

test: build
	@echo "Running tests:"
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build test

format: configure
	cmake --build $(BUILD_DIR) --target format

check-format: configure
	cmake --build $(BUILD_DIR) --target check_format

check-tidy:
	cmake --preset clang_tidy
	cmake --build build_clang_tidy -j$(NPROC) -- -k 0 || true
