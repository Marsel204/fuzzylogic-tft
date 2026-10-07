#!/usr/bin/env python3
"""Render real UI draw calls with the installed TFT_eSPI bitmap fonts."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import tempfile
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]


def numbers(text):
    text = re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.S)
    return [int(value, 0) for value in re.findall(r"0x[0-9A-Fa-f]+|\b\d+\b", text)]


def load_font(path, prefix):
    text = path.read_text()
    # Select TFT_eSPI's default degree-symbol/dollar branches.
    text = re.sub(r"#ifdef[^\n]*\n(.*?)#else\n.*?#endif", r"\1", text, flags=re.S)
    widths = numbers(re.search(r"widtbl_\w+\[96\].*?\{(.*?)\};", text, re.S)[1])
    glyphs = {int(code, 16): numbers(data) for code, data in re.findall(r"chr_" + prefix + r"_([0-9a-fA-F]+)\[[^\]]*\].*?\{(.*?)\};", text, re.S)}
    return widths, glyphs


def color(value):
    return ((value >> 11) * 255 // 31, ((value >> 5) & 63) * 255 // 63, (value & 31) * 255 // 31)


def text_pixels(image, command, fonts):
    x, y, font = command["x"], command["y"], command["font"]
    foreground, background = color(command["color"]), color(command["background"])
    for char in command["text"]:
        code = ord(char)
        if font == 1:
            width, height = 6, 8
            rows = fonts[1][code * 5: code * 5 + 5]
            mask = [[col < 5 and bool(rows[col] & (1 << row)) for col in range(width)] for row in range(height)]
        else:
            widths, glyphs = fonts[font]
            width = widths[code - 32]
            height = 16 if font == 2 else 26
            data = glyphs[code]
            if font == 2:
                stride = (width + 6) // 8
                mask = [[col < stride * 8 and bool(data[row * stride + col // 8] & (0x80 >> (col % 8))) for col in range(width)] for row in range(height)]
            else:
                pixels = []
                for byte in data:
                    pixels.extend([bool(byte & 0x80)] * ((byte & 0x7f) + 1))
                mask = [pixels[row * width:(row + 1) * width] for row in range(height)]
        for row, bits in enumerate(mask):
            for col, active in enumerate(bits):
                if 0 <= x + col < 320 and 0 <= y + row < 240:
                    image.putpixel((x + col, y + row), foreground if active else background)
        x += width


def render(commands, fonts):
    image = Image.new("RGB", (320, 240), "white")
    painter = ImageDraw.Draw(image)
    for c in commands:
        x, y, w, h, r, shade = c["x"], c["y"], c["w"], c["h"], c["r"], color(c["color"])
        if c["kind"] == "text": text_pixels(image, c, fonts)
        elif c["kind"] == "line": painter.line((x, y, w, h), fill=shade)
        elif c["kind"] == "circle": painter.ellipse((x-r,y-r,x+r,y+r), outline=shade)
        elif w > 0 and h > 0:
            box = (x, y, x+w-1, y+h-1)
            if c["kind"] == "outline": painter.rounded_rectangle(box, radius=r, outline=shade)
            elif c["kind"] == "round": painter.rounded_rectangle(box, radius=r, fill=shade)
            else: painter.rectangle(box, fill=shade)
    return image


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--tft-library", type=Path, default=Path.home() / "Arduino/libraries/TFT_eSPI")
    parser.add_argument("--output", type=Path, default=ROOT / "docs/tft-ui-preview.png")
    args = parser.parse_args()
    fonts = {2: load_font(args.tft_library / "Fonts/Font16.c", "f16"),
             4: load_font(args.tft_library / "Fonts/Font32rle.c", "f32")}
    glcd = (args.tft_library / "Fonts/glcdfont.c").read_text()
    fonts[1] = numbers(re.search(r"font\[\].*?\{(.*?)\};", glcd, re.S)[1])
    screens = ["menu", "probe", "milk", "fresh", "reject", "unknown"]
    titles = ["01 / Pilih bahan", "02 / Sensor jaringan", "03 / Sensor susu", "04 / Hasil segar", "05 / Hasil tolak", "06 / Ulangi pengukuran"]
    output = Image.new("RGB", (1040, 592), "#e7ebef")
    label_font = ImageFont.load_default(size=14)
    with tempfile.TemporaryDirectory(prefix="mbg-ui-preview-") as folder:
        build = Path(folder)
        objects = []
        for name in ("mbg_fuzzy_core", "mbg_profiles", "mbg_mock_sensors"):
            obj = build / (name + ".o")
            subprocess.run(["cc", "-std=c99", "-c", str(ROOT / "firmware/esp32s3_mbg_display" / (name + ".c")), "-o", str(obj)], check=True)
            objects.append(str(obj))
        executable = build / "preview"
        subprocess.run(["g++", "-std=gnu++17", "-DCONFIG_IDF_TARGET_ESP32S3", "-DMBG_TFT_GUIDE_SETUP=1",
                        "-I" + str(ROOT / "tests/firmware_stubs"), str(ROOT / "tests/firmware_serial_harness.cpp"),
                        str(ROOT / "firmware/esp32s3_mbg_display/mbg_ui_engine.cpp"), *objects, "-lm", "-o", str(executable)], check=True)
        for index, screen in enumerate(screens):
            text = subprocess.check_output([str(executable), "--screen", screen], text=True)
            commands = [json.loads(line[6:]) for line in text.splitlines() if line.startswith("DRAW: ")]
            frame = render(commands, fonts)
            x, y = 24 + (index % 3) * 336, 42 + (index // 3) * 284
            output.paste(frame, (x, y))
            ImageDraw.Draw(output).text((x, y-26), titles[index], font=label_font, fill="#293341")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    output.save(args.output)
    print(args.output.resolve())


if __name__ == "__main__":
    main()
