
import sys
import os
import csv
from collections import defaultdict
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

M_LABELS = {
    -1:  "M=-1  (Uniform)",
    128: "M=128 (Pivot 128)",
    256: "M=256 (Optimal equal-freq)",
}

# One distinct color per M value, one marker/linestyle per image
M_COLORS = {-1: "tab:blue", 128: "tab:orange", 256: "tab:green"}
M_LINESTYLES = {-1: "-", 128: "-", 256: "-"}
IMG_MARKERS = ["o", "s", "^", "D", "v", "P"]


def load_csv(path):
    """Return list of dicts from a results CSV."""
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({
                "image": row["image"],
                "Q":     int(row["Q"]),
                "M":     int(row["M"]),
                "MSE":   float(row["MSE"]),
                "MAE":   float(row["MAE"]),
            })
    return rows


def build_curves(rows):
    """
    Returns dict: M_value -> {"Q": [...], "MSE": [...], "MAE": [...]}
    sorted by Q.
    """
    by_m = defaultdict(list)
    for r in rows:
        by_m[r["M"]].append(r)
    curves = {}
    for m, pts in by_m.items():
        pts.sort(key=lambda r: r["Q"])
        curves[m] = {
            "Q":   [p["Q"]   for p in pts],
            "MSE": [p["MSE"] for p in pts],
            "MAE": [p["MAE"] for p in pts],
        }
    return curves


def plot_metric(all_data, metric, out_path):
    """
    all_data: list of (label, curves_dict)
    metric: "MSE" or "MAE"
    """
    fig, ax = plt.subplots(figsize=(9, 5))

    for i, (label, curves) in enumerate(all_data):
        marker = IMG_MARKERS[i % len(IMG_MARKERS)]
        for m in sorted(curves.keys()):
            c = curves[m]
            color = M_COLORS.get(m, "tab:gray")
            ls = M_LINESTYLES.get(m, "-")
            m_label = M_LABELS.get(m, f"M={m}")
            ax.plot(
                c["Q"], c[metric],
                color=color,
                linestyle=ls,
                marker=marker,
                markersize=5,
                label=f"{label} | {m_label}",
            )

    ax.set_xlabel("Q (total bits per pixel)", fontsize=12)
    ax.set_ylabel(metric, fontsize=12)
    ax.set_title(f"{metric} vs Q for different quantization modes", fontsize=13)
    ax.xaxis.set_major_locator(ticker.MultipleLocator(3))
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f"{x:,.0f}"))
    ax.grid(True, linestyle="--", alpha=0.5)
    ax.legend(fontsize=8, loc="upper right")

    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    print(f"Saved: {out_path}")
    plt.close(fig)


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 scripts/plot_results.py results/<image>.csv [...]")
        sys.exit(1)

    csv_paths = sys.argv[1:]

    for path in csv_paths:
        rows = load_csv(path)
        if not rows:
            print(f"Warning: {path} is empty, skipping.")
            continue

        label = rows[0]["image"].replace(".rgb", "")
        curves = build_curves(rows)
        single = [(label, curves)]

        out_dir = os.path.dirname(os.path.abspath(path))
        os.makedirs(out_dir, exist_ok=True)

        stem = os.path.splitext(os.path.basename(path))[0]  # e.g. "Lena_512_512"
        plot_metric(single, "MSE", os.path.join(out_dir, f"{stem}_MSE.png"))
        plot_metric(single, "MAE", os.path.join(out_dir, f"{stem}_MAE.png"))


if __name__ == "__main__":
    main()
