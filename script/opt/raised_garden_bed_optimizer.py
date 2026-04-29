import argparse
import math
import os
import sys
from concurrent.futures import ProcessPoolExecutor, as_completed
from itertools import combinations_with_replacement

import numpy as np

from script.opt.cutting_plan import find_fixed_stock_plan

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
DEFAULT_BEAM_LENGTH = 400.0
DEFAULT_BEAM_THICKNESS = 6
DEFAULT_BEAM_COST = 493.6
DEFAULT_TRANSPORT_PATTERNS = (
    # (200.0, 200.0, 100.0),
    # (200.0, 150.0, 150.0),
    (200.0, 200.0),
)
DEFAULT_MILL_KERF = 0.0


def build_required_cuts(long_length, width_length):
    required_cuts = {}

    def add_cut(length, count):
        required_cuts[length] = required_cuts.get(length, 0) + count

    add_cut(long_length, NUM_BEDS*8)
    add_cut(long_length - 2*DEFAULT_BEAM_THICKNESS, NUM_BEDS*6)
    add_cut(width_length, NUM_BEDS*8)
    add_cut(width_length - 2*DEFAULT_BEAM_THICKNESS, NUM_BEDS*6)

    return required_cuts


def _transport_pattern_used_length(pattern, mill_kerf):
    if not pattern:
        return 0.0
    return sum(pattern) + mill_kerf * max(0, len(pattern) - 1)


def normalize_transport_patterns(transport_patterns):
    normalized = []
    for pattern in transport_patterns:
        normalized_pattern = tuple(sorted((float(length) for length in pattern), reverse=True))
        if not normalized_pattern:
            raise ValueError("Transport patterns cannot be empty.")
        normalized.append(normalized_pattern)
    return tuple(normalized)


def validate_transport_patterns(transport_patterns, beam_length, mill_kerf):
    for pattern in transport_patterns:
        if any(length <= 0 for length in pattern):
            raise ValueError(f"Invalid transport pattern {pattern}: all lengths must be positive.")

        used_length = _transport_pattern_used_length(pattern, mill_kerf)
        if used_length > beam_length + 1e-9:
            raise ValueError(
                f"Transport pattern {pattern} needs {used_length:.2f} cm including mill kerf, "
                f"which exceeds the beam length of {beam_length:.2f} cm."
            )


def _expand_transport_patterns(pattern_indexes, transport_patterns):
    stock_piece_lengths = []
    pattern_usage = [0] * len(transport_patterns)
    for pattern_index in pattern_indexes:
        pattern_usage[pattern_index] += 1
        stock_piece_lengths.extend(transport_patterns[pattern_index])
    return tuple(sorted(stock_piece_lengths, reverse=True)), tuple(pattern_usage)


def _pattern_mix_label(pattern_usage, transport_patterns):
    parts = []
    for usage_count, pattern in zip(pattern_usage, transport_patterns):
        if usage_count == 0:
            continue
        pattern_label = "+".join(f"{length / 100:g}m" for length in pattern)
        parts.append(f"{usage_count}x[{pattern_label}]")
    return ", ".join(parts) if parts else "none"


def find_transport_constrained_cutting_plan(
    required_cuts,
    beam_length=DEFAULT_BEAM_LENGTH,
    beam_cost=DEFAULT_BEAM_COST,
    saw_kerf=0.5,
    transport_patterns=DEFAULT_TRANSPORT_PATTERNS,
    mill_kerf=DEFAULT_MILL_KERF,
):
    normalized_patterns = normalize_transport_patterns(transport_patterns)
    validate_transport_patterns(normalized_patterns, beam_length, mill_kerf)

    total_required_length = sum(length * count for length, count in required_cuts.items())
    minimum_beam_count = max(1, math.ceil(total_required_length / beam_length))
    maximum_beam_count = sum(required_cuts.values())

    best_solution = None
    for beam_count in range(minimum_beam_count, maximum_beam_count + 1):
        for pattern_indexes in combinations_with_replacement(range(len(normalized_patterns)), beam_count):
            stock_piece_lengths, pattern_usage = _expand_transport_patterns(pattern_indexes, normalized_patterns)
            fixed_stock_plan = find_fixed_stock_plan(required_cuts, stock_piece_lengths, saw_kerf)
            if fixed_stock_plan is None:
                continue

            solution = {
                "beam_count": beam_count,
                "beam_cost": beam_cost,
                "grand_total_cost": beam_count * beam_cost,
                "transport_patterns": normalized_patterns,
                "transport_pattern_usage": pattern_usage,
                "transport_pattern_summary": _pattern_mix_label(pattern_usage, normalized_patterns),
                "transport_stock_pieces": stock_piece_lengths,
                "used_stock_pieces": fixed_stock_plan["used_stock_pieces"],
                "total_waste": fixed_stock_plan["total_waste"],
                "cutting_plan": fixed_stock_plan["cutting_plan"],
            }
            if best_solution is None or (
                solution["grand_total_cost"],
                solution["total_waste"],
                solution["used_stock_pieces"],
            ) < (
                best_solution["grand_total_cost"],
                best_solution["total_waste"],
                best_solution["used_stock_pieces"],
            ):
                best_solution = solution

        if best_solution is not None:
            return best_solution

    return None


