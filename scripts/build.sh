#!/usr/bin/env bash
set -euo pipefail

if [[ ! -f CMakeLists.txt || ! -d scripts ]]; then
  echo "Run this script from the project root: bash scripts/build.sh" >&2
  exit 1
fi

if [[ "$(uname -s)" != "Darwin" || "$(uname -m)" != "arm64" ]]; then
  echo "Supported platform: Apple Silicon macOS" >&2
  exit 1
fi

for required_command in git cmake gperf brew; do
  command -v "$required_command" >/dev/null 2>&1 || {
    echo "Missing $required_command. Install dependencies from README.md first." >&2
    exit 1
  }
done

openssl_root="$(brew --prefix openssl@3)"

jobs="$(sysctl -n hw.logicalcpu)"
td_src=".tdlib-src/td"
td_build=".tdlib-src/build-tdlib"
td_install="$PWD/vendor/tdlib"
app_build="build"

mkdir -p .local/{home,tmp,cmake} .tdlib-src
export HOME="$PWD/.local/home" TMPDIR="$PWD/.local/tmp" CMAKE_CONFIG_DIR="$PWD/.local/cmake"

if [[ ! -d "$td_src/.git" ]]; then
  git clone https://github.com/tdlib/td.git "$td_src"
fi

cmake_common=(
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
  -DCMAKE_OSX_ARCHITECTURES=arm64
  -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON
  -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
  -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
)

cmake -S "$td_src" -B "$td_build" "${cmake_common[@]}" \
  -DCMAKE_INSTALL_PREFIX="$td_install" \
  -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON \
  -DBUILD_TESTING=OFF \
  -DTD_ENABLE_LTO=ON \
  -DTD_INSTALL_SHARED_LIBRARIES=OFF \
  -DTD_INSTALL_STATIC_LIBRARIES=ON \
  -DOPENSSL_ROOT_DIR="$openssl_root"
cmake --build "$td_build" --target tdclient tdjson_static -j"$jobs"
cmake --build "$td_build" --target install -j"$jobs"

cmake -S . -B "$app_build" "${cmake_common[@]}" \
  -DCMAKE_PREFIX_PATH="$td_install" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$app_build" -j"$jobs"
strip "$app_build/tg-tools"

echo "Built: $app_build/tg-tools"
