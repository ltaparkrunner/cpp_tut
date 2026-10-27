#include <iostream>
#include <thread>

template <typename T>
std::jthread make_worker(T value) {
    return std::jthread([value]() {
// std::thread make_worker(T value) {
//     return std::thread([value]() {
        std::cout << "Thread processing value: " << value << std::endl;
    });
}

int main() {
    auto worker1 = make_worker(42);
    auto worker2 = make_worker("Hello, World!");
    std::cout << "C++ Standard Number: " << __cplusplus << std::endl;
    // The threads will automatically join when they go out of scope
    return 0;
}   