#!/usr/bin/env bash
set -euo pipefail

if [[ ! -f CMakeLists.txt || ! -d scripts ]]; then
  echo "Run this script from the project root: bash scripts/build.sh" >&2
  exit 1
fi

if [[ "$(uname -s)" != "Darwin" || "$(uname -m)" != "arm64" ]]; then
  echo "Supported platform: Apple Silicon macOS arm64" >&2
  exit 1
fi

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is required. Install build dependencies with:" >&2
  echo "  brew install cmake gperf openssl@3" >&2
  exit 1
fi

missing_commands=()
for required_command in git cmake gperf; do
  if ! command -v "$required_command" >/dev/null 2>&1; then
    missing_commands+=("$required_command")
  fi
done

if (( ${#missing_commands[@]} > 0 )); then
  echo "Missing required commands: ${missing_commands[*]}" >&2
  echo "Install Xcode Command Line Tools and Homebrew dependencies:" >&2
  echo "  xcode-select --install" >&2
  echo "  brew install cmake gperf openssl@3" >&2
  exit 1
fi

openssl_root="$(brew --prefix openssl@3 2>/dev/null || true)"
if [[ -z "$openssl_root" ]]; then
  echo "openssl@3 is required. Install it with: brew install openssl@3" >&2
  exit 1
fi

jobs="$(sysctl -n hw.logicalcpu)"
td_src=".tdlib-src/td"
td_build=".tdlib-src/build-tdlib"
td_install="$PWD/vendor/tdlib"
app_build="build"

mkdir -p .local/{home,tmp,cmake} .tdlib-src
export HOME="$PWD/.local/home" TMPDIR="$PWD/.local/tmp" CMAKE_CONFIG_DIR="$PWD/.local/cmake"

if [[ ! -d "$td_src/.git" ]]; then
  git clone --depth=1 https://github.com/tdlib/td.git "$td_src"
fi

cmake_common=(
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_OSX_ARCHITECTURES=arm64
  -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON
  -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
  -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
)

cmake -S "$td_src" -B "$td_build" "${cmake_common[@]}" \
  -DCMAKE_INSTALL_PREFIX="$td_install" \
  -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON \
  -DBUILD_TESTING=OFF \
  -DTD_ENABLE_LTO=OFF \
  -DTD_INSTALL_SHARED_LIBRARIES=OFF \
  -DTD_INSTALL_STATIC_LIBRARIES=ON \
  -DOPENSSL_ROOT_DIR="$openssl_root"
cmake --build "$td_build" --target tdclient tdjson_static -j"$jobs"
cmake --build "$td_build" --target install -j"$jobs"

cmake -S . -B "$app_build" "${cmake_common[@]}" \
  -DCMAKE_PREFIX_PATH="$td_install" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$app_build" -j"$jobs"

echo "Built: $app_build/tg-tools"
