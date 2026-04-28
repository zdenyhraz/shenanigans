import argparse
import math
import os
import sys
from concurrent.futures import ProcessPoolExecutor, as_completed

import numpy as np

from script.opt.cutting_plan import find_optimal_cutting_plan

try:
    from tqdm.auto import tqdm
except ImportError:
    tqdm = None

sys.setrecursionlimit(1000)

DEFAULT_LENGTH_START = 100
DEFAULT_LENGTH_END = 200
DEFAULT_WIDTH_START = 60
DEFAULT_WIDTH_END = 100
DEFAULT_STEP = 5
NUM_BEDS = 2


def build_user_data(long_length, width_length, stock_length, stock_cost, saw_kerf):
    required_cuts = {}

    def add_cut(length, count):
        required_cuts[length] = required_cuts.get(length, 0) + count

    add_cut(long_length, NUM_BEDS*8)
    add_cut(long_length - 8, NUM_BEDS*6)
    add_cut(width_length, NUM_BEDS*8)
    add_cut(width_length - 8, NUM_BEDS*6)

    return [
        {
            "name": f"Raised garden bed: {long_length}x{width_length}cm",
            "stock_length": stock_length,
            "stock_cost": stock_cost,
            "saw_kerf": saw_kerf,
            "required_cuts": required_cuts,
        }
    ]


def evaluate_configuration(long_length, width_length, stock_length=500, stock_cost=411.5, saw_kerf=0.5):
    user_data = build_user_data(
        long_length,
        width_length,
        stock_length=stock_length,
        stock_cost=stock_cost,
        saw_kerf=saw_kerf,
    )
    result = find_optimal_cutting_plan(user_data, verbose=False)
    return result.get("grand_total_cost", math.inf)


def _evaluate_single_configuration(task):
    long_length, width_length, stock_length, stock_cost, saw_kerf = task
    try:
        cost = evaluate_configuration(
            long_length,
            width_length,
            stock_length=stock_length,
            stock_cost=stock_cost,
            saw_kerf=saw_kerf,
        )
    except ValueError:
        cost = math.inf
    return long_length, width_length, cost


def _wrap_with_progress(iterable, total, show_progress, description):
    if not show_progress:
        return iterable

    if tqdm is None:
        print("tqdm is not installed, running without a progress bar.")
        return iterable

    return tqdm(iterable, total=total, desc=description, unit="cfg")


def evaluate_costs(
    length_start=DEFAULT_LENGTH_START,
    length_end=DEFAULT_LENGTH_END,
    step=DEFAULT_STEP,
    width=DEFAULT_WIDTH_START,
    stock_length=500,
    stock_cost=411.5,
    saw_kerf=0.5,
    show_progress=True,
):
    results = []
    long_lengths = list(range(length_start, length_end + 1, step))
    progress = _wrap_with_progress(
        long_lengths,
        total=len(long_lengths),
        show_progress=show_progress,
        description="Evaluating lengths",
    )
    for long_length in progress:
        cost = evaluate_configuration(
            long_length,
            width,
            stock_length=stock_length,
            stock_cost=stock_cost,
            saw_kerf=saw_kerf,
        )
        results.append((long_length, width, cost))
    return results


def evaluate_cost_grid(
    length_start=DEFAULT_LENGTH_START,
    length_end=DEFAULT_LENGTH_END,
    length_step=DEFAULT_STEP,
    width_start=DEFAULT_WIDTH_START,
    width_end=DEFAULT_WIDTH_END,
    width_step=DEFAULT_STEP,
    stock_length=500,
    stock_cost=411.5,
    saw_kerf=0.5,
    max_workers=None,
    show_progress=True,
):
    long_lengths = list(range(length_start, length_end + 1, length_step))
    widths = list(range(width_start, width_end + 1, width_step))
    tasks = [
        (long_length, width_length, stock_length, stock_cost, saw_kerf)
        for width_length in widths
        for long_length in long_lengths
    ]

    if max_workers is None:
        max_workers = max(1, (os.cpu_count() or 1) - 1)

    cost_grid = np.full((len(widths), len(long_lengths)), math.inf, dtype=float)
    width_to_row = {width: index for index, width in enumerate(widths)}
    length_to_col = {length: index for index, length in enumerate(long_lengths)}

    if max_workers == 1:
        results = map(_evaluate_single_configuration, tasks)
        results = _wrap_with_progress(
            results,
            total=len(tasks),
            show_progress=show_progress,
            description="Evaluating bed sizes",
        )
    else:
        with ProcessPoolExecutor(max_workers=max_workers) as executor:
            futures = [
                executor.submit(_evaluate_single_configuration, task)
                for task in tasks
            ]
            progress = _wrap_with_progress(
                as_completed(futures),
                total=len(futures),
                show_progress=show_progress,
                description="Evaluating bed sizes",
            )
            for future in progress:
                long_length, width_length, cost = future.result()
                row = width_to_row[width_length]
                col = length_to_col[long_length]
                cost_grid[row, col] = cost
        return long_lengths, widths, cost_grid

    for long_length, width_length, cost in results:
        row = width_to_row[width_length]
        col = length_to_col[long_length]
        cost_grid[row, col] = cost

    return long_lengths, widths, cost_grid


