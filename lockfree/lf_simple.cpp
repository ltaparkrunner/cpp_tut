#include <iostream>
#include <thread>
#include <vector>
#include <boost/lockfree/stack.hpp>
#include <syncstream>

// Создаем lock-free стек с фиксированным максимальным размером на 128 элементов
boost::lockfree::stack<int, boost::lockfree::capacity<128>> lf_stack;

void producer(int id) {
    for (int i = 0; i < 5; ++i) {
        int value = id * 100 + i;
        // push() возвращает true, если элемент успешно добавлен
        if (lf_stack.push(value)) {
            std::osyncstream(std::cout) << "[Producer " << id << "] Pushed: " << value << "\n";
        } else {
            std::osyncstream(std::cout) << "[Producer " << id << "] Stack is full!\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void consumer(int id) {
    int value;
    for (int i = 0; i < 5; ++i) {
        // pop() возвращает true и записывает значение в переменную, если стек не пуст
        if (lf_stack.pop(value)) {
            std::osyncstream(std::cout) << "[Consumer " << id << "] Popped: " << value << "\n";
        } else {
            std::osyncstream(std::cout) << "[Consumer " << id << "] Stack is empty, retrying...\n";
            --i; // Пробуем еще раз
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
}

int check_versions() {
        // 1. Вывод версии компилятора
    std::cout << "Compiler: ";
#if defined(__clang__)
    std::cout << "Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__GNUC__) || defined(__GNUG__)
    std::cout << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
    std::cout << "MSVC (MSVC++ version: " << _MSC_VER << ", Full: " << _MSC_FULL_VER << ")";
#else
    std::cout << "Unknown Compiler";
#endif
    std::cout << "\n";

    // 2. Вывод версии Boost
    // Макрос BOOST_VERSION хранит версию в виде: MMmmpp (Major * 100000 + Minor * 100 + Patch)
    // Например, 108200 означает Boost 1.82.0
    std::cout << "Boost Raw Version: " << BOOST_VERSION << "\n";
    std::cout << "Boost Version: " 
              << BOOST_VERSION / 100000 << "."      // Major
              << (BOOST_VERSION / 100) % 1000 << "." // Minor
              << BOOST_VERSION % 100 << "\n";       // Patch

    // Также можно использовать готовую строковую константу из Boost:
    std::cout << "Boost Library Version: " << BOOST_LIB_VERSION << "\n";

    std::cout << "Raw __cplusplus value: " << __cplusplus << "\n";

    std::cout << "C++ Standard: ";
    if (__cplusplus == 202600L) std::cout << "C++26 (Experimental/Draft)"; // Примерное значение для C++26
    else if (__cplusplus == 202302L) std::cout << "C++23";
    else if (__cplusplus == 202002L) std::cout << "C++20";
    else if (__cplusplus == 201703L) std::cout << "C++17";
    else if (__cplusplus == 201402L) std::cout << "C++14";
    else if (__cplusplus == 201103L) std::cout << "C++11";
    else if (__cplusplus == 199711L) std::cout << "C++98";
    else std::cout << "Pre-standard C++ or Unknown (" << __cplusplus << ")";
    std::cout << "\n";

    return 0;
}

int main() {
    check_versions();
    std::cout << "Starting Lock-Free Stack Demo...\n";

    // Запускаем 2 пишущих и 2 читающих потока
    std::vector<std::thread> threads;
    threads.emplace_back(producer, 1);
    threads.emplace_back(producer, 2);
    threads.emplace_back(consumer, 1);
    threads.emplace_back(consumer, 2);

    // Ждем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Demo finished.\n";
    return 0;
}
