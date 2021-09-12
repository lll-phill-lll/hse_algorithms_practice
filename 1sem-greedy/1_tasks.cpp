#include <iostream>
#include <set>
#include <tuple>

// long long
// same pairs
int main() {
    long long N, A;
    std::cin >> N >> A;
    std::set<std::tuple<long long, long long, long long>> tasks;

    long long ai, bi;
    for (long long i = 0; i != N; ++i) {
        std::cin >> ai >> bi;
        tasks.emplace(ai, bi, i);
    }

    long long max_tasks = 0;
    for (const auto& task : tasks) {
        if (std::get<0>(task) <= A) {
            A += std::get<1>(task);
            ++max_tasks;
        } else {
            break;
        }
    }

    std::cout << max_tasks << std::endl;
}
