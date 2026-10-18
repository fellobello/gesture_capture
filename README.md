# gesture_capture

[![ci](https://github.com/fellobello/gesture_capture/actions/workflows/ci.yml/badge.svg)](https://github.com/fellobello/gesture_capture/actions/workflows/ci.yml)

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
| `Image_utils.cpp` | YUYV to BGR conversion, separable box blur, grayscale, HSV skin mask, edge mask, frame-difference motion mask with decaying motion history, generic convolution, BFS flood fill into bounding boxes, drawing |
| `Contour.cpp`, `Geo.cpp` | Contour extraction, polygon area and aspect ratio, and a hand-candidate filter (not wired into the main loop yet) |
| `View.cpp` | Debug views and on-screen overlay |
| `Params.h` | Every tunable value in one place (resolution, blur size, skin hue ranges, area/aspect filters) |

## Build and run

The live app is Linux only (V4L2). It needs a C++23 compiler, CMake 3.20+, OpenCV (for display)
and a webcam at `/dev/video0`.

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

The image pipeline itself (everything except `Webcam`, `View` and `main`) is plain C++ with no
OpenCV, so the tests and the benchmark build on macOS too.

## Tests

Unit tests (GoogleTest, fetched by CMake) run each stage on small synthetic images: YUYV
conversion, channel order, blur, skin mask, flood-fill boxes, box drawing at the image edge,
motion history and the polygon helpers. CI runs them on every push with AddressSanitizer and
UndefinedBehaviorSanitizer, so an out-of-bounds write fails the build.

```bash
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DGESTURE_SANITIZE=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

## Performance

`bench_pipeline` times every stage of the main loop on a synthetic 640×480 frame, no camera
needed:

```bash
./build/bench_pipeline 300
```

Release build, Apple M5, 300 frames:

| Stage | Median ms | p99 ms |
|---|---|---|
| YUYV → BGR | 0.32 | 0.47 |
| Box blur (k=3) | 2.35 | 3.20 |
| Grayscale | 0.15 | 0.19 |
| Skin mask | 0.40 | 0.53 |
| Threshold | 0.00 | 0.01 |
| Flood fill → boxes | 1.24 | 1.63 |
| Draw boxes | 0.01 | 0.01 |
| **Total** | **4.47** | **6.04** |

About 220 frames/s, far above the camera's 30. The blur is the most expensive stage, which is
what roadmap item 4 targets.

## Roadmap

1. Find hands with the skin mask instead of the brightness mask, keeping blobs that pass the
   area and aspect-ratio filters already in `Params.h`.
2. Count raised fingers from each hand's outline (convex hull and convexity defects), with no
   training data needed.
3. Track hands across frames and recognize simple gestures from finger count and motion.
4. Make the blur cost independent of kernel size with a running sum.
