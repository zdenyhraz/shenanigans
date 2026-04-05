import math
import sys
from script.opt.cutting_plan import find_optimal_cutting_plan

sys.setrecursionlimit(1000)


def build_user_data(long_length, short_length, stock_length=500, stock_cost=411.5, saw_kerf=0.3):
    return [
        {
            "name": f"Raised garden bed: {long_length}cm",
            "stock_length": stock_length,
            "stock_cost": stock_cost,
            "saw_kerf": saw_kerf,
            "required_cuts": {
                long_length: 10,
                short_length: 6,
                80: 8,
                88: 8,
            },
        }
    ]


def evaluate_costs(length_start=200, length_end=300, step=1):
    results = []
    for long_length in range(length_start, length_end + 1, step):
        short_length = long_length - 8
        user_data = build_user_data(long_length, short_length)
        result = find_optimal_cutting_plan(user_data)
        cost = result.get("grand_total_cost", math.inf)
        results.append((long_length, short_length, cost))
    return results


def find_price_jumps(points):
    jumps = []
    for i in range(1, len(points)):
        prev_cost = points[i - 1][2]
        cost = points[i][2]
        if cost != prev_cost:
            jumps.append((points[i][0], points[i][1], prev_cost, cost, cost - prev_cost))
    return jumps


def plot_cost_curve(points):
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib is required to plot the cost curve. Install it with: pip install matplotlib")
        return

    x = [p[0] for p in points]
    y = [p[2] for p in points]

    plt.figure(figsize=(12, 6))
    plt.plot(x, y, marker="o", linestyle="-", color="tab:blue")
    plt.title("Raised Garden Bed: Stock Cost vs. Long Piece Length")
    plt.xlabel("Long piece length (cm)")
    plt.ylabel("Total material cost (Kč)")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    print("Running raised garden bed cost scan from 200 to 300 cm.")
    points = evaluate_costs(200, 300, 1)

    print("\nResults summary:")
    for long_length, short_length, cost in points:
        print(f"Long={long_length} cm, Short={short_length} cm -> Cost={cost}")

    jumps = find_price_jumps(points)
    print("\nDetected cost jumps:")
    if not jumps:
        print("No cost jumps detected in the scanned range.")
    else:
        for long_length, short_length, prev_cost, cost, diff in jumps:
            print(
                f"At long={long_length} cm (short={short_length} cm), cost changed from {prev_cost} to {cost} (Δ={diff})"
            )

    plot_cost_curve(points)
