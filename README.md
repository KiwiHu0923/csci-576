# CSCI 576 - Multimedia Systems Design

A comprehensive collection of multimedia processing projects demonstrating expertise in image compression, transform coding, and digital signal processing algorithms. These implementations showcase practical applications of theoretical concepts from JPEG/MPEG standards, content-adaptive encoding, and quantitative performance analysis.

## Projects Overview

### [HW1 - Image Processing Pipeline](HW1-Image%20Processing%20Pipeline/)
**Image Resampling, Filtering & Quantization**

A complete image processing pipeline implementing anti-aliasing filters, nearest-neighbor resampling, and three quantization modes (uniform, logarithmic, optimal histogram-based). Features automated analysis framework with MSE/MAE metrics across 24 parameter configurations.

**Technologies**: C++17 · wxWidgets · CMake · Python · Matplotlib

### [HW2 - Content-Adaptive NxN Block Image Encoder/Decoder](HW2-Content-Adaptive%20NxN%20Block%20Image%20Encoder-Decoder/)
**DCT-Based Compression with Adaptive Block Partitioning**

A sophisticated DCT-based image compression system with dual modes: fixed 8×8 JPEG-like encoding and adaptive NxN block partitioning using quadtree-based content analysis. Achieves 25% quality improvement over baseline JPEG at equivalent bitrates through rate-distortion optimization.

**Technologies**: C++11 · wxWidgets · DCT · Quadtree Algorithms · CMake

## Repository Structure

```
HW/
├── HW1-Image Processing Pipeline/
│   ├── src/                      # Image processing pipeline source
│   ├── scripts/                  # Automation and analysis scripts
│   ├── inputs/                   # Test images (512x512 RGB)
│   ├── results/                  # Experiment outputs and plots
│   └── README.md                 # HW1 documentation
│
├── HW2-Content-Adaptive NxN Block Image Encoder-Decoder/
│   ├── src/                      # Encoder/decoder source
│   ├── inputs/                   # Test images
│   ├── dct/                      # Fixed 8x8 DCT outputs
│   ├── dct_adapt/                # Adaptive NxN DCT outputs
│   └── README.md                 # HW2 documentation
│
├── dependency/
│   └── wxWidgets/                # Cross-platform GUI library (submodule)
│
└── README.md                     # This file
```

## Quick Start

### Prerequisites

- **CMake** ≥ 3.10
- **C++ Compiler**: C++17 compatible (GCC/Clang)
- **wxWidgets**: Included as submodule
- **Python 3** + matplotlib (for HW1 analysis)
- **gzip** (for HW2 compression evaluation)

## Development Environment

### Tested Configurations

**Primary Development**:
- **Machine**: Apple Silicon Mac (M3 Max)
- **OS**: macOS Sequoia 15.7.3
- **Architecture**: arm64
- **Compiler**: Apple Clang 17.0.0
- **CMake**: 4.2.3


## Technical Skills Demonstrated

### Image Processing & Compression
- Discrete Cosine Transform (DCT/IDCT)
- Quantization strategies (uniform, logarithmic, optimal)
- Anti-aliasing filtering
- Image resampling and downscaling
- Rate-distortion optimization

### Algorithm Design
- Content-adaptive block partitioning
- Quadtree-based subdivision
- Binary search optimization
- Histogram equalization
- Energy-based heuristics

### Software Engineering
- Cross-platform C++ development
- CMake build systems
- GUI development (wxWidgets)
- Shell scripting automation
- Data visualization (Python/Matplotlib)
- Git submodule management

### Performance Analysis
- MSE/MAE error metrics
- Compression ratio evaluation
- Comparative quality assessment
- Automated experiment frameworks

## Credits


**Course**: CSCI 576 - Multimedia Systems Design, University of Southern California

**Semester**: Spring 2026

---

**Technologies**: C++17 · C++11 · wxWidgets · CMake · DCT · JPEG · Python · Matplotlib · Transform Coding · Image Compression · Algorithm Design