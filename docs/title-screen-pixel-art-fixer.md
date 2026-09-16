# Pixel Art Fixer title-art workflow

Use [Retro Diffusion's Pixel Art Fixer](https://github.com/Retro-Diffusion/pixel-art-fixer)
to turn a title-art concept into grid-aligned source art. It is an external tool:
do not add its checkout or virtual environment to this repository.

## One-time setup

```bash
git clone https://github.com/Retro-Diffusion/pixel-art-fixer.git /tmp/pixel-art-fixer
python3 -m venv /tmp/pixelfixer-venv
/tmp/pixelfixer-venv/bin/pip install -r /tmp/pixel-art-fixer/python/requirements.txt
```

## Convert a new concept

The repository's current Python CLI has a stale import, so use its public Python
API directly. Substitute absolute paths for `INPUT.png` and `OUTPUT.png`.

```bash
PYTHONPATH=/tmp/pixel-art-fixer/python \
/tmp/pixelfixer-venv/bin/python -c '
from pathlib import Path
import numpy as np
from PIL import Image
from pixelfixer import detect
from pixelfixer.reconstruct import reconstruct

source = Path("INPUT.png")
target = Path("OUTPUT.png")
rgba = np.array(Image.open(source).convert("RGBA"))
result = detect(rgba)
print(result)
fixed = reconstruct(rgba, result["step_x"], result["step_y"], result["cols"], result["rows"])
Image.fromarray(fixed).save(target)
'
```

Review `OUTPUT.png` at native size before using it. Pixel Art Fixer reconstructs
the detected native pixel grid; it does not compose a GBA title screen.

For the approved version-name wordmark, use
`.github/assets/wayfarer-simple.png` as `INPUT.png` and
`game/graphics/title_screen/wayfarer/wayfarer_wordmark_pixel_fixed.png` as
`OUTPUT.png`. The reviewed reconstruction detected a 117x41 grid. Then run
`game/tools/generate_wayfarer_banner.py` to fit it into the 128x32 title sprite
and reduce it to transparent, white, and black palette entries.

## GBA integration

Use the [Wayfarer title-background pipeline](title-screen-pipeline.md) after
reviewing the fixed image. It generates the current 240x160, 8bpp text
background, shared RGB555 palette, logo remap, tile map, and native preview.
