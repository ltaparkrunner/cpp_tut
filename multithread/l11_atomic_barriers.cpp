#include <thread>
#include <atomic>
#include <iostream>

std::atomic_int data = 0;
std::atomic_bool ready = false;

int main() {
    std::jthread a([]{
            data.store(42, std::memory_order_relaxed);
            // everything upper executed BEFORE the things downer
            std::atomic_thread_fence(std::memory_order_release);
            ready.store(true, std::memory_order_relaxed);
    });
    std::jthread b([] {
        while (!ready.load(std::memory_order_relaxed))
            ;// in cycle until ready
        // everything downer is after barrier
        std::atomic_thread_fence(std::memory_order_acquire);
        std::cout << data.load(std::memory_order_relaxed);
    });

}