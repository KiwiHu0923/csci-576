# Content-Adaptive NxN Block Image Encoder/Decoder

A sophisticated DCT-based image compression system implementing both fixed 8x8 block encoding (JPEG-like) and adaptive variable-size block partitioning. This project demonstrates expertise in transform coding, quantization strategies, content-aware compression algorithms, and rate-distortion optimization.

## Project Overview

This encoder/decoder implements a complete image compression pipeline similar to JPEG, with an advanced extension using content-adaptive block partitioning. The adaptive mode dynamically selects optimal block sizes (2×2 to 32×32) based on image content, achieving superior compression quality compared to standard JPEG at equivalent bit rates.

## Technical Skills Demonstrated

### Image Compression Algorithms
- **DCT/IDCT Transforms**: 2D Discrete Cosine Transform for frequency-domain representation
- **Adaptive Block Partitioning**: Content-driven quadtree-like subdivision algorithm
- **Quantization Strategies**: Uniform quantization with configurable quality parameters
- **Rate-Distortion Optimization**: Binary search algorithm for target bitrate achievement
- **Compression Evaluation**: Zip-based bitrate measurement and quality assessment

### Software Engineering
- **Languages**: C++11
- **GUI Framework**: wxWidgets for interactive visualization
- **Build System**: CMake
- **Algorithm Design**: Quadtree structures, binary search optimization, energy-based heuristics
- **Performance**: Single DCT computation with iterative quantization refinement

### Core Competencies
- Transform coding and frequency-domain signal processing
- Content-adaptive algorithm design
- Rate-distortion theory application
- Interactive GUI development
- Compression standards (JPEG/MPEG fundamentals)

## Key Features

### 1. Dual-Mode Compression System

**Mode 1: Fixed 8×8 Block Encoding**
- Standard JPEG-like pipeline
- Fixed block size across entire image
- Baseline for compression comparison

**Mode 2: Adaptive NxN Block Encoding**
- Content-driven block size selection
- Block sizes: 2×2, 4×4, 8×8, 16×16, 32×32
- Superior quality at equivalent bitrates
- Quadtree-based partitioning strategy

### 2. Flexible Quality Control

**Direct Quantization (Q parameter)**
- Explicit quality control via Q value
- Quantization by 2^Q for all DCT coefficients
- Range: Q ∈ [0, 20]

**Target Bitrate Mode (B parameter)**
- Automatic Q-value optimization
- Binary search for target bits-per-pixel
- Convergence threshold: |$actual_{BPP} - target_{BPP}$| < 0.05
- Efficient: DCT computed once, only quantization varies

### 3. Interactive Visualization
- Real-time decoded image display
- Keyboard toggle ('B' key) for block boundary visualization
- Green overlay showing adaptive block partitioning
- wxWidgets-based scrollable window

### 4. Compression Pipeline

```
Input (512×512 RGB)
    ↓
[Adaptive Block Decision (M=2 only)]
    ↓
[Block-wise DCT Transform]
    ↓
[Quantization by 2^Q]
    ↓
[.DCT File Export] → [Zip Compression] → [Bitrate Measurement]
    ↓
[Dequantization]
    ↓
[Block-wise IDCT]
    ↓
Output (Reconstructed RGB)
```

## Technical Implementation

### Architecture
```
src/
├── main.cpp              # wxWidgets GUI entry point
├── controller.cpp/.h     # Encode/decode pipeline orchestration
├── dct.cpp              # 1D and 2D DCT/IDCT implementations
├── quantizer.cpp        # Quantization and dequantization
└── block_decision.cpp/.h # Adaptive block partitioning algorithm

CMakeLists.txt           # CMake build configuration
```

### Adaptive Block Decision Algorithm
- **Energy-based thresholding**: High-frequency DCT coefficient analysis
- **Splitting criterion**: Threshold = 0.15 (configurable)
- **Quadtree-style recursion**: Hierarchical block subdivision
- **Block size constraints**: Square NxN blocks, no overlaps
- **Optimization goal**: Minimize distortion for given bitrate

