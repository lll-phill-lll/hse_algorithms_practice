#include <algorithm>
#include <cstdint>
#include <iterator>
#include <random>
#include <vector>
#include "profile.h"

using namespace std;

struct Item {
    int cost = 0;
    int weight = 0;
};

struct Subset {
    int cost = 0;
    int weight = 0;
};

Subset operator+(Subset subset, Item item) {
    return {
        subset.cost + item.cost,
        subset.weight + item.weight
    };
}

struct TestCase {
    const vector<Item>& items;
    int max_weight;
};


constexpr int NO_SOLUTION_COST = numeric_limits<int>::min();


namespace Simple {

    int Try(const vector<Item>& items, int pivot_index, int max_weight, int current_cost) {
        if (max_weight < 0) {
            return NO_SOLUTION_COST;
        }
        if (pivot_index == static_cast<int>(items.size())) {
            return current_cost;
        }
        const Item pivot_item = items[pivot_index];
        int best_cost = NO_SOLUTION_COST;
        for (const bool pivot_state : {false, true}) {
            const int result = Try(
                items,
                pivot_index + 1,
                max_weight - (pivot_state ? pivot_item.weight : 0),
                current_cost + (pivot_state ? pivot_item.cost : 0)
            );
            if (result > best_cost) {
                best_cost = result;
            }
        }
        return best_cost;
    }

    int Solve(const vector<Item>& items, int max_weight) {
        return Try(items, 0, max_weight, 0);
    }

}


namespace SimpleBound {

    int Try(const vector<Item>& items, int pivot_index, int max_weight, int current_cost, int rest_cost, int best_cost) {
        if (max_weight < 0) {
            return NO_SOLUTION_COST;
        }
        if (current_cost + rest_cost < best_cost) {
            return NO_SOLUTION_COST;
        }
        if (pivot_index == static_cast<int>(items.size())) {
            return current_cost;
        }
        const Item& pivot_item = items[pivot_index];
        int local_best_cost = NO_SOLUTION_COST;
        for (const bool pivot_state : {false, true}) {
            const int result = Try(
                items,
                pivot_index + 1,
                max_weight - (pivot_state ? pivot_item.weight : 0),
                current_cost + (pivot_state ? pivot_item.cost : 0),
                rest_cost - pivot_item.cost,
                max(best_cost, local_best_cost)
            );
            if (result > local_best_cost) {
                local_best_cost = result;
            }
        }
        return local_best_cost;
    }

    int Solve(const vector<Item>& items, int max_weight) {
        const int total_cost = transform_reduce(items.begin(), items.end(), 0, plus<>{}, [](Item item) { return item.cost; });
        return Try(items, 0, max_weight, 0, total_cost, NO_SOLUTION_COST);
    }

}


namespace KolesarNoEstimation {

    int Try(const vector<Item>& items, int pivot_index, int max_weight, int current_cost, int rest_cost, int best_cost, vector<bool>& is_used) {
        if (max_weight < 0) {
            return NO_SOLUTION_COST;
        }
        if (current_cost + rest_cost < best_cost) {
            return NO_SOLUTION_COST;
        }
        if (pivot_index == static_cast<int>(items.size())) {
            return current_cost;
        }
        const Item& pivot_item = items[pivot_index];
        int local_best_cost = NO_SOLUTION_COST;
        for (const bool pivot_state : {false, true}) {
            is_used[pivot_index] = pivot_state;
            const int result = Try(
                items,
                pivot_index + 1,
                max_weight - (pivot_state ? pivot_item.weight : 0),
                current_cost + (pivot_state ? pivot_item.cost : 0),
                rest_cost - pivot_item.cost,
                max(best_cost, local_best_cost),
                is_used
            );
            if (result > local_best_cost) {
                local_best_cost = result;
            }
        }
        return local_best_cost;
    }

    int Solve(vector<Item> items, int max_weight) {
        const int item_count = items.size();
        sort(items.begin(), items.end(),
             [](Item lhs, Item rhs) {
                return static_cast<uint64_t>(lhs.cost) * rhs.weight
                    > static_cast<uint64_t>(rhs.cost) * lhs.weight;
             });
        vector<bool> is_used(item_count, false);
        const int total_cost = transform_reduce(items.begin(), items.end(), 0, plus<>{}, [](Item item) { return item.cost; });
        return Try(items, 0, max_weight, 0, total_cost, NO_SOLUTION_COST, is_used);
    }

}


