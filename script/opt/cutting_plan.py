import collections
import sys

sys.setrecursionlimit(2000)


def find_optimal_cutting_plan(materials_data):
    full_results = {}
    grand_total_cost = 0

    print("Starting optimization... This may take a moment for complex plans.")

    for material in materials_data:
        name = material["name"]
        stock_length = material["stock_length"]
        stock_cost = material["stock_cost"]
        saw_kerf = material["saw_kerf"]
        required_cuts = material["required_cuts"]

        print(f"\nProcessing material: {name}")

        # Create a flat list of all individual cuts needed.
        # The space consumed by a cut is its length plus the kerf.
        all_cuts_with_kerf = []
        for length, quantity in required_cuts.items():
            cut_size = length + saw_kerf
            if cut_size > stock_length:
                raise ValueError(
                    f"Error: A required cut ({length} + kerf {saw_kerf}) is "
                    f"longer than the stock length ({stock_length}) for material '{name}'."
                )
            all_cuts_with_kerf.extend([cut_size] * quantity)

        # Sort cuts from largest to smallest. This is the "First Fit Decreasing"
        # heuristic, which dramatically speeds up the search for the optimal
        # solution by finding good solutions early and pruning bad branches.
        all_cuts_with_kerf.sort(reverse=True)

        # This dictionary will hold the best solution found so far.
        # It's a mutable object that can be modified by the recursive function.
        best_solution = {"plan": None, "stock_count": float('inf')}

        def backtrack_solver(cuts_to_place, current_plan):
            # If a plan is found that is already worse than our best known solution,
            # prune this entire branch of the search tree.
            if len(current_plan) >= best_solution["stock_count"]:
                return

            # If all cuts have been placed, we have found a complete, valid plan.
            if not cuts_to_place:
                # If this plan is the best one found so far, save it.
                if len(current_plan) < best_solution["stock_count"]:
                    best_solution["stock_count"] = len(current_plan)
                    # Save a deep copy of the plan
                    best_solution["plan"] = [list(p) for p in current_plan]
                return

            cut_to_place = cuts_to_place[0]
            remaining_cuts = cuts_to_place[1:]

            # Try to place the current cut onto an existing stock piece
            for i in range(len(current_plan)):
                if sum(current_plan[i]) + cut_to_place <= stock_length:
                    current_plan[i].append(cut_to_place)
                    backtrack_solver(remaining_cuts, current_plan)
                    current_plan[i].pop()  # Backtrack

            # Try to place the current cut onto a new stock piece
            current_plan.append([cut_to_place])
            backtrack_solver(remaining_cuts, current_plan)
            current_plan.pop()  # Backtrack

        # Initial call to start the backtracking search
        backtrack_solver(all_cuts_with_kerf, [])

        # --- Process and format the final results for this material ---
        plan_details = []
        if best_solution["plan"] is None:
            print(f"Could not find a solution for {name}.")
            continue

        for i, stock_piece_cuts in enumerate(best_solution["plan"]):
            # Revert from "cut_size" back to original length for display
            original_lengths = [round(c - saw_kerf, 2) for c in stock_piece_cuts]
            total_cut_length_with_kerf = sum(stock_piece_cuts)
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
        print(f"Optimal plan found for {name} using {best_solution['stock_count']} stock piece(s).")

    full_results["grand_total_cost"] = grand_total_cost
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
            "name": "Hranol 80x80",
            "stock_length": 500,
            "stock_cost": 484,
            "saw_kerf": 1,
            "required_cuts": {
                101.5: 2,
                84.5: 2,
                67.5: 2,
                25: 6,
                32: 4,
                22: 4,
                12: 4
            }
        },
        {
            "name": "Hranol 18x82",
            "stock_length": 300,
            "stock_cost": 91,
            "saw_kerf": 1,
            "required_cuts": {
                25: 24
            }
        }
    ]

    final_plan = find_optimal_cutting_plan(user_data)
    print_cutting_plan(final_plan)
