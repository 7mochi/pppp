BUILD_DIR   := build
NPROC := $(shell nproc 2>/dev/null || echo 4)

PGO_BUILD_DIR := build_pgo
PGO_DIR = $(CURDIR)/$(PGO_BUILD_DIR)-profile-data
PGO_TRAINING := parity_difficulty|parity_performance

LLVM_PROFDATA := $(dir $(realpath $(shell which $(CXX) 2> /dev/null)))llvm-profdata
ifeq ($(wildcard $(LLVM_PROFDATA)),)
	LLVM_PROFDATA := llvm-profdata
endif

ifeq ($(shell uname -s 2>/dev/null),Darwin)
	XCRUN = xcrun
endif

cxxisclang := $(findstring clang,$(shell $(CXX) --version 2>/dev/null))

.PHONY: all clean rebuild test-auto test-scalar test-avx2 test-neon generate-parity-tests configure format check-format check-tidy build-windows-2k build-windows-xp-x64 build-pgo clean-pgo-build clean-pgo

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

build-pgo: clean-pgo-build clean-pgo
	@echo "Step 1/4. Building instrumented library ..."
	cmake --preset pgo -DPPPP_PGO=GENERATE -DPPPP_PGO_DIR=$(PGO_DIR)
	cmake --build --preset pgo
	@echo "Step 2/4. Running the parity suites for pgo-build ..."
	ctest --test-dir $(PGO_BUILD_DIR) -R "^($(PGO_TRAINING))$$" --output-on-failure
	@echo "Step 3/4. Building optimized library ..."
ifneq ($(cxxisclang),)
	$(XCRUN) $(LLVM_PROFDATA) merge -output=$(PGO_DIR)/default.profdata $(PGO_DIR)/*.profraw
endif
	cmake --build --preset pgo --target clean
	cmake --preset pgo -DPPPP_PGO=USE
	cmake --build --preset pgo
	@echo "Step 4/4. Deleting profile data ..."
	$(MAKE) clean-pgo

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

clean-pgo-build:
	rm -rf $(PGO_BUILD_DIR)

clean-pgo:
	rm -rf $(PGO_DIR)
	if [ -d $(PGO_BUILD_DIR) ]; then find $(PGO_BUILD_DIR) -name '*.gcda' -delete; fi

clean: clean-pgo-build clean-pgo
	rm -rf $(BUILD_DIR) build_scalar build_avx2 build_neon

rebuild: clean build test-auto

format: configure
	cmake --build $(BUILD_DIR) --target format

check-format: configure
	cmake --build $(BUILD_DIR) --target check_format

check-tidy:
	cmake --preset clang_tidy
	cmake --build build_clang_tidy -j$(NPROC) -- -k 0 || true

generate-parity-tests:
	python3 tests/parity/gen_parity.py
