# Image Processing Pipeline: Resampling, Filtering & Quantization

A comprehensive C++ image processing application implementing advanced algorithms for image downscaling, anti-aliasing, and multi-mode quantization. This project demonstrates expertise in digital image processing fundamentals, algorithm optimization, and quantitative analysis of compression techniques.

## Project Overview

This application processes 512x512 RGB images through a configurable pipeline that simulates real-world media distribution workflows (similar to 4K cinema to HD Blu-ray conversion). It features a wxWidgets-based GUI viewer and a CLI analysis tool for quantitative performance evaluation.

## Technical Skills Demonstrated

### Image Processing Algorithms
- **Anti-Aliasing Filtering**: 3x3 averaging kernel for pre-downscaling smoothing
- **Image Resampling**: Nearest-neighbor downscaling with arbitrary scale factors
- **Multi-Mode Quantization**: Three distinct bit-depth reduction strategies
  - Uniform quantization (equal-interval binning)
  - Logarithmic quantization with adaptive pivot points
  - Optimal equal-frequency histogram quantization

### Software Engineering
- **Languages**: C++17
- **GUI Framework**: wxWidgets for cross-platform image visualization
- **Build System**: CMake with modular target configuration
- **Data Analysis**: Python with matplotlib for error metric visualization
- **Performance Analysis**: MSE (Mean Squared Error) and MAE (Mean Absolute Error) metrics


## Key Features

### 1. Adaptive Image Resampling
- Configurable scale factor (0.0 - 1.0)
- 3x3 averaging filter prevents aliasing artifacts
- Proper edge handling for border pixels

### 2. Three-Mode Quantization System

**Uniform Quantization (M = -1)**
- Equal-interval binning across 0-255 range
- Baseline compression approach

**Logarithmic Quantization (M = 0-255)**
- Pivot-based adaptive quantization
- Optimizes for specific brightness distributions
- Reduces error in high-density pixel value ranges

**Optimal Histogram Quantization (M = 256)**
- Data-driven interval selection
- Equal-frequency binning for minimal error
- Best visual and quantitative performance

### 3. Quantitative Analysis Framework
- Automated experiment runner for parameter sweeps
- MSE and MAE computation across quantization modes
- Comparative visualization of compression performance
- Multi-image batch analysis support

## Technical Implementation

### Architecture
```
src/
├── Main.cpp              # wxWidgets GUI entry point
├── processing.cpp/.h     # Core pipeline: filter, resample, quantize
├── image.cpp/.h          # Image class with planar RGB handling
├── analyze_main.cpp      # CLI tool for error metrics
└── analysis.cpp/.h       # MSE and MAE computation

scripts/
├── run_experiment.sh     # Automated Q/M parameter sweep
└── plot_results.py       # Error metric visualization
```

### Pipeline Flow
```
Input (512x512, 24-bit RGB)
    ↓
[3x3 Averaging Filter]
    ↓
[Nearest-Neighbor Resampling]
    ↓
[Quantization (Uniform/Log/Optimal)]
    ↓
Output (Scaled, Reduced bit-depth)
```

## Usage Examples

### GUI Viewer
```bash
# 50% downscale, 12-bit color, pivot quantization at 128
./build/MyImageApplication inputs/Lena_512_512.rgb 0.5 12 128

# No scaling, 6-bit color, optimal quantization
./build/MyImageApplication inputs/test1_512x512.rgb 1.0 6 256
```

### Analysis CLI
```bash
# Compute MSE/MAE for 6-bit uniform quantization
./build/analyze inputs/Lena_512_512.rgb 6 -1
```

### Automated Experiment Workflow
```bash
# Run full Q/M sweep (3,6,9,...,24 bits × 3 modes)
./scripts/run_experiment.sh inputs/Lena_512_512.rgb

# Generate comparative plots
python3 scripts/plot_results.py "$(pwd)/results/Lena_512_512.csv"
```

## Build Instructions

### Prerequisites
- CMake ≥ 3.5
- C++17 compiler (clang/gcc)
- Python 3 + matplotlib (for plotting)

### Compilation
```bash
# First-time setup (fetch wxWidgets submodule)
git submodule update --init --recursive

# Configure and build
cmake -S . -B build
cmake --build build
```

**Executables**:
- `build/MyImageApplication` - GUI viewer
- `build/analyze` - CLI analysis tool

## Input/Output Specifications

**Input Format**:
- Planar RGB (.rgb files)
- Resolution: 512×512 pixels
- Color depth: 8 bits/channel (24 bits/pixel)
- Channel order: R plane → G plane → B plane

**Parameters**:
- `S` (Scale): Float 0.0-1.0 for downsampling factor
- `Q` (Bits): Integer 3-24 (multiple of 3) for output bit depth
- `M` (Mode): -1 (uniform), 0-255 (pivot), 256 (optimal)

## Course Context

**CSCI 576 - Multimedia Systems Design**
University of Southern California
Assignment 1: Image Processing Fundamentals

This project demonstrates practical application of digital signal processing theory to visual media, simulating real-world challenges in content distribution pipelines.

---

**Technologies**: C++17 · wxWidgets · CMake · Python · Matplotlib · Digital Signal Processing · Image Compression · Algorithm Design