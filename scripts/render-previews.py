#!/usr/bin/env python3
"""Generate labelled OLED preview fixtures using the tracked firmware font."""
from pathlib import Path
import argparse
import html
import math
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "docs/assets/oled-preview.svg"


def font():
    source = (ROOT / "plugins/panelDisplay/PanelDisplay.cpp").read_text()
    characters = re.search(r'const char glyphCharacters\[\] = "([^"]+)";', source).group(1)
    data = re.search(r"const uint8_t glyphs\[\]\[5\] = \{(.*?)\n\};", source, re.S).group(1)
    glyphs = [tuple(int(value, 16) for value in re.findall(r"0x[0-9a-fA-F]+", row))
              for row in re.findall(r"\{([^}]+)\}", data)]
    assert len(glyphs) == len(characters) and all(len(row) == 5 for row in glyphs)
    return dict(zip(characters, glyphs))


def build():
    glyphs = font()
    version_source = (ROOT / "plugins/panelFeedback/FirmwareVersion.h").read_text()
    version = re.search(r'onstep200pVersion="([^"]+)"', version_source).group(1)
    panels = [
        ("Boot", [(34, 40, "ONSTEP200P"), (25, 48, f"VERSION {version}"), (16, 56, "BIN ABCDEF012345")]),
        ("Mount", [(0, 14, "AZ   123.45"), (0, 24, "ALT  +45.00"), (0, 34, "TRACKING"), (0, 44, "MOUNT IDLE"), (0, 54, "ALIGN NOT VERIFIED")]),
        ("Network", [(0, 14, "HOME WIFI CONNECTED"), (0, 24, "192.0.2.10"), (0, 34, "TCP 9999"), (0, 44, "APP LX200 CONNECTED"), (0, 54, "DEMO OBSERVATORY")]),
        ("Observatory", [(0, 14, "TIME 12:34:56"), (0, 24, "DATE 09-26-26"), (0, 34, "LAT  +00.00"), (0, 44, "LNG  W00.00"), (0, 54, "ELEV 100 m")]),
        ("Calibration", [(0, 14, "FULL CIRCLES 2-3"), (0, 24, "X 200 1600 3400"), (0, 34, "Y 0 2000 4095"), (0, 44, "MOVE THROUGH CORNERS"), (0, 54, "NO CLICKS - 3 TURNS")]),
        ("System", [(0, 14, "LIVE WIFI TCP"), (0, 24, "VCC NO SENSOR"), (0, 34, "SOUND ON"), (0, 44, "L-UT +03:00"), (0, 54, "INIT OK")]),
    ]
    headers = ["", "MOUNT", "NETWORK", "OBSERVATORY", "JOY CAL", "SYSTEM"]
    svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="960" height="540" viewBox="0 0 960 540">',
           '<title>OnStep200P simulated OLED previews</title>',
           '<desc>Firmware glyphs with fictional fixture values. Not live hardware screenshots.</desc>',
           '<rect width="960" height="540" rx="20" fill="#0c1426"/>',
           '<text x="30" y="34" fill="#e4f6ff" font-family="sans-serif" font-size="20">Inside the 128 × 64 dashboard</text>']
    for index, (label, lines) in enumerate(panels):
        pixels = set()

        def pixel(x, y):
            if 0 <= x < 128 and 0 <= y < 64:
                pixels.add((x, y))

        def text(x, y, value):
            for char in value.upper():
                char = {"/": "-", "*": "."}.get(char, char)
                if x + 5 >= 128:
                    break
                for column, bits in enumerate(glyphs.get(char, glyphs["?"])):
                    for row in range(7):
                        if bits & (1 << row):
                            pixel(x + column, y + row)
                x += 6

        if index == 0:
            for i in range(180):
                a = i * 0.0349066
                pixel(int(64 + 29 * math.cos(a)), int(22 + 10 * math.sin(a) + 6 * math.cos(a)))
            for d in range(-7, 8):
                pixel(64+d, 22)
                pixel(64, 22+d)
        else:
            text(2, 0, headers[index])
            text(72, 0, "U")
            text(107, 0, "+")
            text(120, 0, str(index + 1))
            for x in range(128):
                pixel(x, 10)
            for x in range(5):
                for y in range(5):
                    if x in {0, 4} or y in {0, 4}:
                        pixel(83+x, 1+y)
        for x, y, value in lines:
            text(x, y, value)
        x0, y0 = 30 + (index % 3) * 310, 58 + (index // 3) * 218
        svg.append(f'<rect x="{x0-8}" y="{y0-8}" width="280" height="190" rx="12" fill="#15223a"/>')
        svg.append(f'<rect x="{x0}" y="{y0}" width="256" height="128" fill="#02070c"/>')
        for x, y in sorted(pixels):
            svg.append(f'<rect x="{x0+x*2}" y="{y0+y*2}" width="2" height="2" fill="#9aeafa"/>')
        svg.append(f'<text x="{x0}" y="{y0+158}" fill="#c6d9eb" font-family="sans-serif" font-size="15">{html.escape(label)}</text>')
    svg.append('<text x="30" y="512" fill="#95acc8" font-family="sans-serif" font-size="14">SIMULATED PREVIEWS · Fictional data · Monochrome SSD1306 · Generated from firmware glyphs</text>')
    svg.append('</svg>')
    return "\n".join(svg) + "\n"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    content = build()
    if args.check:
        if not OUTPUT.exists() or OUTPUT.read_text() != content:
            print("FAIL OLED previews are stale; run make previews", file=sys.stderr)
            return 1
        print("PASS generated OLED previews")
    else:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(content)
        print("Generated docs/assets/oled-preview.svg")
    return 0


if __name__ == "__main__":
    sys.exit(main())
