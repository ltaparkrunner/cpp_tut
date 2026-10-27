#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <latch>
#include <syncstream>


class Data{
public: 
    Data() {
        std::cout << "Data::Data()" << std::endl;
    }
    void UseData() const {
        std::cout << "Data::UseData()" << std::endl;
    }
};
class Widget {
public:
    Widget() = default;
    Widget(const Widget&) = delete;
    Widget& operator = (const Widget&) = delete;
    ~Widget() {
        delete m_data;
    }
    void Foo() const {
        std::osyncstream(std::cout) << "Widget::Foo()" << std::endl;
//        if (std::lock_guard lk{m_mutex}; !m_data)
//            m_data = new Data();
//        std::call_once(flag, [](const Data* data){data = new Data;}, m_data);
        std::call_once(flag, [this](){m_data = new Data;});
        m_data->UseData();
    }
private:
//    void init(Data* dat) {m_data = new Data;}
    mutable const Data* m_data = nullptr;
    mutable std::once_flag flag;
//    mutable std::mutex m_mutex;
};

int main() {
    Widget widget;

    const unsigned numThreads = 8;
    std::latch latch{ numThreads };
    std::vector<std::jthread> threads;
    for (unsigned i = 0; i < numThreads; ++i) {
        threads.emplace_back([&latch, &widget] {
            latch.arrive_and_wait();
            widget.Foo();
        });
    }
}


