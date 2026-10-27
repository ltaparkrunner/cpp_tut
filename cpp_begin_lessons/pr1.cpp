//using namespace std;
#include <iostream>
#include <algorithm>
#include <thread>
#include <vector>

template <typename T>
std::jthread SortVector(std::vector<T>& values) {
    return std::jthread([&values]() {
        std::ranges::sort(values);
    });
}

// template <typename T>
// std:jthread SortVector(std::vector<T>& values) {
//     return std::jthread([&values] {
//         std::sort(values.begin(), values.end());
//     });
// }

int main() {
    std::vector<int> values = {5, 2, 9, 1, 5, 6};
    std::vector<std::string> strings = {"one", "two", "three", "four", "five"};
    {
        auto sorting_thread = SortVector(values);
        auto sorting_thread2 = SortVector(strings);
    }
    // Print sorted values
    for (const auto& value : values) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    for (const auto& str : strings) {
        std::cout << str << " ";
    }
    std::cout << std::endl;

    return 0;
}