namespace KolesarDFS {

    int EstimateMaxRestCost(const vector<Item>& items, int start_index, int max_weight) {
        int max_rest_cost = 0;
        for (; start_index < static_cast<int>(items.size()); ++start_index) {
            const Item item = items[start_index];
            if (item.weight <= max_weight) {
                max_rest_cost += item.cost;
                max_weight -= item.weight;
            } else {
                max_rest_cost += (static_cast<uint64_t>(item.cost) * max_weight + item.weight - 1) / item.weight;
                break;
            }
        }
        return max_rest_cost;
    }

    int Try(const vector<Item>& items, int pivot_index, int max_weight, int current_cost, int best_cost, vector<bool>& is_used) {
        if (max_weight < 0) {
            return NO_SOLUTION_COST;
        }
        if (current_cost + EstimateMaxRestCost(items, pivot_index, max_weight) < best_cost) {
            return NO_SOLUTION_COST;
        }
        if (pivot_index == static_cast<int>(items.size())) {
            return current_cost;
        }
        const Item& pivot_item = items[pivot_index];
        int local_best_cost = NO_SOLUTION_COST;
        for (const bool pivot_state : {false, true}) {
            is_used[pivot_index] = pivot_state;
            const int result = Try(
                items,
                pivot_index + 1,
                max_weight - (pivot_state ? pivot_item.weight : 0),
                current_cost + (pivot_state ? pivot_item.cost : 0),
                max(best_cost, local_best_cost),
                is_used
            );
            if (result > local_best_cost) {
                local_best_cost = result;
            }
        }
        return local_best_cost;
    }

    int Solve(vector<Item> items, int max_weight) {
        const int item_count = items.size();
        sort(items.begin(), items.end(), [](Item lhs, Item rhs) { return static_cast<uint64_t>(lhs.cost) * rhs.weight > static_cast<uint64_t>(rhs.cost) * lhs.weight; });
        vector<bool> is_used(item_count, false);
        return Try(items, 0, max_weight, 0, NO_SOLUTION_COST, is_used);
    }

}


namespace MeetInTheMiddle {

    void SupportTry(const vector<Item>& items, int pivot_index, int max_weight, Subset current_subset, vector<Subset>& subsets) {
        if (max_weight < 0) {
            return;
        }
        if (pivot_index == static_cast<int>(items.size())) {
            subsets.push_back(current_subset);
            return;
        }

        const Item item = items[pivot_index];
        for (const bool pivot_state : {false, true}) {
            SupportTry(
                items,
                pivot_index + 1,
                max_weight - (pivot_state ? item.weight : 0),
                current_subset + (pivot_state ? item : Item{}),
                subsets
            );
        }
    }

    vector<Subset> GenerateSupportSubsets(const vector<Item>& items, int max_weight) {
        LOG_DURATION("--- support subsets")
        vector<Subset> subsets;
        SupportTry(items, 0, max_weight, {0, 0}, subsets);
        sort(subsets.begin(), subsets.end(), [](Subset lhs, Subset rhs) { return lhs.weight < rhs.weight; });
        partial_sum(
            subsets.begin(), subsets.end(),
            subsets.begin(),
            [](Subset lhs, Subset rhs) {
                return Subset{max(lhs.cost, rhs.cost), rhs.weight};
            }
        );
        return subsets;
    }

    Subset FindBestSupportSubset(const vector<Subset>& support_subsets, int max_weight) {
        const auto it = upper_bound(
            support_subsets.begin(), support_subsets.end(),
            Subset{0, max_weight},
            [](Subset lhs, Subset rhs) { return lhs.weight < rhs.weight; }
        );
        if (it == support_subsets.begin()) {
            return {0, 0};
        } else {
            return *prev(it);
        }
    }

