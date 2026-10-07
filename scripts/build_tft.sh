#!/usr/bin/env bash
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
controller=ili9341
board=esp32s3
touch_flags=""
build_dir=""
while (($#)); do
  case "$1" in
    --board) board=${2:?Specify esp32s3 or guition}; shift 2 ;;
    --controller) controller=${2:?Specify ili9341 or st7789}; shift 2 ;;
    --no-touch) touch_flags="-DMBG_TFT_NO_TOUCH=1"; shift ;;
    --build-dir) build_dir=${2:?Specify a build directory}; shift 2 ;;
    *) printf 'Unknown option: %s\n' "$1" >&2; exit 2 ;;
  esac
done
case "$controller" in
  ili9341) controller_flags="" ;;
  st7789) controller_flags="-DMBG_TFT_ST7789=1" ;;
  *) printf 'Unsupported controller: %s\n' "$controller" >&2; exit 2 ;;
esac
case "$board" in
  esp32s3) fqbn=esp32:esp32:esp32s3; board_flags=""; build_name="tft-$controller" ;;
  guition) fqbn=esp32:esp32:esp32; board_flags="-DMBG_TFT_GUITION=1"; build_name="tft-guition-$controller" ;;
  *) printf 'Unsupported board: %s\n' "$board" >&2; exit 2 ;;
esac
build_dir=${build_dir:-"$project_dir/bin/$build_name"}
setup_file="$project_dir/firmware/esp32s3_mbg_display/mbg_tft_setup.h"
# TFT_eSPI.cpp must receive the same pins and driver as the sketch and UI engine.
arduino-cli compile --fqbn "$fqbn" --build-path "$build_dir" \
  --build-property "compiler.cpp.extra_flags=-DMBG_TFT_GUIDE_SETUP=1 -include \"$setup_file\" $board_flags $controller_flags $touch_flags" \
  "$project_dir/firmware/esp32s3_mbg_display"
