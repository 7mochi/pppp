BUILD_DIR   := build
NPROC := $(shell nproc 2>/dev/null || echo 4)

.PHONY: all clean rebuild test configure format format-check check-tidy

all: build

# The Python target joins the dev configuration when its headers exist, so clangd
# (which reads build/compile_commands.json) and clang-tidy see Python.h for the bindings.
PYTHON_FLAG := $(shell python3 -c "import os, sysconfig; raise SystemExit(0 if os.path.exists(os.path.join(sysconfig.get_paths()['include'], 'Python.h')) else 1)" 2>/dev/null && echo -DPPPP_BUILD_PYTHON=ON)

configure:
	@echo "Configuring pppp:"
	cmake -S . -B $(BUILD_DIR) \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		$(PYTHON_FLAG)

build: configure
	@echo "Building pppp:"
	cmake --build $(BUILD_DIR) -j$(NPROC)

test: build
	@echo "Running tests:"
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build test

FORMAT_SOURCES := $(shell git ls-files --cached --others --exclude-standard '*.cpp' '*.h' | \
	while IFS= read -r f; do [ -e "$$f" ] && printf '%s\n' "$$f"; done)

format:
	clang-format -i $(FORMAT_SOURCES)

format-check:
	clang-format --dry-run --Werror $(FORMAT_SOURCES)

check-tidy: configure
	@find src include tests -name '*.cpp' | sort | xargs clang-tidy -p $(BUILD_DIR) --quiet \
		> $(BUILD_DIR)/clang-tidy.log 2>&1 || true; \
		grep -E ': (warning|error):' $(BUILD_DIR)/clang-tidy.log | grep -vE '^/.*/third_party/' || true
