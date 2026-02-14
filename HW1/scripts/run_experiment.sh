#!/usr/bin/env bash
#
# run_experiment.sh — run the analysis experiment for one image
#
# Usage:
#   ./scripts/run_experiment.sh <imagePath> [outputCSV]
#
# Example:
#   ./scripts/run_experiment.sh sample_images/Lena_512_512.rgb results/Lena.csv
#
# If outputCSV is omitted, results are written to results/<imageName>.csv
#
# The script sweeps Q over 3,6,9,...,24 and M over -1, 128, 256.
# Output CSV columns: image,Q,M,MSE,MAE

set -euo pipefail

IMAGE_PATH="${1:-}"
if [[ -z "$IMAGE_PATH" ]]; then
    echo "Usage: $0 <imagePath> [outputCSV]"
    exit 1
fi

# Resolve the analyze binary (look next to this script's parent, then in PATH)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

ANALYZE_BIN=""
for candidate in \
    "$REPO_ROOT/build/analyze" \
    "$REPO_ROOT/build/Debug/analyze" \
    "$REPO_ROOT/build/Release/analyze" \
    "$(command -v analyze 2>/dev/null || true)"
do
    if [[ -x "$candidate" ]]; then
        ANALYZE_BIN="$candidate"
        break
    fi
done

if [[ -z "$ANALYZE_BIN" ]]; then
    echo "Error: 'analyze' binary not found. Build the project first:"
    echo "  cmake -S . -B build && cmake --build build"
    exit 1
fi

IMAGE_NAME="$(basename "$IMAGE_PATH")"
RESULTS_DIR="$REPO_ROOT/results"
mkdir -p "$RESULTS_DIR"

OUTPUT_CSV="${2:-$RESULTS_DIR/${IMAGE_NAME%.rgb}.csv}"

echo "Image:   $IMAGE_PATH"
echo "Binary:  $ANALYZE_BIN"
echo "Output:  $OUTPUT_CSV"
echo ""

# Write CSV header
echo "image,Q,M,MSE,MAE" > "$OUTPUT_CSV"

Q_VALUES=(3 6 9 12 15 18 21 24)
M_VALUES=(-1 128 256)

for Q in "${Q_VALUES[@]}"; do
    for M in "${M_VALUES[@]}"; do
        ROW="$("$ANALYZE_BIN" "$IMAGE_PATH" "$Q" "$M")"
        echo "$ROW" >> "$OUTPUT_CSV"
        echo "  Q=$Q  M=$M  -> $ROW"
    done
done

echo ""
echo "Done. Results saved to: $OUTPUT_CSV"
