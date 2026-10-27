#include <iostream>
#include <thread>
// void Increment(int& counter) { ++counter; }
// void Decrement(int& counter) { --counter; }

void Increment(std::atomic<int>& counter) { ++counter; }
void Decrement(std::atomic<int>& counter) { --counter; }

int main() {
    std::atomic<int> counter = 0;
    std::jthread t1{ [&counter] {
        for (int i=0; i< 1'000'000'000l; ++i)
            Increment(counter);
    }};
    std::jthread t2{ [&counter] {
        for (int i=0; i<1'000'000'000l; ++i)
            Decrement(counter);
    }};
    t1.join(); t2.join();
    std::cout << "Counter: " << counter << '\n';
}

// Increment(int&)
//  add     DWORD PTR [rdi], 0x1
//  ret

// Increment(std::atomic<int>&):
//     lock add DWORD PTR [rdi], 0x1
//     ret

// mutual exclusion
// turn off interruptions
// x86-64 IF(INterrupt Flag) в регистре RFLAGS
// Instructions: CLI STI
// in linux irq_save/irq_restore only the core
// spin_lock_irqsave / spin_lock_irqrestore
