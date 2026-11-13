XMAKE       ?= xmake
PYTHON      ?= python3
CTYPESGEN   ?= ctypesgen
UV          ?= uv
CLANG_FORMAT ?= clang-format

FORMAT_DIRS  = include src tests third_party/fosu/src third_party/fosu/tests
FORMAT_FILES = $(shell find $(FORMAT_DIRS) -type f \( -name '*.h' -o -name '*.cpp' -o -name '*.cc' \) 2>/dev/null)

.PHONY: all configure build release build-python-wheel rebuild \
        test-auto test-scalar test-avx2 test-neon test-python \
        generate-parity-tests generate-mods fetch-mods generate-python \
        format check check-format check-headers check-tidy \
        build-windows-2k build-windows-xp-x64

all: build

configure:
	@echo "Configuring pppp with xmake:"
	$(XMAKE) f -y -c -m debug --pppp_warnings_as_errors=y

build: configure
	@echo "Building pppp:"
	$(XMAKE) -y

release:
	$(XMAKE) f -y -c -m release --pppp_build_tests=n
	$(XMAKE) -y

build-python-wheel: release
	@$(UV) build --wheel --no-build-isolation

rebuild: clean build test-auto

test-auto: build
	@echo "Testing pppp (pppp_isa=auto):"
	$(XMAKE) test
	$(XMAKE) test_install

test-scalar:
	@echo "Testing pppp (pppp_isa=scalar):"
	$(XMAKE) f -y -c -m debug --pppp_isa=scalar
	$(XMAKE) -y
	$(XMAKE) test
	$(XMAKE) test_install

test-avx2:
	@echo "Testing pppp (pppp_isa=avx2) - needs an x86-64 host:"
	$(XMAKE) f -y -c -m debug --pppp_isa=avx2
	$(XMAKE) -y
	$(XMAKE) test
	$(XMAKE) test_install

test-neon:
	@echo "Testing pppp (pppp_isa=neon) - needs an AArch64 host:"
	$(XMAKE) f -y -c -m debug --pppp_isa=neon
	$(XMAKE) -y
	$(XMAKE) test
	$(XMAKE) test_install

test-python: build
	@$(PYTHON) -m pytest python/tests -q
	@$(PYTHON) -m mypy

clean:
	rm -rf build .xmake

format:
	$(CLANG_FORMAT) -i $(FORMAT_FILES)

check: check-format check-headers check-tidy

check-format:
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_FILES)

check-headers:
	$(XMAKE) check_headers

check-tidy: configure
	$(XMAKE) check clang.tidy || true

fetch-mods:
	$(PYTHON) codegen/mods/fetch_mods.py

generate-mods:
	$(PYTHON) codegen/mods/gen_mods.py
	$(CLANG_FORMAT) -i src/pppp/mods/generated_mods.h src/pppp/mods/generated_mods.cpp

generate-parity-tests:
	$(PYTHON) tests/parity/gen_parity.py

generate-python:
	@$(CTYPESGEN) --no-embed-preamble -l pppp_c --strip-build-path "$(CURDIR)" \
	  --header-template /dev/null -I include -o python/pppp/_ctypes/_native.py include/pppp/capi.h

build-windows-2k build-windows-xp-x64:
	@echo "Legacy Windows cross-compilation is deferred to the xmake port; see docs/portability.md."
	@false
