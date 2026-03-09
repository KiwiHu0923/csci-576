CS 576 - Assignment 2: Content-Adaptive NxN Block Image Encoder/Decoder
=========================================================================

COMPILATION INSTRUCTIONS
------------------------

This project uses CMake for building. To compile:

1. Run CMake to generate build files:
   cmake ..

2. The executable will be generated in the build directory


RUNNING THE PROGRAM
-------------------

The program takes 4 command-line arguments:

./MyImageApplication InputImage M Q B

Parameters:
  - InputImage: Path to input image file (.rgb format, 512x512)
  - M: Mode (1 for fixed 8x8 blocks, 2 for adaptive NxN blocks)
  - Q: Quantizer value (positive integer or -1)
       - If positive: quantize DCT coefficients by 2^Q
       - If -1: automatically find Q to achieve target BPP (requires B > 0)
  - B: Bits per pixel (floating point or -1.0)
       - If -1.0: Q parameter controls quality
       - If > 0: automatically find Q to achieve this target BPP (requires Q = -1)

Note: Either Q or B must be -1, both cannot be positive simultaneously.


EXAMPLES
--------

1. Fixed 8x8 blocks with Q=0 (no quantization):
   ./MyImageApplication inputs/test0.rgb 1 0 -1

2. Fixed 8x8 blocks with Q=10 (heavy quantization):
   ./MyImageApplication inputs/test0.rgb 1 10 -1

3. Adaptive NxN blocks with Q=6:
   ./MyImageApplication inputs/test0.rgb 2 6 -1

4. Fixed 8x8 blocks with target 1.5 bits per pixel:
   ./MyImageApplication inputs/test0.rgb 1 -1 1.5

5. Adaptive NxN blocks with target 1.5 bits per pixel:
   ./MyImageApplication inputs/test0.rgb 2 -1 1.5


KEYBOARD CONTROLS
-----------------

Press 'B' or 'b' to toggle the display of block boundaries on/off.
This visualizes the NxN block partitioning used by the encoder.


OUTPUT FILES
------------

The program generates .DCT files containing quantized DCT coefficients:
  - M=1 (fixed): Saved in dct/ directory as <basename>_Q<value>.DCT
  - M=2 (adaptive): Saved in dct_adapt/ directory as <basename>_Q<value>.DCT

These files are used for compression evaluation using the zip metric:
  Compressed bits/pixel = zip(file.DCT) / (width × height)


IMPLEMENTATION DETAILS
----------------------

Mode M=1 (Fixed 8x8 Blocks):
  - Image divided into fixed 8x8 blocks
  - Standard JPEG-like pipeline: DCT -> Quantize -> Dequantize -> IDCT
  - Quantization by 2^Q for all coefficients uniformly

Mode M=2 (Adaptive NxN Blocks):
  - Content-adaptive block partitioning
  - Block sizes: {2, 4, 8, 16, 32}
  - Quadtree-like splitting based on high-frequency DCT energy
  - Threshold: 0.15 (configurable in block_decision.cpp)

Automatic Q-Finding (B > 0):
  - Binary search to find optimal Q value
  - Target: compressed BPP ≤ B
  - Optimization: DCT computed once, only quantization varies
  - Search range: Q ∈ [0, 20]
  - Convergence: stops when |actual_BPP - target_BPP| < 0.05


PROJECT STRUCTURE
-----------------

src/
  main.cpp              - wxWidgets GUI application entry point
  controller.cpp        - Main encoding/decoding pipelines
  controller.h          - Pipeline function declarations
  dct.cpp              - DCT/IDCT implementations (1D and 2D)
  quantizer.cpp        - Quantization/dequantization functions
  block_decision.cpp   - Adaptive block partitioning algorithm
  block_decision.h     - Block decision interface

CMakeLists.txt         - CMake build configuration
README.txt            - This file


DEPENDENCIES
------------

- wxWidgets 3.0 or higher (for GUI display)
- C++11 compatible compiler
- CMake 3.10 or higher
- gzip (for compression evaluation)


NOTES
-----

- Input images must be 512x512 pixels in .rgb format
- RGB format: RRRR...GGGG...BBBB (all R values, then G, then B)
- Level shifting: subtract 128 before DCT, add 128 after IDCT
- Output displayed using wxWidgets scrollable window
- Block boundaries drawn in green when 'B' key pressed
- For B > 0 mode, directories dct/ or dct_adapt/ must exist


AUTHOR
------

CS 576 Spring 2026
Assignment 2 Implementation
