#!/usr/bin/env bash
set -euo pipefail

parallel_cpus() {
  case "$(uname)" in
    Darwin)  sysctl -n hw.logicalcpu ;;
    Linux)   nproc ;;
    *)       echo 2 ;;
  esac
}

if [[ ! -f CMakeLists.txt || ! -d scripts ]]; then
  echo "Run this script from the project root: bash scripts/build.sh" >&2
  exit 1
fi

os="$(uname -s)"
arch="$(uname -m)"
if [[ !("$os" == "Linux" && "$arch" == "x86_64") && !("$os" == "Darwin" && "$arch" == "arm64") ]]; then
  echo "Supported platforms: Linux x86_64, macOS arm64" >&2
  echo "Current platform: $os $arch" >&2
  exit 1
fi

td_source_dir=".tdlib-src/td"
td_build_dir=".tdlib-src/build-tdlib"
td_install_dir="$PWD/vendor/tdlib"
app_build_dir="build"
local_state_dir=".local"

mkdir -p "$local_state_dir/home" "$local_state_dir/tmp" "$local_state_dir/cmake"
export HOME="$PWD/$local_state_dir/home"
export TMPDIR="$PWD/$local_state_dir/tmp"
export CMAKE_CONFIG_DIR="$PWD/$local_state_dir/cmake"

if [[ ! -d "$td_source_dir/.git" ]]; then
  mkdir -p "$(dirname "$td_source_dir")"
  git clone --depth=1 https://github.com/tdlib/td.git "$td_source_dir"
fi

cmake_args=(
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX="$td_install_dir"
  -DTD_ENABLE_LTO=OFF
  -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON
  -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
  -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
)

if [[ "$os" == "Darwin" ]] && command -v brew >/dev/null 2>&1; then
  openssl_root="$(brew --prefix openssl@3 2>/dev/null || true)"
  if [[ -n "$openssl_root" ]]; then
    cmake_args+=(-DOPENSSL_ROOT_DIR="$openssl_root")
  fi
fi

cmake -S "$td_source_dir" -B "$td_build_dir" "${cmake_args[@]}"
cmake --build "$td_build_dir" --target install -j$(parallel_cpus)

cmake -S . -B "$app_build_dir" \
  -DCMAKE_PREFIX_PATH="$td_install_dir" \
  -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON \
  -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF \
  -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
cmake --build "$app_build_dir" -j$(parallel_cpus)

echo "Built: $app_build_dir/tg-video-cli"
