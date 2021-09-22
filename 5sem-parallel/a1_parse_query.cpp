#include <algorithm>
#include <execution>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "profile.h"

using namespace std;

vector<string> SplitIntoWords(string_view text) {
    vector<string> words = {""};
    for (const char c : text) {
        if (c == ' ') {
            words.emplace_back();
        } else {
            words.back().push_back(c);
        }
    }
    return words;
}

string GenerateQuery(mt19937& generator, int max_length, int space_rate) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string query(length, ' ');
    for (char& c : query) {
        const int rnd = uniform_int_distribution(0, space_rate - 1)(generator);
        if (rnd > 0) {
            c = 'a' + (rnd - 1);
        }
    }
    return query;
}

vector<string> GenerateQueries(mt19937& generator, int query_count, int max_length, int space_rate) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, max_length, space_rate));
    }
    return queries;
}


int main() {
    mt19937 generator;
    const auto queries = GenerateQueries(generator, 20000000, 2, 4);

    {
        vector<int> word_counts(queries.size());
        {
            LOG_DURATION("simple");
            transform(queries.begin(), queries.end(), word_counts.begin(),
                      [](string_view query) {
                          return SplitIntoWords(query).size();
                      });
        }
        cout << accumulate(word_counts.begin(), word_counts.end(), 0) << endl;
    }

    {
        vector<int> word_counts(queries.size());
        {
            LOG_DURATION("parallel");
            transform(execution::par, queries.begin(), queries.end(), word_counts.begin(),
                      [](string_view query) {
                          return SplitIntoWords(query).size();
                      });
        }
        cout << accumulate(word_counts.begin(), word_counts.end(), 0) << endl;
    }

    {
        vector<int> word_counts(queries.size());
        {
            LOG_DURATION("parallel unseq");
            transform(execution::par_unseq, queries.begin(), queries.end(), word_counts.begin(),
                      [](string_view query) {
                          return SplitIntoWords(query).size();
                      });
        }
        cout << accumulate(word_counts.begin(), word_counts.end(), 0) << endl;
    }

    {
        vector<int> word_counts(queries.size());
        {
            LOG_DURATION("parallel with sum");
            transform(execution::par, queries.begin(), queries.end(), word_counts.begin(),
                      [](string_view query) {
                          return SplitIntoWords(query).size();
                      });
            cout << reduce(word_counts.begin(), word_counts.end()) << endl;
        }
    }

    {
        vector<int> word_counts(queries.size());
        {
            LOG_DURATION("parallel with parallel sum");
            transform(execution::par, queries.begin(), queries.end(), word_counts.begin(),
                      [](string_view query) {
                          return SplitIntoWords(query).size();
                      });
            cout << reduce(execution::par, word_counts.begin(), word_counts.end()) << endl;
        }
    }

    {
        vector<int> word_counts(queries.size());
        {
            LOG_DURATION("seq with transform_reduce");
            cout << transform_reduce(
                execution::seq,
                queries.begin(), queries.end(),
                static_cast<size_t>(0),
                plus<>{},
                [](string_view query) {
                    return SplitIntoWords(query).size();
                }
            ) << endl;
        }
    }

    {
        vector<int> word_counts(queries.size());
        {
            LOG_DURATION("parallel with transform_reduce");
            cout << transform_reduce(
                execution::par,
                queries.begin(), queries.end(),
                static_cast<size_t>(0),
                plus<>{},
                [](string_view query) {
                    return SplitIntoWords(query).size();
                }
            ) << endl;
        }
    }
}
