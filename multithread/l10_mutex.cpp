#include <thread>
#include <atomic>
#include <iostream>
#include <cassert>

#include <vector>
#include <latch>
#include <syncstream>

class SpinMutex {
public:
    SpinMutex() = default;
    ~SpinMutex() {
        assert(m_flag.load(std::memory_order::relaxed) == false);
    }
    void lock() noexcept {
        bool current;
        do {
            current = false;
        } while (!m_flag.compare_exchange_weak(current, true, 
            std::memory_order::acquire));
    }

    bool try_lock() noexcept {
        bool current = false;
        return m_flag.compare_exchange_strong(current, true, 
            std::memory_order::acquire);
    }

    void unlock() noexcept {
        m_flag.store(false, std::memory_order::release);
    }
private:
    std::atomic_bool m_flag = false;
};

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
        auto data = m_data.load(std::memory_order::acquire);
        if(!data) {
            std::lock_guard lk { m_mutex };
            if (!(data = m_data.load(std::memory_order::relaxed))) {
                data = new Data();
                m_data.store(data, std::memory_order::release);
            }
        }
    }
private:
    mutable std::atomic<const Data*> m_data = nullptr;
//    mutable SpinMutex m_mutex;
    mutable std::mutex m_mutex;
};

int main() {

}