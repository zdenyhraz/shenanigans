import sys
from functools import lru_cache

sys.setrecursionlimit(10000)


def calculate_used_length(total_length, total_items, saw_kerf, stock_length):
    if total_items == 0:
        return 0

    used_without_end_waste = total_length + saw_kerf * max(0, total_items - 1)
    if abs(used_without_end_waste - stock_length) < 1e-9:
        return used_without_end_waste

    return total_length + saw_kerf * total_items


def find_optimal_cutting_plan(materials_data, verbose=True):
    full_results = {}
    grand_total_cost = 0

    if verbose:
        print("Starting optimization... This may take a moment for complex plans.")

    for material in materials_data:
        name = material["name"]
        stock_length = material["stock_length"]
        stock_cost = material["stock_cost"]
        saw_kerf = material["saw_kerf"]
        required_cuts = material["required_cuts"]

        if verbose:
            print(f"\nProcessing material: {name}")

        # Build a compact representation of required cuts by distinct length.
        distinct_lengths = sorted(required_cuts.keys(), reverse=True)
        demands = [required_cuts[length] for length in distinct_lengths]

        for length in distinct_lengths:
            if length > stock_length:
                raise ValueError(
                    f"Error: A required cut ({length}) is longer than the stock length ({stock_length}) for material '{name}'."
                )

        patterns = []

        def generate_patterns(index, current_pattern, total_length, total_items):
            if index == len(distinct_lengths):
                if total_items == 0:
                    return
                used_length = calculate_used_length(total_length, total_items, saw_kerf, stock_length)
                if used_length <= stock_length:
                    patterns.append(tuple(current_pattern))
                return

            length = distinct_lengths[index]
            max_count = demands[index]
            for count in range(max_count + 1):
                current_pattern[index] = count
                new_total_length = total_length + count * length
                new_total_items = total_items + count
                if new_total_items == 0:
                    generate_patterns(index + 1, current_pattern, new_total_length, new_total_items)
                else:
                    used_length = calculate_used_length(
                        new_total_length,
                        new_total_items,
                        saw_kerf,
                        stock_length,
                    )
                    if used_length <= stock_length:
                        generate_patterns(index + 1, current_pattern, new_total_length, new_total_items)
                    else:
                        break
            current_pattern[index] = 0

        generate_patterns(0, [0] * len(distinct_lengths), 0, 0)

        if not patterns:
            raise ValueError(
                f"Error: Could not generate any valid cutting patterns for material '{name}'"
            )

        patterns.sort(
            key=lambda pat: (
                -sum(pat),
                -sum(pat[i] * distinct_lengths[i] for i in range(len(distinct_lengths)))
            )
        )

        choice = {}

        @lru_cache(None)
        def min_stock_count(state):
            if all(v == 0 for v in state):
                return 0

            best = float('inf')
            for pat in patterns:
                if all(state[i] >= pat[i] for i in range(len(state))):
                    new_state = tuple(state[i] - pat[i] for i in range(len(state)))
                    count = min_stock_count(new_state)
                    if count + 1 < best:
                        best = count + 1
                        choice[state] = pat
                        if best == 1:
                            break
            return best

        initial_state = tuple(demands)
        best_stock_count = min_stock_count(initial_state)

        if best_stock_count == float('inf'):
            if verbose:
                print(f"Could not find a solution for {name}.")
            continue

        best_solution = {"plan": [], "stock_count": best_stock_count}
        state = initial_state
        while any(v > 0 for v in state):
            pat = choice.get(state)
            if pat is None:
                raise RuntimeError(
                    f"Internal error reconstructing cutting plan for material '{name}'"
                )
            stock_piece = []
            for i, count in enumerate(pat):
                stock_piece.extend([distinct_lengths[i]] * count)
            best_solution["plan"].append(stock_piece)
            state = tuple(state[i] - pat[i] for i in range(len(state)))

        # --- Process and format the final results for this material ---
        plan_details = []
        if best_solution["plan"] is None:
            if verbose:
                print(f"Could not find a solution for {name}.")
            continue

        for i, stock_piece_cuts in enumerate(best_solution["plan"]):
            # stock_piece_cuts already contains original lengths (no kerf added)
            original_lengths = stock_piece_cuts
            total_cut_length = sum(stock_piece_cuts)
            total_items = len(stock_piece_cuts)
            total_cut_length_with_kerf = calculate_used_length(
                total_cut_length,
                total_items,
                saw_kerf,
                stock_length,
            )
            waste = stock_length - total_cut_length_with_kerf
            plan_details.append({
                "stock_piece_number": i + 1,
                "cuts_to_make": sorted(original_lengths, reverse=True),
                "total_length_used": round(total_cut_length_with_kerf, 2),
                "waste": round(waste, 2)
            })

        material_total_cost = best_solution["stock_count"] * stock_cost
        grand_total_cost += material_total_cost

        full_results[name] = {
            "total_stock_pieces_needed": best_solution["stock_count"],
            "material_total_cost": material_total_cost,
            "stock_length": stock_length,
            "saw_kerf": saw_kerf,
            "cutting_plan": plan_details
        }
        if verbose:
            print(f"Optimal plan found for {name} using {best_solution['stock_count']} stock piece(s).")

    full_results["grand_total_cost"] = grand_total_cost
    if verbose:
        print("\nOptimization complete.")
    return full_results


def print_cutting_plan(results):
    """Formats and prints the results in a user-friendly way."""
    if not results:
        print("No results to display.")
        return

    print("\n-----------------------------------------")
    print("      *** OPTIMAL CUTTING PLAN ***")
    print("-----------------------------------------")

    for name, data in results.items():
        if name == "grand_total_cost":
            continue

        print("-----------------------------------------")
        print(f"\n## Material: {name}")
        print(f" > Celkem potřeba: {data['total_stock_pieces_needed']} ks")
        print(f" > Délka jednoho kusu: {data['stock_length']} cm")
        print(f" > Náklady na materiál: {data['material_total_cost']} Kč")
        print("\n   Řezný plán:")
        for piece in data["cutting_plan"]:
            print(f"   - Hranol č. {piece['stock_piece_number']}:")
            print(f"     Nařežte díly (délky v cm): {piece['cuts_to_make']}")
            print(f"     Celková využitá délka (vč. prořezů): {piece['total_length_used']} cm")
            print(f"     Zbytek: {piece['waste']} cm")

    print("\n-----------------------------------------")
    print(f"### CELKOVÉ NÁKLADY: {results.get('grand_total_cost', 0)} Kč ###")
    print("-----------------------------------------")


if __name__ == '__main__':
    user_data = [
        {
            "name": "Hranol 40x120",
            "stock_length": 500,
            "stock_cost": 423.5,
            "saw_kerf": 0.3,
            "required_cuts": {
                140: 16,
                132: 12,
                90: 16,
                82: 12,
            }
        },
    ]

    final_plan = find_optimal_cutting_plan(user_data)
    print_cutting_plan(final_plan)
