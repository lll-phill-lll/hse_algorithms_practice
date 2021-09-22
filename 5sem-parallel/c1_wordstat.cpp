#include <algorithm>
#include <atomic>
#include <execution>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "profile.h"

using namespace std;


vector<string_view> SplitIntoWords(string_view text) {
    vector<string_view> words;
    while (true) {
        const size_t space_pos = text.find(' ');
        words.push_back(text.substr(0, space_pos));
        if (space_pos == text.npos) {
            break;
        } else {
            text.remove_prefix(space_pos + 1);
        }
    }
    return words;
}


class SearchServer {
public:
    void AddDocument(int document_id, string_view text) {
        for (const string_view word : SplitIntoWords(text)) {
            word_to_documents_[string(word)].insert(document_id);
        }
    }
    vector<int> Find(string_view query) const {
        vector<int> documents;
        for (const string_view word : SplitIntoWords(query)) {
            const auto documents_it = word_to_documents_.find(string(word));
            if (documents_it != word_to_documents_.end()) {
                documents.insert(documents.end(), documents_it->second.begin(), documents_it->second.end());
            }
        }
        documents.erase(unique(documents.begin(), documents.end()), documents.end());
        return documents;
    }

private:
    unordered_map<string, unordered_set<int>> word_to_documents_;
};

string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
    const int word_count = uniform_int_distribution(1, max_word_count)(generator);
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}

template<typename Value>
int ComputeTotalWordStat(const unordered_map<string, Value>& word_stat) {
    int result = 0;
    for (const auto& [_, query_count] : word_stat) {
        result += query_count;
    }
    return result;
}


int main() {
    LOG_DURATION("all");
    mt19937 generator;
    const auto dictionary = GenerateDictionary(generator, 1'000, 25);
    const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);
    SearchServer search_server;
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i]);
    }

    const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);

    cout << "prepared" << endl;

    {
        vector<int> results(queries.size());
        unordered_map<string, int> word_stat;
        {
            LOG_DURATION("simple");
            transform(queries.begin(), queries.end(), results.begin(),
                      [&search_server, &word_stat](string_view query) {
                          for (const string_view word : SplitIntoWords(query)) {
                              ++word_stat[string(word)];
                          }
                          return search_server.Find(query).size();
                      });
        }
        cout << accumulate(results.begin(), results.end(), 0) << " " << ComputeTotalWordStat(word_stat) << endl;
    }

    /*{
        vector<int> results(queries.size());
        unordered_map<string, int> word_stat;
        {
            LOG_DURATION("parallel");
            transform(execution::par,
                      queries.begin(), queries.end(), results.begin(),
                      [&search_server, &word_stat](string_view query) {
                          for (const string_view word : SplitIntoWords(query)) {
                              ++word_stat[string(word)];
                          }
                          return search_server.Find(query).size();
                      });
        }
        cout << accumulate(results.begin(), results.end(), 0) << " " << ComputeTotalWordStat(word_stat) << endl;
    }*/

    {
        vector<int> results(queries.size());
        unordered_map<string, int> word_stat;
        mutex word_stat_mutex;
        {
            LOG_DURATION("global lock");
            transform(execution::par_unseq, queries.begin(), queries.end(), results.begin(),
                      [&search_server, &word_stat, &word_stat_mutex](string_view query) {
                          for (lock_guard guard{word_stat_mutex}; const string_view word : SplitIntoWords(query)) {
                              ++word_stat[string(word)];
                          }
                          return search_server.Find(query).size();
                      });
        }
        cout << accumulate(results.begin(), results.end(), 0) << " " << ComputeTotalWordStat(word_stat) << endl;
    }

    {
        vector<int> results(queries.size());
        unordered_map<string, int> word_stat;
        mutex word_stat_mutex;
        {
            LOG_DURATION("local lock");
            transform(execution::par_unseq, queries.begin(), queries.end(), results.begin(),
                      [&search_server, &word_stat, &word_stat_mutex](string_view query) {
                          for (const string_view word : SplitIntoWords(query)) {
                              lock_guard guard{word_stat_mutex};
                              ++word_stat[string(word)];
                          }
                          return search_server.Find(query).size();
                      });
        }
        cout << accumulate(results.begin(), results.end(), 0) << " " << ComputeTotalWordStat(word_stat) << endl;
    }

    {
        vector<int> results(queries.size());
        unordered_map<string, int> word_stat;
        mutex word_stat_mutex;
        {
            LOG_DURATION("conditional lock");
            transform(execution::par_unseq, queries.begin(), queries.end(), results.begin(),
                      [&search_server, &word_stat, &word_stat_mutex](string_view query) {
                          for (const string_view word : SplitIntoWords(query)) {
                              const string word_copy(word);
                              const auto it = word_stat.find(word_copy);
                              if (it != word_stat.end()) {
                                  ++it->second;  // troubles
                              } else {
                                  lock_guard guard{word_stat_mutex};
                                  ++word_stat[word_copy];
                              }
                          }
                          return search_server.Find(query).size();
                      });
        }
        cout << accumulate(results.begin(), results.end(), 0) << " " << ComputeTotalWordStat(word_stat) << endl;
    }

    {
        vector<int> results(queries.size());
        unordered_map<string, atomic_int> word_stat;
        mutex word_stat_mutex;
        {
            LOG_DURATION("conditional lock with atomics");
            transform(execution::par_unseq, queries.begin(), queries.end(), results.begin(),
                      [&search_server, &word_stat, &word_stat_mutex](string_view query) {
                          for (const string_view word : SplitIntoWords(query)) {
                              const string word_copy(word);
                              const auto it = word_stat.find(word_copy);
                              if (it != word_stat.end()) {
                                  ++it->second;
                              } else {
                                  lock_guard guard{word_stat_mutex};
                                  ++word_stat[word_copy];
                              }
                          }
                          return search_server.Find(query).size();
                      });
        }
        cout << accumulate(results.begin(), results.end(), 0) << " " << ComputeTotalWordStat(word_stat) << endl;
    }

    /*{
        vector<int> results(queries.size());
        unordered_map<string, atomic_int> word_stat;
        mutex word_stat_mutex;
        {
            LOG_DURATION("conditional lock with relaxed atomics");
            transform(execution::par_unseq, queries.begin(), queries.end(), results.begin(),
                      [&search_server, &word_stat, &word_stat_mutex](string_view query) {
                          for (const string_view word : SplitIntoWords(query)) {
                              const string word_copy(word);
                              const auto it = word_stat.find(word_copy);
                              if (it != word_stat.end()) {
                                  it->second.fetch_add(1, memory_order::relaxed);
                              } else {
                                  lock_guard guard{word_stat_mutex};
                                  word_stat[word_copy].fetch_add(1, memory_order::relaxed);
                              }
                          }
                          return search_server.Find(query).size();
                      });
        }
        cout << accumulate(results.begin(), results.end(), 0) << " " << ComputeTotalWordStat(word_stat) << endl;
    }*/
}
