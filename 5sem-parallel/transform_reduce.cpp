#include <numeric>
#include <vector>
#include <iostream>
#include <string>

int main() {
    auto mice = std::vector<std::string>{"Mickey","Minnie","Jerry"};
    auto num_chars = std::transform_reduce(
      mice.begin(),
      mice.end(),
      size_t{0},
      // reduce: combine all lengths into an answer
      [](size_t a, size_t b) -> size_t { return a + b; },
      // transform: count length of each string
      [](const std::string& m) -> size_t{ return m.size(); }
    );

    std::cout << num_chars << std::endl;
}
