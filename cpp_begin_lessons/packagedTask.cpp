#include <future>
#include <iostream>
#include <stdexcept>

int Add(int x, int y){
    std::cout << "Add thread id: " << std::this_thread::get_id() << std::endl;
    return x + y;
}

void Divide(int x, int y, std::promise<int> promise) {
    std::cout << "Divide thread id: " << std::this_thread::get_id() << std::endl;
    if (y == 0)
        promise.set_exception(std::make_exception_ptr(std::invalid_argument("Division by zero")));
    else
        promise.set_value(x/y);
}

int main(){
    std::cout << "Main thread id: " << std::this_thread::get_id() << std::endl;
    try{
        std::promise<int> promise1;
        auto result1 = promise1.get_future();
        std::jthread worker1{ Divide, 30,4, std::move(promise1)};
        std::cout << result1.get() << std::endl;

        std::promise<int> promise2;
        auto result2 = promise2.get_future();
        std::jthread worker2{ Divide, 30,4, std::move(promise2)};
        std::cout << result2.get() << std::endl;

    }catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}

int main2(){
    std::cout << "Main threaad id:" << std::this_thread::get_id() << std::endl;
    std::packaged_task<int(int, int)> add(Add);
    auto result = add.get_future();
    std::jthread worker{ std::move(add), 10, 20 };
    worker.join();

    std::cout << result.get() << std::endl;
    return 0;
}

int main3(){
    std::promise<std::string> strPromise;
    std::vector<std::string> strs;

    try {
        strPromise.set_value(strs.at(0));
    }catch (...) {
        strPromise.set_exception(std::current_exception());
    }

    try {
        std::cout << strPromise.get_future().get() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
