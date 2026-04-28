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


def build_feasible_patterns(stock_length, distinct_lengths, max_demands, saw_kerf, allow_empty=False):
    patterns = []

    def generate_patterns(index, current_pattern, total_length, total_items):
        if index == len(distinct_lengths):
            if total_items == 0:
                if allow_empty:
                    patterns.append(tuple(current_pattern))
                return

            used_length = calculate_used_length(total_length, total_items, saw_kerf, stock_length)
            if used_length <= stock_length:
                patterns.append(tuple(current_pattern))
            return

        length = distinct_lengths[index]
        max_count = max_demands[index]
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
    patterns.sort(
        key=lambda pat: (
            sum(pat) == 0,
            -sum(pat),
            -sum(pat[i] * distinct_lengths[i] for i in range(len(distinct_lengths))),
        )
    )
    return patterns


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

        patterns = build_feasible_patterns(
            stock_length,
            distinct_lengths,
            demands,
            saw_kerf,
            allow_empty=False,
        )

        if not patterns:
            raise ValueError(
                f"Error: Could not generate any valid cutting patterns for material '{name}'"
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


def find_fixed_stock_plan(required_cuts, stock_piece_lengths, saw_kerf):
    import numpy as np
    from scipy.optimize import Bounds, LinearConstraint, milp

    if not stock_piece_lengths:
        return None

    distinct_lengths = sorted(required_cuts.keys(), reverse=True)
    demands = tuple(required_cuts[length] for length in distinct_lengths)

    for length in distinct_lengths:
        if length > max(stock_piece_lengths):
            return None

    stock_length_types = tuple(sorted(set(stock_piece_lengths), reverse=True))
    stock_length_counts = tuple(
        stock_piece_lengths.count(stock_length)
        for stock_length in stock_length_types
    )
    patterns_by_stock_length = {}
    for stock_length in stock_length_types:
        patterns = build_feasible_patterns(
            stock_length,
            distinct_lengths,
            demands,
            saw_kerf,
            allow_empty=False,
        )
        if not patterns:
            continue
        patterns_by_stock_length[stock_length] = patterns

    variables = []
    for stock_type_index, stock_length in enumerate(stock_length_types):
        for pattern in patterns_by_stock_length.get(stock_length, []):
            cut_count = sum(pattern)
            cuts_to_make = []
            for pattern_index, count in enumerate(pattern):
                cuts_to_make.extend([distinct_lengths[pattern_index]] * count)

            total_cut_length = sum(cuts_to_make)
            used_length = calculate_used_length(
                total_cut_length,
                cut_count,
                saw_kerf,
                stock_length,
            )
            waste = stock_length - used_length
            variables.append({
                "stock_type_index": stock_type_index,
                "stock_length": stock_length,
                "pattern": pattern,
                "cuts_to_make": tuple(sorted(cuts_to_make, reverse=True)),
                "used_length": round(used_length, 2),
                "waste": round(waste, 2),
            })

    if not variables:
        return None

    demand_matrix = np.array(
        [
            [variable["pattern"][demand_index] for variable in variables]
            for demand_index in range(len(distinct_lengths))
        ],
        dtype=float,
    )
    demand_constraint = LinearConstraint(
        demand_matrix,
        lb=np.array(demands, dtype=float),
        ub=np.array(demands, dtype=float),
    )

    stock_matrix = np.array(
        [
            [
                1.0 if variable["stock_type_index"] == stock_type_index else 0.0
                for variable in variables
            ]
            for stock_type_index in range(len(stock_length_types))
        ],
        dtype=float,
    )
    stock_constraint = LinearConstraint(
        stock_matrix,
        lb=np.zeros(len(stock_length_types)),
        ub=np.array(stock_length_counts, dtype=float),
    )

    objective = np.array(
        [variable["waste"] + 1e-3 for variable in variables],
        dtype=float,
    )
    solution = milp(
        c=objective,
        constraints=[demand_constraint, stock_constraint],
        integrality=np.ones(len(variables), dtype=int),
        bounds=Bounds(
            np.zeros(len(variables), dtype=float),
            np.full(len(variables), np.inf, dtype=float),
        ),
    )

    if not solution.success or solution.x is None:
        return None

    variable_counts = np.rint(solution.x).astype(int)
    raw_plan = []
    total_waste = 0.0
    used_stock_pieces = 0
    for variable, variable_count in zip(variables, variable_counts):
        for _ in range(variable_count):
            raw_plan.append((
                variable["stock_length"],
                variable["cuts_to_make"],
                variable["used_length"],
                variable["waste"],
            ))
            used_stock_pieces += 1
            total_waste += variable["waste"]

    plan_details = []
    for piece_number, entry in enumerate(raw_plan, start=1):
        stock_length, cuts_to_make, total_length_used, waste = entry
        plan_details.append({
            "stock_piece_number": piece_number,
            "stock_length": stock_length,
            "cuts_to_make": list(cuts_to_make),
            "total_length_used": total_length_used,
            "waste": waste,
        })

    return {
        "used_stock_pieces": used_stock_pieces,
        "total_waste": round(total_waste, 2),
        "cutting_plan": plan_details,
    }


def print_cutting_plan(results):
    """Prints the cutting plan in a compact, scan-friendly format."""
    if not results:
        print("No results to display.")
        return

    print("\nOptimal cutting plan:")

    for name, data in results.items():
        if name == "grand_total_cost":
            continue

        print(f"\nMaterial: {name}")
        print(
            f"Stock pieces: {data['total_stock_pieces_needed']} x {data['stock_length']} cm | "
            f"Cost: {data['material_total_cost']:.1f} CZK | "
            f"Saw kerf: {data['saw_kerf']:.2f} cm"
        )
        print("Cutting plan:")
        for piece in data["cutting_plan"]:
            print(
                f"Piece {piece['stock_piece_number']:>2} -> "
                f"cuts {piece['cuts_to_make']} | "
                f"used {piece['total_length_used']:.2f} cm | "
                f"waste {piece['waste']:.2f} cm"
            )

    print(f"\nGrand total: {results.get('grand_total_cost', 0):.1f} CZK")


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
