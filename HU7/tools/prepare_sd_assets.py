#!/usr/bin/env python3
"""Create LVGL 8 RGB565 binary assets beside the source images on the SD card."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from PIL import Image, ImageOps

LV_IMG_CF_TRUE_COLOR = 4
LV_IMG_CF_TRUE_COLOR_ALPHA = 5


def fit_image(source: Path, size: tuple[int, int], alpha: bool) -> Image.Image:
    mode = "RGBA" if alpha else "RGB"
    with Image.open(source) as image:
        return ImageOps.fit(
            image.convert(mode),
            size,
            method=Image.Resampling.LANCZOS,
            centering=(0.5, 0.5),
        )


def write_lvgl_bin(image: Image.Image, destination: Path, alpha: bool) -> None:
    width, height = image.size
    color_format = LV_IMG_CF_TRUE_COLOR_ALPHA if alpha else LV_IMG_CF_TRUE_COLOR
    header = color_format | (width << 10) | (height << 21)

    with destination.open("wb") as output:
        output.write(struct.pack("<I", header))
        for pixel in image.getdata():
            red, green, blue = pixel[:3]
            rgb565 = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
            output.write(struct.pack("<H", rgb565))
            if alpha:
                output.write(bytes((pixel[3],)))


def convert(root: Path, relative: str, size: tuple[int, int], alpha: bool) -> None:
    source = root / relative
    if not source.is_file():
        raise FileNotFoundError(source)
    destination = source.with_suffix(".bin")
    image = fit_image(source, size, alpha)
    write_lvgl_bin(image, destination, alpha)
    print(f"{source.name} -> {destination.name} ({size[0]}x{size[1]})")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sd_root", type=Path, help="HU_STORAGE drive root, for example G:\\")
    args = parser.parse_args()
    root = args.sd_root.resolve()

    jobs = (
        ("assets/ui/Home_setting.jpg", (215, 372), False),
        ("assets/ui/Home_Map.jpg", (215, 372), False),
        ("assets/ui/Home_Media.png", (215, 372), True),
        ("assets/map/NaverMap.png", (1024, 516), False),
        ("assets/media/No_Media.png", (50, 50), True),
    )
    for relative, size, alpha in jobs:
        convert(root, relative, size, alpha)


if __name__ == "__main__":
    main()
