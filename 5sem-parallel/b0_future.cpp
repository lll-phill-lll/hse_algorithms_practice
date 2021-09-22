#include "profile.h"
#include <future>
#include <numeric>
#include <vector>

using namespace std;

int main() {
    vector<int> v(400'000'000, 8);

    {
        LOG_DURATION("1 thread");
        cout << accumulate(v.begin(), v.end(), 0) << endl;
    }
    {
        LOG_DURATION("2 threads");
        const auto middle = v.begin() + v.size() / 2;
        auto f1 = async([&v, middle] { return accumulate(v.begin(), middle, 0); });
        auto sum2 = accumulate(middle, v.end(), 0);
        cout << f1.get() + sum2 << endl;
        cout << "lala" << endl;
    }
}
