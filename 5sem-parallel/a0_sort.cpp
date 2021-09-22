#include "profile.h"
#include <algorithm>
#include <execution>
#include <random>
#include <vector>

using namespace std;

vector<int> GenerateNumbers(mt19937& generator,
                            int number_count, int max_value) {
    vector<int> v(number_count);
    for (int& value : v) {
        value = uniform_int_distribution(0, max_value)(generator);
    }
    return v;
}

int main() {
    mt19937 generator;
    const vector<int> v = GenerateNumbers(generator, 10'000'000, 1'000);

    vector<int> v1 = v;
    {
        LOG_DURATION("simple");
        sort(v1.begin(), v1.end());
    }

    vector<int> v2 = v;
    {
        LOG_DURATION("seq");
        sort(execution::seq, v2.begin(), v2.end());
    }

    vector<int> v3 = v;
    {
        LOG_DURATION("par");
        sort(execution::par, v3.begin(), v3.end());
    }
}
