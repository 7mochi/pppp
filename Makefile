XMAKE       ?= xmake
PYTHON      ?= python3
CTYPESGEN   ?= ctypesgen
UV          ?= uv
CLANG_FORMAT ?= clang-format

BUMP_VERSION ?= bump-my-version
BUMP_AUTHOR_NAME ?= 7mochi-bot
BUMP_AUTHOR_EMAIL ?= bot@osupe.ru

ARTIFACT    ?=
PART        ?= patch

FORMAT_DIRS  = include src tests third_party/fosu/src third_party/fosu/tests
FORMAT_FILES = $(shell find $(FORMAT_DIRS) -type f \( -name '*.h' -o -name '*.cpp' -o -name '*.cc' \) 2>/dev/null)

.PHONY: all configure build release bump build-python-wheel build-csharp-nupkg rebuild \
        test-auto test-scalar test-avx2 test-neon test-python test-csharp \
        fetch-mods generate-mods generate-parity-tests generate-python generate-csharp \
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

bump:
	@case "$(ARTIFACT)" in core|python|csharp) ;; *) echo "usage: make bump ARTIFACT=core|python|csharp PART=patch|minor|major"; exit 1;; esac
	@case "$(PART)" in patch|minor|major) ;; *) echo "usage: make bump ARTIFACT=core|python|csharp PART=patch|minor|major"; exit 1;; esac
	@test "$$(git branch --show-current)" = master || { echo "error: make bump must run on master"; exit 1; }
	@test -z "$$(git status --porcelain --untracked-files=no)" || { echo "error: tracked or staged changes must be committed first"; exit 1; }

	GIT_AUTHOR_NAME="$(BUMP_AUTHOR_NAME)" \
	GIT_AUTHOR_EMAIL="$(BUMP_AUTHOR_EMAIL)" \
	GIT_COMMITTER_NAME="$(BUMP_AUTHOR_NAME)" \
	GIT_COMMITTER_EMAIL="$(BUMP_AUTHOR_EMAIL)" \
	$(BUMP_VERSION) bump --config-file .bumpversion/$(ARTIFACT).toml $(PART)
	@tag="$$(git tag --points-at HEAD | grep -E '^(core|py|cs)-v' | head -1)"; \
	  test -n "$$tag" || { echo "error: bumpversion did not create a release tag"; exit 1; }; \
	  git push origin HEAD; \
	  git push origin "$$tag"

build-python-wheel: release
	@$(UV) build --wheel --no-build-isolation

build-csharp-nupkg: release
	dotnet pack csharp/Pppp.csproj -c Release -o artifacts

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

test-csharp: build
	dotnet test csharp/tests/Pppp.Tests.csproj -c Debug

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

# ClangSharp exits with the number of warnings. The visibility warnings from capi.h are expected.
generate-csharp:
	@ClangSharpPInvokeGenerator -f include/pppp/capi.h -I include -n Pppp -m Native \
	  -o csharp/Generated/Native.g.cs -l pppp_c -c codegen=compatible \
	  -was "*=internal" -wu "*=System.Runtime.CompilerServices" \
	  --resource-directory "$$(clang -print-resource-dir)" || test -s csharp/Generated/Native.g.cs

build-windows-2k build-windows-xp-x64:
	@echo "Legacy Windows cross-compilation is deferred to the xmake port; see docs/portability.md."
	@false
