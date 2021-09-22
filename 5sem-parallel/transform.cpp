#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string s("hello");
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) -> unsigned char { return std::toupper(c); });

    std::vector<std::size_t> ordinals;
    std::transform(s.begin(), s.end(), std::back_inserter(ordinals),
                   [](unsigned char c) -> std::size_t { return c; });

    std::cout << s << std::endl;
    std::cout << s << ':';
    for (auto ord : ordinals) {
       std::cout << ' ' << ord;
    }
}
