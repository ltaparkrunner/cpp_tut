#include <thread>
#include <atomic>
#include <iostream>

std::atomic<bool> x = false, y = false;
std::atomic<int> z = 0;

int main() {
    std::jthread a([]{
        x.store(true, std::memory_order::seq_cst);
//        std::cout << "a.store.true.finished" << std::endl;
    });
    std::jthread b([]{
        y.store(true, std::memory_order::seq_cst);
//        std::cout << "b.store.true.finished" << std::endl;
    });
    std::jthread c([]{
        while(!x.load(std::memory_order::seq_cst))
        ;
//        std::cout << "c.while.finished" << std::endl;
        if (y.load(std::memory_order::seq_cst)) {
             ++z;
//            std::cout << "++z" << std::endl;           
        }
    });
    std::jthread d([]{
        while(!y.load(std::memory_order::seq_cst))
        ;
//        std::cout << "d.while.finished" << std::endl;
        if (x.load(std::memory_order::seq_cst)){
            ++z;
//            std::cout << "++z" << std::endl;
        }
            
    });
    a.join(); b.join(); c.join(); d.join();
    std::cout << "z.load(): " << z.load() << std::endl;
}