def find_price_jumps(points):
    jumps = []
    for i in range(1, len(points)):
        prev_cost = points[i - 1][2]
        cost = points[i][2]
        if cost != prev_cost:
            jumps.append((points[i][0], points[i][1], prev_cost, cost, cost - prev_cost))
    return jumps


def summarize_cost_grid(long_lengths, widths, cost_grid, top_n=10):
    finite_mask = np.isfinite(cost_grid)
    if not finite_mask.any():
        print("No feasible raised garden bed dimensions were found in the scanned range.")
        return

    best_index = np.unravel_index(np.argmin(np.where(finite_mask, cost_grid, np.inf)), cost_grid.shape)
    best_width = widths[best_index[0]]
    best_length = long_lengths[best_index[1]]
    best_cost = cost_grid[best_index]

    print(
        f"Best configuration: length={best_length} cm, width={best_width} cm -> cost={best_cost:.1f} CZK"
    )

    flattened = [
        (long_lengths[col], widths[row], cost_grid[row, col])
        for row in range(len(widths))
        for col in range(len(long_lengths))
        if math.isfinite(cost_grid[row, col])
    ]
    flattened.sort(key=lambda item: (item[2], item[0], item[1]))

    print(f"\nTop {top_n} cheapest configurations:")
    for long_length, width_length, cost in flattened[:top_n]:
        print(f"Length={long_length} cm, Width={width_length} cm -> Cost={cost:.1f} CZK")


def build_cost_table(long_lengths, widths, cost_grid):
    try:
        import pandas as pd
    except ImportError:
        return None

    return pd.DataFrame(cost_grid, index=widths, columns=long_lengths)


def print_cost_table(cost_table):
    if cost_table is None:
        print("\nInstall pandas to print the 2D cost table in tabular form: pip install pandas")
        return

    row_count, col_count = cost_table.shape
    if row_count <= 25 and col_count <= 25:
        display_table = cost_table
        print("\n2D cost table (rows=width cm, columns=length cm):")
    else:
        row_step = _choose_tick_step(list(cost_table.index))
        col_step = _choose_tick_step(list(cost_table.columns))
        display_table = cost_table.iloc[::row_step, ::col_step]
        print(
            "\n2D cost table sample "
            f"(full grid: {row_count} widths x {col_count} lengths, "
            f"showing every {row_step} width step and every {col_step} length step):"
        )

    with np.printoptions(suppress=True):
        print(display_table.to_string(float_format=lambda value: f"{value:7.1f}"))


def export_cost_table_csv(cost_table, output_path):
    if cost_table is None or not output_path:
        return

    cost_table.to_csv(output_path, index_label="width_cm")
    print(f"\nSaved full 2D cost table to: {output_path}")


def _choose_tick_step(values):
    if len(values) <= 12:
        return 1
    if len(values) <= 30:
        return 2
    if len(values) <= 60:
        return 5
    return 10


def _format_cost_label(value):
    if not math.isfinite(value):
        return ""
    return f"{int(round(value))}"


def _annotation_font_size(row_count, col_count):
    largest_dimension = max(row_count, col_count)
    if largest_dimension <= 10:
        return 10
    if largest_dimension <= 16:
        return 9
    if largest_dimension <= 25:
        return 8
    if largest_dimension <= 40:
        return 6
    return 5


