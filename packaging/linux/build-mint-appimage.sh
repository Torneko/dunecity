#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${BUILD_DIR:-${repo_root}/build-linux-mint}"
app_dir="${build_dir}/AppDir"
output_dir="${OUTPUT_DIR:-${build_dir}}"
cmake_bin="${CMAKE_BIN:-cmake}"
vcpkg_root="${VCPKG_ROOT:-/opt/vcpkg}"

if [[ -r /etc/os-release ]]; then
    # shellcheck disable=SC1091
    source /etc/os-release
    base="${UBUNTU_CODENAME:-${VERSION_CODENAME:-unknown}}"
    if [[ "${base}" != "jammy" ]]; then
        echo "Warning: this portable build is intended to be produced on Ubuntu 22.04/jammy or Linux Mint 21.x." >&2
    fi
fi

for command_name in "${cmake_bin}" ninja wget sha256sum; do
    command -v "${command_name}" >/dev/null 2>&1 || { echo "Missing command: ${command_name}" >&2; exit 2; }
done

if [[ ! -f "${vcpkg_root}/scripts/buildsystems/vcpkg.cmake" ]]; then
    echo "Set VCPKG_ROOT to a vcpkg checkout containing scripts/buildsystems/vcpkg.cmake." >&2
    exit 3
fi
if [[ -e "${app_dir}" ]]; then
    echo "AppDir already exists: ${app_dir}. Remove it before rebuilding." >&2
    exit 4
fi

"${cmake_bin}" -S "${repo_root}" -B "${build_dir}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DDUNELEGACY_BUILD_TESTS=OFF \
    -DCMAKE_TOOLCHAIN_FILE="${vcpkg_root}/scripts/buildsystems/vcpkg.cmake"
"${cmake_bin}" --build "${build_dir}" --parallel "${BUILD_JOBS:-2}"
"${cmake_bin}" --install "${build_dir}" --prefix "${app_dir}/usr" --config Release

linuxdeploy="${build_dir}/linuxdeploy-x86_64.AppImage"
if [[ ! -x "${linuxdeploy}" ]]; then
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O "${linuxdeploy}"
    chmod +x "${linuxdeploy}"
fi

desktop_file="${app_dir}/usr/share/applications/dunelegacy-tornie.desktop"
icon_file="${app_dir}/usr/share/icons/hicolor/128x128/apps/dunelegacy-tornie.png"
[[ -f "${desktop_file}" && -f "${icon_file}" ]] || { echo "Installed AppImage metadata is incomplete." >&2; exit 5; }

mkdir -p "${output_dir}"
cd "${build_dir}"
ARCH=x86_64 FUSE_APPIMAGE_EXTRACT_AND_RUN=1 \
    "${linuxdeploy}" \
    --appdir "${app_dir}" \
    --executable "${app_dir}/usr/bin/dunelegacy" \
    --desktop-file "${desktop_file}" \
    --icon-file "${icon_file}" \
    --output appimage

version="$(sed -n 's/^project(DuneLegacyTornie VERSION \([0-9][0-9.]*\).*/\1/p' "${repo_root}/CMakeLists.txt")"
generated="$(find "${build_dir}" -maxdepth 1 -type f -name '*.AppImage' ! -name 'linuxdeploy-*.AppImage' -print -quit)"
[[ -n "${generated}" ]] || { echo "linuxdeploy did not produce an AppImage." >&2; exit 6; }
output="${output_dir}/DuneLegacyTornie-${version}-Linux-Mint-x86_64.AppImage"
mv "${generated}" "${output}"
sha256sum "${output}" > "${output}.sha256"
echo "APPIMAGE=${output}"
echo "SHA256=${output}.sha256"