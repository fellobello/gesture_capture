# gesture_capture

A webcam vision pipeline in C++23, written from the camera driver up. Frames come straight
from the Linux V4L2 API, and every image operation (color conversion, blur, skin segmentation,
thresholding, connected-component grouping) is hand-written. OpenCV is used only to show the
window and draw text.

**Status: work in progress.** Capture and the image pipeline work on a live 640×480 feed.
Hand detection and gesture recognition are next (see [Roadmap](#roadmap)).

## How it works

```
/dev/video0 ──V4L2 (ioctl + mmap)──> YUYV frame
   └─> RGB (fixed-point BT.601)
         ├─> box blur (separable)
         ├─> grayscale ─> threshold ─> flood fill ─> bounding boxes ─> drawn on frame
         └─> HSV skin mask
```

| File | What it does |
|---|---|
| `Webcam.cpp` | Opens the device, sets YUYV 640×480, requests and memory-maps kernel buffers, queues/dequeues one per frame, converts YUYV to RGB with integer math |
| `Image_utils.cpp` | Separable box blur, grayscale, HSV skin mask, edge mask, frame-difference motion mask with decaying motion history, generic convolution, BFS flood fill into bounding boxes, drawing |
| `Contour.cpp`, `Geo.cpp` | Contour extraction, polygon area and aspect ratio, and a hand-candidate filter (not wired into the main loop yet) |
| `View.cpp` | Debug views and on-screen overlay |
| `Params.h` | Every tunable value in one place (resolution, blur size, skin hue ranges, area/aspect filters) |

## Build and run

Linux only (V4L2). Needs a C++23 compiler, CMake 4.0+, OpenCV (for display) and a webcam at
`/dev/video0`.

```bash
cmake -B build
cmake --build build
./build/new_gesture
```

| Key | Action |
|---|---|
| `d` | Toggle debug mode |
| `0` / `1` / `2` (debug mode) | Final frame / grayscale mask / skin mask |
| `Esc` | Quit |

## Roadmap

1. Find hands with the skin mask instead of the brightness mask, keeping blobs that pass the
   area and aspect-ratio filters already in `Params.h`.
2. Count raised fingers from each hand's outline (convex hull and convexity defects), with no
   training data needed.
3. Track hands across frames and recognize simple gestures from finger count and motion.
4. Make the blur cost independent of kernel size with a running sum.
