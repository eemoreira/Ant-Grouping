#!/usr/bin/env python3
# plot_steps.py
# usage:
#   python3 plot_steps.py           -> mostra o último frame
#   python3 plot_steps.py --anim     -> cria animation.mp4 (requires ffmpeg for saving mp4)
#   python3 plot_steps.py --dir path --anim --fps 6

import os, re, glob, argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# matches step_123.pgm or step_123.ppm
PGM_OR_PPM_RE = re.compile(r"step_(\d+)\.(pgm|ppm)$")

def read_non_comment_line(f):
    line = f.readline()
    while line.startswith(b'#'):
        line = f.readline()
    return line

def read_p5_pgm(path):
    with open(path, "rb") as f:
        magic = f.readline().strip()
        if magic != b'P5':
            raise ValueError("Not a P5 PGM")
        # read width,height and maxval robustly
        line = read_non_comment_line(f)
        while line.strip() == b'':
            line = read_non_comment_line(f)
        parts = line.split()
        if len(parts) == 2:
            width, height = map(int, parts)
            maxval = int(read_non_comment_line(f))
        else:
            width = int(parts[0])
            height = int(read_non_comment_line(f).split()[0])
            maxval = int(read_non_comment_line(f))
        if maxval > 255:
            raise ValueError("Maxval > 255 not supported")
        data = f.read(width * height)
        arr = np.frombuffer(data, dtype=np.uint8).reshape((height, width))
        return arr

def read_p6_ppm(path):
    with open(path, "rb") as f:
        magic = f.readline().strip()
        if magic != b'P6':
            raise ValueError("Not a P6 PPM")
        line = read_non_comment_line(f)
        while line.strip() == b'':
            line = read_non_comment_line(f)
        parts = line.split()
        if len(parts) == 2:
            width, height = map(int, parts)
            maxval = int(read_non_comment_line(f))
        else:
            width = int(parts[0])
            height = int(read_non_comment_line(f).split()[0])
            maxval = int(read_non_comment_line(f))
        if maxval > 255:
            raise ValueError("Maxval > 255 not supported")
        data = f.read(width * height * 3)
        arr = np.frombuffer(data, dtype=np.uint8).reshape((height, width, 3))
        return arr

# Palette used by the writer (keeps same visual language from PPM writer earlier)
PALETTE = np.array([
    [6,   6,   6],   # 0 ant  -> quase preto
    [239, 71, 111], # 1 data1 -> rosa/vermelho
    [255,209,102],  # 2 data2 -> amarelo quente
    [6, 214,160],   # 3 data3 -> verde água
    [17,138,178],   # 4 data4 -> azul petróleo
    [245,245,245],  # 5 empty -> cinza claro
], dtype=np.uint8)

# color used for carrier if we want to force highlight when PGM uses an index for carrying
CARRIER_COLOR = np.array([255, 0, 0], dtype=np.uint8)  # vermelho vivo

def colorize(arr):
    """
    If arr has 3 channels (H,W,3) assume it's already RGB (from P6), return as-is.
    If arr is grayscale (H,W), map values to palette by nearest known reference values.
    """
    if arr.ndim == 3 and arr.shape[2] == 3:
        return arr.copy()

    # grayscale -> map to palette
    h, w = arr.shape
    out = np.ones((h, w, 3), dtype=np.uint8) * 255

    # Reference grayscale keys we expect (common variants)
    # These are typical values used historically in the project; mapping will pick nearest.
    keys = np.array([0, 40, 110, 180, 220, 255], dtype=np.int32)
    palette = PALETTE  # same ordering: ant, data1, data2, data3, data4, empty

    # Build a lookup table for 0..255 to map each grayscale to nearest key index
    lut = np.zeros((256, 3), dtype=np.uint8)
    vals = np.arange(256)
    # compute nearest key index for each possible grayscale value
    nearest_idx = np.argmin(np.abs(vals[:, None].astype(np.int32) - keys[None, :]), axis=1)
    for v in range(256):
        idx = nearest_idx[v]
        lut[v] = palette[idx]

    # apply LUT
    out = lut[arr]

    # heuristics: if there's a grayscale value very close to 180 (carrier) but not exact,
    # we might want to set it explicitly to carrier color — detect presence of values near 180
    # and if found set them to CARRIER_COLOR
    # This is optional but helps when old PGM used 180 for carry.
    carrier_candidates = np.where(np.abs(arr.astype(np.int32) - 180) <= 3)
    if carrier_candidates[0].size > 0:
        out[carrier_candidates] = CARRIER_COLOR

    return out

def find_steps(directory):
    files = glob.glob(os.path.join(directory, "step_*.*"))
    data = []
    for p in files:
        m = PGM_OR_PPM_RE.search(os.path.basename(p))
        if m:
            data.append((int(m.group(1)), p))
    data.sort()
    return data

def load_image(path):
    # auto-detect P5 or P6
    with open(path, "rb") as f:
        magic = f.readline().strip()
    if magic == b'P6':
        return read_p6_ppm(path)
    elif magic == b'P5':
        gray = read_p5_pgm(path)
        return colorize(gray)
    else:
        raise ValueError(f"Unsupported image format in {path}: {magic}")

def show_last_frame(directory):
    steps = find_steps(directory)
    if not steps:
        print("No step_*.pgm/.ppm files found in", directory)
        return
    _, last = steps[-1]
    img = load_image(last)
    plt.figure(figsize=(6,6))
    plt.imshow(img, interpolation='nearest')
    plt.title(os.path.basename(last))
    plt.axis('off')
    plt.show()

def make_animation(directory, out_file, fps=1):
    steps = find_steps(directory)
    if not steps:
        print("No step_*.pgm/.ppm files found in", directory)
        return
    imgs = []
    for n, path in steps:
        img = load_image(path)
        imgs.append(img)

    fig = plt.figure(figsize=(6,6))
    im = plt.imshow(imgs[0], interpolation='nearest')
    plt.axis('off')

    def update(i):
        im.set_data(imgs[i])
        return (im,)

    ani = animation.FuncAnimation(fig, update, frames=len(imgs), interval=1000/fps, blit=True)
    # save
    print("Saving animation to", out_file, "...")
    Writer = animation.writers['ffmpeg'] if 'ffmpeg' in animation.writers.list() else None
    if Writer:
        writer = Writer(fps=fps, metadata=dict(artist='ant-sim'), bitrate=1800)
        ani.save(out_file, writer=writer)
        print("Saved.")
    else:
        # fallback: save as gif (may be slower)
        gif_out = os.path.splitext(out_file)[0] + ".gif"
        ani.save(gif_out, writer='imagemagick', fps=fps)
        print("Saved gif to", gif_out)

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", default="res", help="directory with step_*.pgm or step_*.ppm")
    ap.add_argument("--anim", action="store_true", help="create animation file instead of showing last frame")
    ap.add_argument("--out", default="res/animation.mp4", help="output filename for animation")
    ap.add_argument("--fps", type=int, default=1, help="frames per second for animation")
    args = ap.parse_args()

    if args.anim:
        make_animation(args.dir, out_file=args.out, fps=args.fps)
    else:
        show_last_frame(args.dir)

