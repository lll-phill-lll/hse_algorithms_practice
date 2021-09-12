/*
Для хранения двух агрессивных жидкостей 𝐴 и 𝐵 используется емкость с
многослойной перегородкой, которая изготавливается из имеющихся 𝑁 листов.
Для каждого листа 𝑖 (𝑖 = 1, ..., 𝑁) известно время его растворения
жидкостью 𝐴 - 𝑎𝑖 и жидкостью 𝐵 - 𝑏𝑖

Растворение перегородки каждой из жидкостей происходит последовательно лист за листом,
с постоянной скоростью по толщине листа.
Требуется спроектировать такую перегородку, время растворения которой было бы максимальным.

В первой строке входного файла записано число 𝑁
(1 ≤ 𝑁 ≤ 256). В каждой из последующих N строк содержатся два положительных вещественных
числа 𝑎𝑖 и 𝑏i, разделенные пробелом.

Выходные данные:
В первую строку выходного файла записать время растворения перегородки с точностью до 3 цифр
после десятичной точки. В следующую строку файла записать номера листов в порядке
их расположения от жидкости A к жидкости B, разделяя числа пробелами.
Примеры
Входные данные

4
1 2
1 2
0.5 1.5
7 3.5

Выходные данные

6.000
4 1 2 3
*/



#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

struct plate {
    double a, b;
    int idx;

    plate(double a, double b, int idx): a(a), b(b), idx(idx){}
};

double run(std::vector<plate>& v) {
    int i = 0, j = v.size() - 1;
    double dissolve_time = 0;
    while (i < j) {
        // left dissolved earlier
        if (v[i].a < v[j].b) {
            v[j].a = v[j].a * (v[j].b - v[i].a) / v[j].b;
            v[j].b = v[j].b - v[i].a;
            dissolve_time += v[i].a;
            ++i;
        } else {
            v[i].b = v[i].b * (v[i].a - v[j].b) / v[i].a;
            v[i].a = v[i].a - v[j].b;
            dissolve_time += v[j].b;
            --j;

        }
        // in case is both dissolved in the same time
        if (v[i].a == 0) ++i;
        if (v[j].b == 0) --j;
    }

    // 1 plate left
    if (i == j) {
        dissolve_time += v[i].a * v[i].b / (v[i].a + v[i].b);
    }

    return dissolve_time;
}

int main() {
    int N;
    std::cin >> N;

    std::vector<plate> v;

    double a, b;
    for (int i = 0; i != N; ++i) {
        std::cin >> a >> b;
        v.emplace_back(a, b, i + 1);
    }

    auto cmp = [](const plate& lhs, const plate& rhs ) {
       return lhs.a / lhs.b > rhs.a / rhs.b;
    };

    // auto cmp = [](const plate& lhs, const plate& rhs ) {
    //    return lhs.a - lhs.b > rhs.a - rhs.b;
    // };

    std::sort(v.begin(), v.end(), cmp);

    std::cout << std::setprecision(3) << std::fixed;
    std::cout << run(v) << std::endl;
    for (const auto& p : v) {
        std::cout << p.idx << " ";
    }
}
