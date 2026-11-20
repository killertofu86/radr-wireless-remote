#!/usr/bin/env python3
"""Convert an image to an RGB565 header and select it for the next build.

Usage:
    python src/helpers/image_converter.py /path/to/image.png
"""

import argparse
import re
from pathlib import Path

import numpy as np
from PIL import Image

MAX_WIDTH = 320
MAX_HEIGHT = 240


def identifier(name: str) -> str:
    value = re.sub(r"[^A-Za-z0-9_]", "_", name).upper()
    if not value or value[0].isdigit():
        value = f"IMAGE_{value}"
    return value


def write_image_header(image_path: Path, output_path: Path) -> tuple[str, int, int]:
    image = Image.open(image_path).convert("RGB")
    image.thumbnail((MAX_WIDTH, MAX_HEIGHT), Image.LANCZOS)
    width, height = image.size

    pixels = np.array(image)
    rgb565 = (
        ((pixels[:, :, 0].astype(np.uint16) >> 3) << 11)
        | ((pixels[:, :, 1].astype(np.uint16) >> 2) << 5)
        | (pixels[:, :, 2].astype(np.uint16) >> 3)
    )

    symbol = identifier(output_path.stem)
    guard = f"IMAGES_{symbol}_H"

    with output_path.open("w") as header:
        header.write(f"#ifndef {guard}\n#define {guard}\n\n")
        header.write("#include <Arduino.h>\n\n")
        header.write(f"const uint16_t {symbol}_WIDTH = {width};\n")
        header.write(f"const uint16_t {symbol}_HEIGHT = {height};\n")
        header.write(f"const uint16_t {symbol}[] PROGMEM = {{\n")

        for index, value in enumerate(rgb565.ravel(), start=1):
            header.write(f"0x{value:04x}")
            header.write(",\n" if index % 16 == 0 else ",")

        header.write("\n};\n\n#endif\n")

    return symbol, width, height


def write_current_build(path: Path, header_name: str, symbol: str) -> None:
    path.write_text(
        "#pragma once\n\n"
        f'#include "{header_name}"\n\n'
        f"#define CURRENT_BUILD_IMAGE {symbol}\n"
        f"#define CURRENT_BUILD_WIDTH {symbol}_WIDTH\n"
        f"#define CURRENT_BUILD_HEIGHT {symbol}_HEIGHT\n"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Create an RGB565 display header and select it for the next firmware build."
    )
    parser.add_argument("image", type=Path, help="Input image (PNG, JPG, …)")
    args = parser.parse_args()

    image_path = args.image.expanduser().resolve()
    if not image_path.is_file():
        parser.error(f"Image not found: {image_path}")

    images_dir = Path(__file__).resolve().parents[1] / "images"
    output_path = images_dir / f"{image_path.stem}.h"

    symbol, width, height = write_image_header(image_path, output_path)
    write_current_build(images_dir / "current_build.h", output_path.name, symbol)

    print(f"Created: {output_path}")
    print(f"Selected for build: {images_dir / 'current_build.h'}")
    print(f"Size: {width}x{height}px")
    print("Next step: pio run -e development -t upload")


if __name__ == "__main__":
    main()
