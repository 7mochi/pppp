BUILD_DIR   := build
PYTHON      ?= python3
NPROC := $(shell nproc 2>/dev/null || echo 4)

.PHONY: all clean rebuild test-auto test-scalar test-avx2 test-neon generate-parity-tests generate-mods fetch-mods configure format check check-format check-tidy build-windows-2k build-windows-xp-x64

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

test-auto: build
	@echo "Testing pppp (PPPP_ISA=auto):"
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

rebuild: clean build test-auto

format: configure
	cmake --build $(BUILD_DIR) --target format

check: check-format check-tidy

check-format: configure
	cmake --build $(BUILD_DIR) --target check_format

check-tidy:
	cmake --preset clang_tidy
	cmake --build build_clang_tidy -j$(NPROC) -- -k 0 || true

fetch-mods:
	$(PYTHON) codegen/mods/fetch_mods.py

generate-mods:
	$(PYTHON) codegen/mods/gen_mods.py
	clang-format -i src/pppp/mods/generated_mods.h src/pppp/mods/generated_mods.cpp

generate-parity-tests:
	$(PYTHON) tests/parity/gen_parity.py