    int BaseTry(const vector<Item>& base_items, const vector<Subset>& support_subsets, int pivot_index, int max_weight, int current_cost) {
        if (max_weight < 0) {
            return NO_SOLUTION_COST;
        }
        if (pivot_index == static_cast<int>(base_items.size())) {
            return current_cost + FindBestSupportSubset(support_subsets, max_weight).cost;
        }

        int best_cost = NO_SOLUTION_COST;

        const Item item = base_items[pivot_index];
        for (const bool pivot_state : {false, true}) {
            const int result = BaseTry(
                base_items,
                support_subsets,
                pivot_index + 1,
                max_weight - (pivot_state ? item.weight : 0),
                current_cost + (pivot_state ? item.cost : 0)
            );
            best_cost = max(best_cost, result);
        }

        return best_cost;
    }

    int Solve(const vector<Item>& items, int max_weight) {
        const int item_count = items.size();

        const vector base_items(items.begin(), items.begin() + item_count / 2);
        const vector support_items(items.begin() + item_count / 2, items.end());

        const vector<Subset> support_subsets = GenerateSupportSubsets(support_items, max_weight);

        LOG_DURATION("--- base bt")
        return BaseTry(base_items, support_subsets, 0, max_weight, 0);
    }

}


namespace MeetInTheMiddleOptimized {

    void SupportTry(const vector<Item>& items, int pivot_index, int max_weight, Subset current_subset, vector<Subset>& subsets) {
        if (max_weight < 0) {
            return;
        }
        if (pivot_index == static_cast<int>(items.size())) {
            subsets.push_back(current_subset);
            return;
        }

        const Item item = items[pivot_index];
        for (const bool pivot_state : {false, true}) {
            SupportTry(
                items,
                pivot_index + 1,
                max_weight - (pivot_state ? item.weight : 0),
                current_subset + (pivot_state ? item : Item{}),
                subsets
            );
        }
    }

    vector<Subset> GenerateSupportSubsets(const vector<Item>& items, int max_weight) {
        LOG_DURATION("--- support subsets")
        vector<Subset> subsets;
        SupportTry(items, 0, max_weight, {0, 0}, subsets);
        sort(subsets.begin(), subsets.end(), [](Subset lhs, Subset rhs) { return lhs.weight < rhs.weight; });
        partial_sum(
            subsets.begin(), subsets.end(),
            subsets.begin(),
            [](Subset lhs, Subset rhs) {
                return Subset{max(lhs.cost, rhs.cost), rhs.weight};
            }
        );
        return subsets;
    }

    Subset FindBestSupportSubset(const vector<Subset>& support_subsets, int max_weight) {
        const auto it = upper_bound(
            support_subsets.begin(), support_subsets.end(),
            Subset{0, max_weight},
            [](Subset lhs, Subset rhs) { return lhs.weight < rhs.weight; }
        );
        if (it == support_subsets.begin()) {
            return {0, 0};
        } else {
            return *prev(it);
        }
    }

    int EstimateMaxRestCost(const vector<Item>& base_items, const vector<Subset>& support_subsets, int start_index, int max_weight) {
        int max_rest_cost = 0;
        for (; start_index < static_cast<int>(base_items.size()); ++start_index) {
            const Item item = base_items[start_index];
            if (item.weight <= max_weight) {
                max_rest_cost += item.cost;
                max_weight -= item.weight;
            } else {
                max_rest_cost += (static_cast<uint64_t>(item.cost) * max_weight + item.weight - 1) / item.weight;
                return max_rest_cost;
            }
        }
        return max_rest_cost + FindBestSupportSubset(support_subsets, max_weight).cost;
    }

    int BaseTry(const vector<Item>& base_items, const vector<Subset>& support_subsets, int pivot_index, int max_weight, int current_cost, int best_cost) {
        if (max_weight < 0) {
            return NO_SOLUTION_COST;
        }
        if (current_cost + EstimateMaxRestCost(base_items, support_subsets, pivot_index, max_weight) < best_cost) {
            return NO_SOLUTION_COST;
        }
        if (pivot_index == static_cast<int>(base_items.size())) {
            return current_cost + FindBestSupportSubset(support_subsets, max_weight).cost;
        }

        int local_best_cost = NO_SOLUTION_COST;

        const Item item = base_items[pivot_index];
        for (const bool pivot_state : {false, true}) {
            const int result = BaseTry(
                base_items,
                support_subsets,
                pivot_index + 1,
                max_weight - (pivot_state ? item.weight : 0),
                current_cost + (pivot_state ? item.cost : 0),
                max(best_cost, local_best_cost)
            );
            local_best_cost = max(local_best_cost, result);
        }

        return local_best_cost;
    }

