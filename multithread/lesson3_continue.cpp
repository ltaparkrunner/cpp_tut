// std::forward lecture of templates
// detach -- отсоединить
// std::reference_wrapper std::cref std::cref

// forwarding - пересылка, экспедиция, экспедиция, отправка

#include <iostream>
#include <thread>

class Spy {
public:
    Spy() {
        std::cout << "Spy() in thread " << std::this_thread::get_id() << std::endl;
    }
    Spy(int value) {
        std::cout << "Spy(" << value << ") in thread " << std::this_thread::get_id() << std::endl;
    }
    Spy(const Spy&) {  // copy constractor
        std::cout << "Spy(const Spy&) in thread " << std::this_thread::get_id() << std::endl;
    }
    Spy(Spy&&) noexcept {   // move constructor
        std::cout << "Spy(Spy&&) in thread " << std::this_thread::get_id() << std::endl;
    }
    ~Spy() {
        std::cout << "~Spy() in thread " << std::this_thread::get_id() << std::endl;
    }
};

int main() {
    std::cout << "Main thread id: " << std::this_thread::get_id() << std::endl;
    std::jthread t1{
        [](Spy) {
            std::cout << "Thread func is invoked in thread " << std::this_thread::get_id() << std::endl;
        },
        Spy()
    };
    t1.join();

    std::jthread t2{
        [](const Spy&) {
            std::cout << "Thread func is invoked in thread " << std::this_thread::get_id() << std::endl;
        },
        42
    };
    t2.join();
}
/*
When copy constructor's called:
When an object is initialized from another object of the same class (e.g., MyClass b = a; or MyClass b(a);).
When an object is passed by value to a function.
When an object is returned by value from a function (though this can be optimized by the compiler via Return Value Optimization, RVO)
*/

/*
When move constructor's called:
When an object is initialized from an rvalue (like a temporary object returned from a function).
When std::move() is used to explicitly cast an lvalue (named object) to an rvalue reference, indicating that its resources can be safely moved.
Used internally by standard library containers (like std::vector) to resize efficiently.

*/
// для синхронизации ввода вывода используем 
// std::isyncstream/std::osyncsteram
