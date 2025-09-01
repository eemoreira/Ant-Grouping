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

PGM_RE = re.compile(r"step_(\d+)\.pgm$")

def read_p5_pgm(path):
    with open(path, "rb") as f:
        magic = f.readline().strip()
        if magic != b'P5':
            raise ValueError("Only binary P5 PGM supported")
        # skip comments
        def read_non_comment_line():
            line = f.readline()
            while line.startswith(b'#'):
                line = f.readline()
            return line
        line = read_non_comment_line()
        while line.strip() == b'':
            line = read_non_comment_line()
        # line contains width height or maybe width only; handle robustly
        parts = line.split()
        if len(parts) == 2:
            width, height = map(int, parts)
            maxval = int(read_non_comment_line())
        else:
            # width on this line, height on next
            width = int(parts[0])
            height = int(read_non_comment_line().split()[0])
            maxval = int(read_non_comment_line())
        if maxval > 255:
            raise ValueError("Maxval > 255 not supported in simple reader")
        data = f.read(width * height)
        arr = np.frombuffer(data, dtype=np.uint8).reshape((height, width))
        return arr

def colorize(arr):
    # mapping according to writer:
    # 0 -> both (black)
    # 80 -> item only (blue-ish)
    # 220 -> ant only (red-ish)
    # 255 -> empty (white)
    h, w = arr.shape
    out = np.ones((h, w, 3), dtype=np.uint8) * 255  # default white
    mask_both = (arr == 0)
    mask_item = (arr == 80)
    mask_ant  = (arr == 220)
    mask_carry = (arr == 180)
    out[mask_both] = np.array([0, 0, 0], dtype=np.uint8)
    out[mask_item] = np.array([40, 100, 200], dtype=np.uint8)   # blue
    out[mask_ant]  = np.array([220, 40, 40], dtype=np.uint8)    # red
    out[mask_carry] = np.array([255, 0, 255], dtype=np.uint8)   # magenta
    return out

def find_steps(directory):
    files = glob.glob(os.path.join(directory, "step_*.pgm"))
    data = []
    for p in files:
        m = PGM_RE.search(os.path.basename(p))
        if m:
            data.append((int(m.group(1)), p))
    data.sort()
    return data

def show_last_frame(directory):
    steps = find_steps(directory)
    if not steps:
        print("No step_*.pgm files found in", directory)
        return
    _, last = steps[-1]
    arr = read_p5_pgm(last)
    col = colorize(arr)
    plt.figure(figsize=(6,6))
    plt.imshow(col, interpolation='nearest')
    plt.title(os.path.basename(last))
    plt.axis('off')
    plt.show()

def make_animation(directory, out_file, fps=1):
    steps = find_steps(directory)
    if not steps:
        print("No step_*.pgm files found in", directory)
        return
    imgs = []
    for n, path in steps:
        arr = read_p5_pgm(path)
        imgs.append(colorize(arr))

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
    ap.add_argument("--dir", default="res", help="directory with step_*.pgm")
    ap.add_argument("--anim", action="store_true", help="create animation file instead of showing last frame")
    ap.add_argument("--out", default="res/animation.mp4", help="output filename for animation")
    ap.add_argument("--fps", type=int, default=1, help="frames per second for animation")
    args = ap.parse_args()

    print(args.out)
    if args.anim:
        make_animation(args.dir, out_file=args.out, fps=args.fps)
    else:
        show_last_frame(args.dir)