    int Solve(vector<Item> items, int max_weight) {
        const int item_count = items.size();
        sort(items.begin(), items.end(), [](Item lhs, Item rhs) { return static_cast<uint64_t>(lhs.cost) * rhs.weight > static_cast<uint64_t>(rhs.cost) * lhs.weight; });

        const vector base_items(items.begin(), items.begin() + item_count / 2);
        const vector support_items(items.begin() + item_count / 2, items.end());

        const int base_total_weight = transform_reduce(base_items.begin(), base_items.end(), 0, plus<>{}, [](Item item) { return item.weight; });
        const int base_min_weight = transform_reduce(
            base_items.begin(), base_items.end(),
            numeric_limits<int>::max(),
            [](int x, int y) { return min(x, y); },
            [](Item item) { return item.weight; }
        );
        const vector<Subset> support_subsets =
            GenerateSupportSubsets(
                support_items,
                max(0, max_weight - base_min_weight);

        LOG_DURATION("--- base bt")
        return BaseTry(base_items, support_subsets, 0, max_weight, 0, NO_SOLUTION_COST);
    }

}


Item GenerateItem(mt19937& generator, int max_cost, int max_weight) {
    return {
        uniform_int_distribution(1, max_cost)(generator),
        uniform_int_distribution(1, max_weight)(generator)
    };
}

vector<Item> GenerateItems(mt19937& generator, int item_count, int max_cost, int max_weight) {
    vector<Item> items;
    items.reserve(item_count);
    for (int i = 0; i < item_count; ++i) {
        items.push_back(GenerateItem(generator, max_cost, max_weight));
    }
    return items;
}


template<typename Solver>
void Test(string_view mark, Solver solver, const vector<Item>& items, int max_weight) {
    int result;
    {
        LOG_DURATION(mark);
        result = solver(items, max_weight);
    }
    if (result == NO_SOLUTION_COST) {
        cout << "No solution" << endl;
    } else {
        cout << result << endl;
    }
}

#define TEST(ns) Test(#ns, ns::Solve, items, max_weight)


int main() {
    mt19937 generator;

    const vector<Item> items_small = {
        {40, 60},
        {60, 50}, // +
        {10, 30},
        {10, 10}, // +
        {3, 10},  // +
        {20, 40},
        {60, 30}  // +
    };
    const TestCase small = {items_small, 100};
    (void) small;

    /*
    const vector<Item> nomim_items = {
        {10, 10},
        {10, 10},
        {10, 10},
        {1, 2}
    };
    const TestCase nomim = {nomim_items, 12};
    (void) nomim;
    */

    const vector<Item> items_l = GenerateItems(generator, 25, 10'000, 1'000'000);
    const TestCase l_normal = {items_l, 10'000'000};
    const TestCase l_all = {items_l, 1'000'000'000};
    (void) l_normal;
    (void) l_all;

    const vector<Item> items_xl = GenerateItems(generator, 39, 10'000, 1'000'000);
    const TestCase xl_vmany = {items_xl, 15'000'000};
    const TestCase xl_many = {items_xl, 10'000'000};
    const TestCase xl_less = {items_xl, 7'000'000};
    (void) xl_vmany;
    (void) xl_many;
    (void) xl_less;

    const vector<Item> items_xxl = GenerateItems(generator, 1000, 10'000, 1'000'000);
    const TestCase xxl_normal = {items_xxl, 100'000'000};
    (void) xxl_normal;

    const auto& [items, max_weight] = xl_less;

    TEST(Simple);
    // TEST(SimpleBound);
    // TEST(KolesarNoEstimation);
    // TEST(KolesarDFS);
    // TEST(MeetInTheMiddle);
    // TEST(MeetInTheMiddleOptimized);
}