def solve_configuration(
    long_length,
    width_length,
    beam_length=DEFAULT_BEAM_LENGTH,
    beam_cost=DEFAULT_BEAM_COST,
    saw_kerf=0.5,
    transport_patterns=DEFAULT_TRANSPORT_PATTERNS,
    mill_kerf=DEFAULT_MILL_KERF,
):
    required_cuts = build_required_cuts(long_length, width_length)
    result = find_transport_constrained_cutting_plan(
        required_cuts,
        beam_length=beam_length,
        beam_cost=beam_cost,
        saw_kerf=saw_kerf,
        transport_patterns=transport_patterns,
        mill_kerf=mill_kerf,
    )
    return result


def evaluate_configuration(
    long_length,
    width_length,
    beam_length=DEFAULT_BEAM_LENGTH,
    beam_cost=DEFAULT_BEAM_COST,
    saw_kerf=0.5,
    transport_patterns=DEFAULT_TRANSPORT_PATTERNS,
    mill_kerf=DEFAULT_MILL_KERF,
):
    result = solve_configuration(
        long_length,
        width_length,
        beam_length=beam_length,
        beam_cost=beam_cost,
        saw_kerf=saw_kerf,
        transport_patterns=transport_patterns,
        mill_kerf=mill_kerf,
    )
    if result is None:
        return math.inf
    return result.get("grand_total_cost", math.inf)


