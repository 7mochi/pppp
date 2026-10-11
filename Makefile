BUILD_DIR   := build
NPROC := $(shell nproc 2>/dev/null || echo 4)

.PHONY: all clean rebuild test test-scalar test-avx2 test-neon generate-parity-tests configure format check-format check-tidy build-windows-2k build-windows-xp-x64

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

test-scalar:
	@echo "Testing pppp (PPPP_ISA=scalar):"
	cmake --preset scalar
	cmake --build --preset scalar
	ctest --test-dir build_scalar --output-on-failure

test-avx2:
	@echo "Testing pppp (PPPP_ISA=avx2) - needs an x86-64 host:"
	cmake --preset avx2
	cmake --build --preset avx2
	ctest --test-dir build_avx2 --output-on-failure

test-neon:
	@echo "Testing pppp (PPPP_ISA=neon) - needs an AArch64 host:"
	cmake --preset neon
	cmake --build --preset neon
	ctest --test-dir build_neon --output-on-failure

clean:
	rm -rf $(BUILD_DIR) build_scalar build_avx2 build_neon

rebuild: clean build test

format: configure
	cmake --build $(BUILD_DIR) --target format

check-format: configure
	cmake --build $(BUILD_DIR) --target check_format

check-tidy:
	cmake --preset clang_tidy
	cmake --build build_clang_tidy -j$(NPROC) -- -k 0 || true

generate-parity-tests:
	python3 tests/parity/gen_parity.py
	$(MAKE) format
