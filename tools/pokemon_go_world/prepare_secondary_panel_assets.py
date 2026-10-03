"""Prepare exact, nearest-neighbor sprite sheets for the SDL2 lower screen.

The original player-provided sheets stay in graphics/pc_panel/sources. BMP is
used because the Windows build loads SDL surfaces without SDL2_image.
"""

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
PANEL = ROOT / "graphics" / "pc_panel"
KEY = (255, 0, 255)


def save_colorkey(source: Path, target: Path, box=None, alpha=False):
    image = Image.open(source).convert("RGBA")
    if box is not None:
        image = image.crop(box)
    output = Image.new("RGB", image.size, KEY)
    pixels = image.load()
    result = output.load()
    for y in range(image.height):
        for x in range(image.width):
            red, green, blue, opacity = pixels[x, y]
            if alpha:
                if opacity >= 32:
                    result[x, y] = (255, 255, 255)
            elif opacity >= 32 and not (red >= 246 and green >= 246 and blue >= 246):
                result[x, y] = (red, green, blue)
    output.save(target, format="BMP")


def main():
    PANEL.mkdir(parents=True, exist_ok=True)
    save_colorkey(PANEL / "sources" / "platinum_menu_icons.png",
                  PANEL / "platinum_icons.bmp")
    # The green normal/highlight buttons, blue back arrows and touch swatches.
    save_colorkey(PANEL / "sources" / "platinum_battle_system.png",
                  PANEL / "platinum_buttons.bmp", (0, 1750, 589, 2620))
    # First eleven 29x36 cells rows: Latin letters, numbers and accents.
    save_colorkey(PANEL / "sources" / "oras_font.png",
                  PANEL / "oras_font.bmp", (0, 0, 464, 396), alpha=True)
    print("Prepared Platinum buttons/icons and ORAS glyphs for the PC panel.")


if __name__ == "__main__":
    main()
