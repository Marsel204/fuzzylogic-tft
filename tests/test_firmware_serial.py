"""Host regressions for the display sketch's serial inspection lifecycle."""
import json
from pathlib import Path
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[1]
DISPLAY = ROOT / "firmware" / "esp32s3_mbg_display"


@pytest.fixture(scope="module", params=["esp32s3", "guition"])
def firmware_harness(tmp_path_factory, request):
    build = tmp_path_factory.mktemp("firmware-host")
    objects = []
    for name in ("mbg_fuzzy_core", "mbg_profiles", "mbg_mock_sensors"):
        obj = build / f"{name}.o"
        subprocess.run(["cc", "-std=c99", "-c", str(DISPLAY / f"{name}.c"), "-o", str(obj)], check=True, capture_output=True)
        objects.append(str(obj))
    executable = build / "serial_harness"
    subprocess.run([
        "g++", "-std=gnu++17", "-fsanitize=undefined,float-cast-overflow",
        "-DCONFIG_IDF_TARGET_ESP32S3" if request.param == "esp32s3" else "-DMBG_TFT_GUITION=1",
        "-DMBG_TFT_GUIDE_SETUP=1", "-I" + str(ROOT / "tests" / "firmware_stubs"),
        str(ROOT / "tests" / "firmware_serial_harness.cpp"), str(DISPLAY / "mbg_ui_engine.cpp"),
        *objects, "-lm", "-o", str(executable)
    ], check=True, capture_output=True)
    return executable


def run_serial(harness, commands, *args):
    process = subprocess.run([str(harness), commands, *args], check=True, capture_output=True, text=True)
    assert not process.stderr  # Includes undefined/NaN-to-integer conversion diagnostics.
    receipts = [json.loads(line.removeprefix("JSON: ")) for line in process.stdout.splitlines() if line.startswith("JSON: ")]
    return process.stdout, receipts


@pytest.mark.parametrize("commands", ["e", "a", "1re"])
def test_commands_require_selection_and_a_completed_inspection(firmware_harness, commands):
    output, receipts = run_serial(firmware_harness, commands)
    assert receipts == []
    assert "CHEMICALLY SAFE" not in output


def test_evaluation_resamples_after_scenario_change(firmware_harness):
    _, receipts = run_serial(firmware_harness, "1ese")
    assert len(receipts) == 2
    assert receipts[0]["score"] >= 65
    assert receipts[1]["score"] <= 35
    assert receipts[1]["verdict"] == "TIDAK MEMENUHI SYARAT"


def test_audit_reprints_the_completed_inspection_snapshot(firmware_harness):
    output, receipts = run_serial(firmware_harness, "1se7fa")
    assert len(receipts) == 2
    assert receipts[0] == receipts[1]
    last_receipt = output.split("AUDIT RECEIPT INSPEKSI CCP-1")[-1]
    assert "Daging Ayam Broiler" in last_receipt
    assert "Sensor Probe : R_eis=" in last_receipt
    assert "Sensor Probe : pH=" not in last_receipt


def test_indeterminate_verdict_renders_without_nan_casts_and_exports_null(firmware_harness):
    output, receipts = run_serial(firmware_harness, "7", "indeterminate")
    assert receipts[-1]["score"] is None
    assert "BELUM DAPAT DITENTUKAN" in receipts[-1]["verdict"]
    assert "Skor Mutu    : --" in output


def test_shared_inference_code_is_identical_in_both_sketches():
    for file in ("mbg_fuzzy_core.c", "mbg_fuzzy_core.h", "mbg_profiles.c", "mbg_profiles.h"):
        shared = (ROOT / "firmware" / "src" / file).read_bytes()
        for sketch in ("esp32s3_mbg_display", "esp32s3_mbg_inspector"):
            assert (ROOT / "firmware" / sketch / file).read_bytes() == shared


@pytest.mark.parametrize("mode", ["--touch-flow", "--hold-touch", "--calibration", "--redraw", "--cst-input"])
def test_touch_inspection_lifecycle(firmware_harness, mode):
    output, _ = run_serial(firmware_harness, mode)
    assert "PASSED" in output


@pytest.mark.parametrize("screen", ["boot", "menu", "probe", "milk", "fresh", "reject", "unknown"])
def test_screen_text_fits_320_by_240_and_has_no_text_collisions(firmware_harness, screen):
    output, _ = run_serial(firmware_harness, "--screen", screen)
    draws = [json.loads(line.removeprefix("DRAW: ")) for line in output.splitlines() if line.startswith("DRAW: ")]
    assert draws
    text = [draw for draw in draws if draw["kind"] == "text"]
    for draw in text:
        assert 0 <= draw["x"] and draw["x"] + draw["w"] <= 320, draw
        assert 0 <= draw["y"] and draw["y"] + draw["h"] <= 240, draw
    for index, first in enumerate(text):
        for second in text[index + 1:]:
            intersects = (first["x"] < second["x"] + second["w"] and second["x"] < first["x"] + first["w"] and
                          first["y"] < second["y"] + second["h"] and second["y"] < first["y"] + first["h"])
            assert not intersects, (first, second)
