#include <iostream>
#include <functional>
#include <thread>

//std::hazard_pointers;
void func(const std::function<void(int)> callback) {
    for(int i=0; i<10; i++)
        callback(i);
}

int main() {
    func([](int i){
        std::cout << " func output: " << i << std::endl;
    });
}