def _evaluate_single_configuration(task):
    long_length, width_length, beam_length, beam_cost, saw_kerf, transport_patterns, mill_kerf = task
    try:
        cost = evaluate_configuration(
            long_length,
            width_length,
            beam_length=beam_length,
            beam_cost=beam_cost,
            saw_kerf=saw_kerf,
            transport_patterns=transport_patterns,
            mill_kerf=mill_kerf,
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
    beam_length=DEFAULT_BEAM_LENGTH,
    beam_cost=DEFAULT_BEAM_COST,
    saw_kerf=0.5,
    transport_patterns=DEFAULT_TRANSPORT_PATTERNS,
    mill_kerf=DEFAULT_MILL_KERF,
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
            beam_length=beam_length,
            beam_cost=beam_cost,
            saw_kerf=saw_kerf,
            transport_patterns=transport_patterns,
            mill_kerf=mill_kerf,
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
    beam_length=DEFAULT_BEAM_LENGTH,
    beam_cost=DEFAULT_BEAM_COST,
    saw_kerf=0.5,
    transport_patterns=DEFAULT_TRANSPORT_PATTERNS,
    mill_kerf=DEFAULT_MILL_KERF,
    max_workers=None,
    show_progress=True,
):
    transport_patterns = normalize_transport_patterns(transport_patterns)
    validate_transport_patterns(transport_patterns, beam_length, mill_kerf)

    long_lengths = list(range(length_start, length_end + 1, length_step))
    widths = list(range(width_start, width_end + 1, width_step))
    tasks = [
        (long_length, width_length, beam_length, beam_cost, saw_kerf, transport_patterns, mill_kerf)
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


def print_selected_cutting_plan(long_length, width_length, result):
    print("\nSelected configuration cutting plan:")
    print(f"Bed size: {long_length} x {width_length} cm")

    if result is None:
        print("No feasible plan found for this configuration.")
        return

    print(f"Total cost: {result['grand_total_cost']:.1f} CZK")
    print(f"5 m beams needed: {result['beam_count']} at {result['beam_cost']:.1f} CZK each")
    print(f"Transport pattern mix: {result['transport_pattern_summary']}")
    print(f"Transportable stock pieces used: {result['used_stock_pieces']}")
    print(f"Total waste after final cuts: {result['total_waste']:.2f} cm")
    print("\nCutting plan:")
    for piece in result["cutting_plan"]:
        print(
            f"Piece {piece['stock_piece_number']:>2} "
            f"({piece['stock_length']:.0f} cm stock) -> "
            f"cuts {piece['cuts_to_make']} | "
            f"used {piece['total_length_used']:.2f} cm | "
            f"waste {piece['waste']:.2f} cm"
        )


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


def parse_transport_pattern(text):
    try:
        pattern = tuple(float(part.strip()) for part in text.split(",") if part.strip())
    except ValueError as error:
        raise argparse.ArgumentTypeError(
            f"Invalid transport pattern '{text}'. Use comma-separated lengths in cm, for example 200,200,100."
        ) from error

    if not pattern:
        raise argparse.ArgumentTypeError("Transport patterns cannot be empty.")

    return pattern


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
    parser.add_argument("--beam-length", type=float, default=DEFAULT_BEAM_LENGTH)
    parser.add_argument("--beam-cost", type=float, default=DEFAULT_BEAM_COST)
    parser.add_argument("--saw-kerf", type=float, default=0.5)
    parser.add_argument("--mill-kerf", type=float, default=DEFAULT_MILL_KERF)
    parser.add_argument("--plan-length", type=int, default=None)
    parser.add_argument("--plan-width", type=int, default=None)
    parser.add_argument(
        "--transport-pattern",
        dest="transport_patterns",
        action="append",
        type=parse_transport_pattern,
        help="Allowed mill pre-cut pattern in cm, for example --transport-pattern 200,200,100",
    )
    parser.add_argument("--workers", type=int, default=max(1, (os.cpu_count() or 1) - 1))
    parser.add_argument("--csv", type=str, default=None)
    parser.add_argument("--no-progress", action="store_true")
    parser.add_argument("--no-plot", action="store_true")
    parser.add_argument("--plan-only", action="store_true")
    args = parser.parse_args()
    if not args.transport_patterns:
        args.transport_patterns = list(DEFAULT_TRANSPORT_PATTERNS)
    if (args.plan_length is None) != (args.plan_width is None):
        parser.error("--plan-length and --plan-width must be provided together.")
    if args.plan_only and (args.plan_length is None or args.plan_width is None):
        parser.error("--plan-only requires --plan-length and --plan-width.")
    return args


if __name__ == "__main__":
    args = parse_args()

    if args.plan_only:
        selected_result = solve_configuration(
            args.plan_length,
            args.plan_width,
            beam_length=args.beam_length,
            beam_cost=args.beam_cost,
            saw_kerf=args.saw_kerf,
            transport_patterns=args.transport_patterns,
            mill_kerf=args.mill_kerf,
        )
        print_selected_cutting_plan(args.plan_length, args.plan_width, selected_result)
        sys.exit(0)

    print(
        "Running 2D raised garden bed cost scan "
        f"for lengths {args.length_start}-{args.length_end} cm "
        f"and widths {args.width_start}-{args.width_end} cm "
        f"using beam length {args.beam_length:.1f} cm "
        f"and transport patterns {args.transport_patterns}."
    )
    long_lengths, widths, cost_grid = evaluate_cost_grid(
        length_start=args.length_start,
        length_end=args.length_end,
        length_step=args.length_step,
        width_start=args.width_start,
        width_end=args.width_end,
        width_step=args.width_step,
        beam_length=args.beam_length,
        beam_cost=args.beam_cost,
        saw_kerf=args.saw_kerf,
        transport_patterns=args.transport_patterns,
        mill_kerf=args.mill_kerf,
        max_workers=args.workers,
        show_progress=not args.no_progress,
    )

    cost_table = build_cost_table(long_lengths, widths, cost_grid)
    print_cost_table(cost_table)
    export_cost_table_csv(cost_table, args.csv)
    print()
    summarize_cost_grid(long_lengths, widths, cost_grid)

    if args.plan_length is not None and args.plan_width is not None:
        selected_result = solve_configuration(
            args.plan_length,
            args.plan_width,
            beam_length=args.beam_length,
            beam_cost=args.beam_cost,
            saw_kerf=args.saw_kerf,
            transport_patterns=args.transport_patterns,
            mill_kerf=args.mill_kerf,
        )
        print_selected_cutting_plan(args.plan_length, args.plan_width, selected_result)

    if not args.no_plot:
        plot_cost_heatmap(long_lengths, widths, cost_grid)
