#include <future>
#include <iostream>
#include <vector>
#include <mutex>


std::mutex mtx;

// output is wrong (not order)
// multiple threads try to write to stdout
// RACE CONDITION
int print_no_mutex(int m) {
    std::cout << "got: " << m << std::endl;
    return 2 * m;
}

// mutex helps here
// but it's better to use lock_guard
// to avoid dead lock in case of exceptions
int print_with_mutex(int m) {
    mtx.lock();
    std::cout << "got: " << m << std::endl;
    mtx.unlock();
    return 2 * m;
}

// program freezes
// after exception mutex is locked
// and each thread waits for mutex to unlock
// DEAD LOCK
int print_with_mutex_exception(int m) {
    mtx.lock();
    if (m == 5) throw "Want hugs";
    std::cout << "got: " << m << std::endl;
    mtx.unlock();
    return 2 * m;
}

// Program finishes correctly (without 5)
int print_with_mutex_exception_lock_guard(int m) {
    std::lock_guard<std::mutex> lck (mtx);
    if (m == 5) throw "Want hugs";
    std::cout << "got: " << m << std::endl;
    return 2 * m;
}

int main() {
    std::vector<std::future<int>> futures;

    for(int i = 0; i < 10; ++i) {
        // futures.push_back(std::async(print_no_mutex, i));
        // futures.push_back(std::async(print_with_mutex, i));
        // futures.push_back(std::async(print_with_mutex_exception, i));
        futures.push_back(std::async(print_with_mutex_exception_lock_guard, i));
    }

    for (auto& fut : futures) {
        fut.wait();
    }

    return 0;
}