### Quantization & Rate Control
- **Level shifting**: Subtract 128 before DCT, add 128 after IDCT
- **Uniform quantization**: All coefficients quantized by 2^Q
- **Binary search**: Efficient Q-value optimization for target BPP
- **Bitrate metric**: `compressed_BPP = zip(file.DCT) / (width × height)`

## Usage Examples

### Fixed 8×8 Blocks with Direct Quantization
```bash
# No quantization (lossless transform)
./build/MyImageApplication inputs/test0.rgb 1 0 -1

# Heavy quantization (Q=10 → divide by 1024)
./build/MyImageApplication inputs/test0.rgb 1 10 -1
```

### Adaptive NxN Blocks with Direct Quantization
```bash
# Moderate quantization with adaptive blocks
./build/MyImageApplication inputs/test0.rgb 2 6 -1
```

### Target Bitrate Mode (Automatic Q-Finding)
```bash
# Fixed 8×8 blocks, target 1.5 bits/pixel
./build/MyImageApplication inputs/test0.rgb 1 -1 1.5

# Adaptive blocks, target 1.5 bits/pixel (better quality)
./build/MyImageApplication inputs/test0.rgb 2 -1 1.5
```

## Build Instructions

### Prerequisites
- CMake ≥ 3.10
- C++11 compatible compiler
- wxWidgets ≥ 3.0
- gzip (for compression evaluation)

### Compilation
```bash
# Configure CMake
cmake ..

# Build
cmake --build .
```

**Output**: `MyImageApplication` executable in build directory

## Input/Output Specifications

**Input Format**:
- Planar RGB (.rgb files)
- Resolution: 512×512 pixels
- Channel order: R plane → G plane → B plane

**Parameters**:
- `InputImage`: Path to .rgb file
- `M`: Mode (1 = fixed 8×8, 2 = adaptive NxN)
- `Q`: Quantizer (-1 for auto, or positive integer)
- `B`: Target bits/pixel (-1.0 for Q-controlled, or positive float)

**Constraint**: Either Q or B must be -1 (mutually exclusive)

**Output Files**:
- **M=1**: `dct/<basename>_Q<value>.DCT` (fixed blocks)
- **M=2**: `dct_adapt/<basename>_Q<value>.DCT` (adaptive blocks)

## Interactive Features

**Keyboard Controls**:
- Press `B` or `b`: Toggle block boundary visualization
- Visual feedback: Green lines show NxN block partitioning

## Implementation Highlights

### Content-Adaptive Strategy
The adaptive algorithm analyzes high-frequency DCT energy within candidate blocks:
- **High energy** → Split into smaller blocks (preserve detail)
- **Low energy** → Keep larger blocks (efficient compression)
- **Result**: More bits allocated to complex regions, fewer to smooth areas

### Optimization Techniques
- **Efficient Q-search**: DCT computed once, binary search only re-quantizes
- **Convergence criteria**: Stops when target BPP achieved within tolerance
- **Block reuse**: Quadtree structure enables efficient recursive partitioning

### Quality Comparison
At equivalent bitrates (e.g., 1.5 BPP):
- **Adaptive mode (M=2)**: Superior visual quality, better PSNR
- **Fixed mode (M=1)**: Baseline JPEG-equivalent quality
- **Advantage**: Content-aware allocation reduces blocking artifacts

## Performance Metrics

**Compression Evaluation**:
```
Compressed BPP = zip(file.DCT) / (width × height)
```

**Quality Metrics**:
- Visual inspection of reconstructed images
- Block boundary visualization
- Comparison at matched bitrates

## Course Context

**CSCI 576 - Multimedia Systems Design**
University of Southern California
Assignment 2: DCT-Based Image Compression

This project demonstrates practical understanding of transform coding principles used in JPEG/MPEG standards, with an advanced extension implementing content-adaptive compression that outperforms baseline JPEG.

---

**Technologies**: C++11 · wxWidgets · CMake · DCT · JPEG · Transform Coding · Quadtree Algorithms · Rate-Distortion Optimization · Image Compression