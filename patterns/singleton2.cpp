#include <iostream>
#include <thread>
#include <latch>

class Singletone {
public:
    static Singletone getInstance(int jv){
        j = jv;
        static Singletone instance;
        return instance;
    }
    void setI(int iv){i = iv;}
    void outpI() {
        std::cout << "Singletone I: " << i << "  J: " << j << std::endl;
    }

private:
    Singletone(){
        std::cout << "Constructor" << std::endl;
    }
    int i;
    inline static int j = 0;
};

int main(){
    int i;
    std::latch latch{2};
    std::jthread t1{[&latch](){
        latch.arrive_and_wait();
        Singletone s1 = Singletone::getInstance(1);
        s1.setI(5);
        s1.outpI();
    }};
    std::jthread t2{[&latch](){
        latch.arrive_and_wait();
        Singletone s1 = Singletone::getInstance(2);
        s1.setI(8);
        s1.outpI();
    }};
    t1.join(); t2.join();
}