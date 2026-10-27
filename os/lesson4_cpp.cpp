#include <thread>
#include <vector>
#include <iostream>

thread_local int g_lastErr = 0;
template <typename T>
std::jthread SortVector(std::vector<T>& values) {
    return std::jthread{[&values] {
        std::ranges::sort(values);
    }};
};

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

// FSM Конечный автомат Finite-State Machine  event-driven
// Thread-local storage (TLS)