def plot_cost_heatmap(long_lengths, widths, cost_grid):
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib is required to plot the cost heatmap. Install it with: pip install matplotlib")
        return

    masked_grid = np.ma.masked_invalid(cost_grid)
    finite_mask = np.isfinite(cost_grid)
    if not finite_mask.any():
        print("No feasible points to plot.")
        return

    best_index = np.unravel_index(np.argmin(np.where(finite_mask, cost_grid, np.inf)), cost_grid.shape)
    best_width = widths[best_index[0]]
    best_length = long_lengths[best_index[1]]
    best_cost = cost_grid[best_index]

    figure, axis = plt.subplots(figsize=(14, 8))
    image = axis.imshow(
        masked_grid,
        origin="lower",
        aspect="auto",
        interpolation="nearest",
        cmap="RdYlGn_r",
        extent=(
            long_lengths[0] - 0.5,
            long_lengths[-1] + 0.5,
            widths[0] - 0.5,
            widths[-1] + 0.5,
        ),
    )
    colorbar = figure.colorbar(image, ax=axis, pad=0.02)
    colorbar.set_label("Total material cost (CZK)")

    axis.scatter(
        best_length,
        best_width,
        marker="*",
        s=220,
        color="black",
        edgecolors="white",
        linewidths=0.8,
        zorder=3,
    )
    axis.annotate(
        f"Best: {best_length} x {best_width} cm\n{int(round(best_cost))} CZK",
        xy=(best_length, best_width),
        xytext=(12, 12),
        textcoords="offset points",
        bbox={"boxstyle": "round,pad=0.25", "fc": "white", "ec": "black", "alpha": 0.85},
    )

    row_count, col_count = cost_grid.shape
    font_size = _annotation_font_size(row_count, col_count)
    norm = image.norm
    cmap = image.cmap
    for row_index, width in enumerate(widths):
        for col_index, long_length in enumerate(long_lengths):
            value = cost_grid[row_index, col_index]
            if not math.isfinite(value):
                continue

            rgba = cmap(norm(value))
            luminance = 0.2126 * rgba[0] + 0.7152 * rgba[1] + 0.0722 * rgba[2]
            text_color = "black" if luminance > 0.55 else "white"
            axis.text(
                long_length,
                width,
                _format_cost_label(value),
                ha="center",
                va="center",
                fontsize=font_size,
                color=text_color,
                zorder=4,
            )

    x_step = _choose_tick_step(long_lengths)
    y_step = _choose_tick_step(widths)
    axis.set_xticks(long_lengths[::x_step])
    axis.set_yticks(widths[::y_step])
    axis.set_xlabel("Bed length (cm)")
    axis.set_ylabel("Bed width (cm)")
    axis.set_title("Raised Garden Bed 2D Cost Heatmap with Cell Values")
    axis.grid(False)

    figure.tight_layout()
    plt.show()


def parse_args():
    parser = argparse.ArgumentParser(description="Raised garden bed optimizer in 2D.")
    parser.add_argument("--length-start", type=int, default=DEFAULT_LENGTH_START)
    parser.add_argument("--length-end", type=int, default=DEFAULT_LENGTH_END)
    parser.add_argument("--length-step", type=int, default=DEFAULT_STEP)
    parser.add_argument("--width-start", type=int, default=DEFAULT_WIDTH_START)
    parser.add_argument("--width-end", type=int, default=DEFAULT_WIDTH_END)
    parser.add_argument("--width-step", type=int, default=DEFAULT_STEP)
    parser.add_argument("--stock-length", type=float, default=200)
    parser.add_argument("--stock-cost", type=float, default=423.5*2/5)
    parser.add_argument("--saw-kerf", type=float, default=0.5)
    parser.add_argument("--workers", type=int, default=max(1, (os.cpu_count() or 1) - 1))
    parser.add_argument("--csv", type=str, default=None)
    parser.add_argument("--no-progress", action="store_true")
    parser.add_argument("--no-plot", action="store_true")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    print(
        "Running 2D raised garden bed cost scan "
        f"for lengths {args.length_start}-{args.length_end} cm "
        f"and widths {args.width_start}-{args.width_end} cm."
    )
    long_lengths, widths, cost_grid = evaluate_cost_grid(
        length_start=args.length_start,
        length_end=args.length_end,
        length_step=args.length_step,
        width_start=args.width_start,
        width_end=args.width_end,
        width_step=args.width_step,
        stock_length=args.stock_length,
        stock_cost=args.stock_cost,
        saw_kerf=args.saw_kerf,
        max_workers=args.workers,
        show_progress=not args.no_progress,
    )

    cost_table = build_cost_table(long_lengths, widths, cost_grid)
    print_cost_table(cost_table)
    export_cost_table_csv(cost_table, args.csv)
    print()
    summarize_cost_grid(long_lengths, widths, cost_grid)

    if not args.no_plot:
        plot_cost_heatmap(long_lengths, widths, cost_grid)
