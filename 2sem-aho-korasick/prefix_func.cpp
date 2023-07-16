#include <iostream>
#include <string>
#include <vector>

std::vector<int> pref_func(std::string& s) {
    std::vector<int> pi(s.length());
    pi[0] = 0;
    int k;

    for (int i = 1; i != s.length(); ++i) {
        k = pi[i - 1];
        while (k > 0 && s[i] != s[k] ) {
            k = pi[k - 1];
        }

        if (s[i] == s[k]) {
            ++k;
        }

        pi[i] = k;
    }

    return pi;

}

std::vector<int> string_by_pref(std::vector<int>& pi) {
    std::vector<int> res;
    int v = 0;

    for (int i = 0; i != pi.size(); ++i) {
        if (pi[i] == 0) {
            res.push_back(v);
            ++v;
        } else {
            int prev = res[pi[i] - 1];
            res.push_back(prev);
        }
    }

    return res;
}

int main() {

    std::string s = "abacabab";
    std::vector<int> res = pref_func(s);
    for (int i = 0; i != s.length(); ++i) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;
    for (const char c : s) {
        std::cout << c << "\t";
    }
    std::cout << std::endl;
    for (int v : res) {
        std::cout << v << "\t";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "found string" << std::endl;
    std::vector<int> sbp = string_by_pref(res);

    for (int v : sbp) {
        std::cout << v << "\t";
    }
}
