================================================================================
CSCI 576 - Assignment 1
Image Processing: Resampling and Quantization
================================================================================

PROJECT OVERVIEW
----------------
This project implements an image processing pipeline with the following stages:
  1. Filtering     - 3x3 averaging filter applied before downscaling
  2. Resampling    - Nearest-neighbor scaling by factor S
  3. Quantization  - Three modes:
       M = -1   : Uniform (equal-interval) quantization
       M = 0-255: Pivot quantization (split range at value M)
       M = 256  : Optimal equal-frequency histogram quantization

A separate analysis tool (analyze) computes MSE and MAE error metrics for the
quantization analysis section of the assignment.

Directory structure:
  src/                  C++ source files
    Main.cpp            GUI entry point (wxWidgets)
    processing.cpp/.h   Filter, resample, quantize, pipeline
    image.cpp/.h        Image class
    analyze_main.cpp    Standalone CLI for error analysis (no wxWidgets)
    analysis.cpp/.h     MSE and MAE computation
  scripts/
    run_experiment.sh   Runs the full Q/M sweep for one image, outputs CSV
    plot_results.py     Plots MSE and MAE curves from CSV files
  sample_images/        512x512 .rgb test images
  results/              Output CSVs and plots (created at runtime)
  build/                CMake build output

================================================================================
1. COMPILATION INSTRUCTIONS
================================================================================

Prerequisites:
  - CMake >= 3.5
  - C++17-compatible compiler (clang or gcc)
  - Python 3 with matplotlib (for plotting only)

Step 1 - First-time setup (fetch submodules for wxWidgets):

    git submodule update --init --recursive

Step 2 - Configure CMake:

    cmake -S . -B build

    In VSCode: Command Palette > "CMake: Configure"
    Select your compiler when prompted (clang recommended on macOS).

Step 3 - Build all targets:

    cmake --build build

    In VSCode: Command Palette > "CMake: Build"

    This produces two executables inside the build/ folder:
      build/MyImageApplication   (GUI viewer)
      build/analyze              (CLI error analysis tool)

    Note: The first build may take several minutes because wxWidgets is compiled
    from source. Subsequent builds are incremental and much faster.

Step 4 - Rebuild after source changes:

    cmake --build build

    In VSCode: Command Palette > "CMake: Build"

    For a full clean rebuild (e.g. after changing CMakeLists.txt):
    Command Palette > "CMake: Clean Rebuild"

Install matplotlib (required for plotting):

    pip3 install matplotlib

================================================================================
2. RUN INSTRUCTIONS
================================================================================

--- GUI Viewer (MyImageApplication) ---

Usage:
    ./build/MyImageApplication <imagePath> <S> <Q> <M>

Arguments:
    imagePath   Path to a 512x512 .rgb image file (planar RGB format)
    S           Scale factor (float). Use 1.0 for no scaling.
    Q           Total bits per pixel (integer, must be 3-24 and divisible by 3)
                Examples: 3, 6, 9, 12, 15, 18, 21, 24
    M           Quantization mode:
                  -1       Uniform quantization
                  0-255    Pivot quantization (split at value M)
                  256      Optimal equal-frequency quantization

Examples:
    # No scaling, 24-bit (lossless), uniform quantization
    ./build/MyImageApplication sample_images/Lena_512_512.rgb 1.0 24 -1

    # Scale to 50%, 12-bit, pivot at 128
    ./build/MyImageApplication sample_images/Lena_512_512.rgb 0.5 12 128

    # No scaling, 6-bit, optimal quantization
    ./build/MyImageApplication sample_images/test1_512x512.rgb 1.0 6 256

--- Analysis CLI (analyze) ---

Usage:
    ./build/analyze <imagePath> <Q> <M>

Output: one CSV row printed to stdout:
    <imageName>,<Q>,<M>,<MSE>,<MAE>

Example:
    ./build/analyze sample_images/Lena_512_512.rgb 6 -1
    # Output: Lena_512_512.rgb,6,-1,259893000,12230600

Note: S is fixed at 1.0 (no scaling) as required by the analysis question.

================================================================================
3. SCRIPT INSTRUCTIONS
================================================================================

--- run_experiment.sh ---

Runs the full experiment sweep for a single image:
  - Q values: 3, 6, 9, 12, 15, 18, 21, 24
  - M values: -1 (Uniform), 128 (Pivot), 256 (Optimal)
Produces a CSV file with all 24 combinations.

Usage:
    ./scripts/run_experiment.sh <imagePath> [outputCSV]

Arguments:
    imagePath   Path to the .rgb image file
    outputCSV   (Optional) Output CSV path.
                Default: results/<imageName>.csv

Examples:
    # Results saved to results/Lena_512_512.csv
    ./scripts/run_experiment.sh sample_images/Lena_512_512.rgb

    # Results saved to a custom path
    ./scripts/run_experiment.sh sample_images/test1_512x512.rgb results/test1.csv

CSV format:
    image,Q,M,MSE,MAE
    Lena_512_512.rgb,3,-1,958316000,23495800
    Lena_512_512.rgb,3,128,946061000,23282000
    ...

--- plot_results.py ---

Reads one or more CSV files and produces MSE and MAE plots for each image.
Each image gets its own pair of output PNG files.

Usage:
    python3 scripts/plot_results.py <csv1> [<csv2> ...]

Arguments:
    csv1, csv2, ...   Absolute or relative paths to result CSV files.
                      Use absolute paths to avoid ambiguity.

Output files (saved in the same directory as each CSV):
    <imageName>_MSE.png   MSE vs Q plot (3 curves, one per M mode)
    <imageName>_MAE.png   MAE vs Q plot (3 curves, one per M mode)

Plot layout:
    X axis: Q values (3, 6, 9, ..., 24)
    Y axis: Error value (MSE or MAE)
    Colors: Blue = M=-1 (Uniform), Orange = M=128 (Pivot), Green = M=256 (Optimal)
    Markers: Different shape per image when multiple images are plotted together

Examples:
    # Single image
    python3 scripts/plot_results.py "$(pwd)/results/Lena_512_512.csv"

    # Multiple images (each gets its own output files)
    python3 scripts/plot_results.py \
        "$(pwd)/results/Lena_512_512.csv" \
        "$(pwd)/results/test1_512x512.csv" \
        "$(pwd)/results/test2_512x512.csv"

--- Full experiment workflow for one image ---

    # Step 1: Build (if not already built)
    cmake -S . -B build && cmake --build build

    # Step 2: Run experiment and save CSV
    ./scripts/run_experiment.sh sample_images/Lena_512_512.rgb

    # Step 3: Plot
    python3 scripts/plot_results.py "$(pwd)/results/Lena_512_512.csv"

    # Step 4: View results
    open results/Lena_512_512_MSE.png
    open results/Lena_512_512_MAE.png

================================================================================
