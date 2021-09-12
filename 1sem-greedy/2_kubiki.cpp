#include <iostream>
#include <set>
#include <vector>

struct Color {
    int max_value;
    int color;

    Color (int max_value, int color): max_value(max_value), color(color) {}
};

int main() {
    // want to use upper_bound. 2 > 3 in this function
    auto max_to_min_cmp = [](const Color& a, const Color& b) {
        return a.max_value > b.max_value || (a.max_value == b.max_value && a.color > b.color);
    };
    std::set<Color, decltype(max_to_min_cmp)> max_to_color(max_to_min_cmp);

    int n;
    std::cin >> n;

    std::vector<int> order(n);

    int max_color = 0;
    int num;
    // will use this pair for searching in max_to_color set
    auto to_search = Color{-1, -1};

    for (int i = 0; i != n; ++i) {
        std::cin >> num;

        to_search.max_value = num;

        auto upper = max_to_color.upper_bound(to_search);
        // if we found the color with largest value lower then num.
        if (upper != max_to_color.end()) {
            order[i] = upper->color;

            auto new_max = Color{num, upper->color};
            max_to_color.erase(upper);
            max_to_color.insert(new_max);
        } else {
            // create new color
            max_to_color.emplace(num, ++max_color);
            order[i] = max_color;
        }
    }

    std::cout << max_to_color.size() << std::endl;

    for (int i = 0; i != order.size(); ++i) {
        std::cout << order[i];
        if (i != order.size() - 1) {
            std::cout << " ";
        } else {
            std::cout << std::endl;
        }
    }
}
