#include <atomic>
#include <thread>
#include <string>
#include <iostream>
#include <cassert>

std::string shared_data;           // Обычные данные
std::atomic<bool> ready(false);    // Флаг готовности

void producer() {
    shared_data = "Hello from the thread 1!"; // 1. Записываем данные
    
    // БАРЬЕР RELEASE: гарантирует, что запись в shared_data 
    // завершится ДО того, как флаг станет true.
    std::atomic_thread_fence(std::memory_order_release);
    
    ready.store(true, std::memory_order_relaxed); // 2. Устанавливаем флаг
}

void consumer() {
    // Ждем, пока флаг станет true
    while (!ready.load(std::memory_order_relaxed)) {
        // Ждем...
    }

    // БАРЬЕР ACQUIRE: гарантирует, что чтение shared_data 
    // произойдет ПОСЛЕ того, как мы увидели флаг true.
    std::atomic_thread_fence(std::memory_order_acquire);

    // 3. Теперь мы на 100% уверены, что данные готовы
    std::cout << "Received: " << shared_data << std::endl;
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();

    std::atomic<int> counter(10);
    int vr = 7;
    // Adds 5 to counter, returns 10
    int old_val = counter.fetch_add(vr, std::memory_order_relaxed);

    std::cout << "Old value: " << old_val << "\n"; // Output: 10
    std::cout << "New value: " << counter.load() << "\n"; // Output: 15

    for(int i=0; i<10000; ++i) {

        counter.store(0, std::memory_order_relaxed);
        std::thread t3([&counter] {
            for (int i = 0; i < 1000; ++i) {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
        std::thread t4([&counter] {
            for (int i = 0; i < 1000; ++i) {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
        t3.join();
        t4.join();
        assert(counter.load() == 2000); // Может не сработать из-за гонки данных
    }
    return 0;
}

/*

multy thread memory models: 
            sequential consistency
			acquire/release
			relaxed memory ordering
			
			reffered as atom operation argument

        memory_order_relaxed: no ordering guarantees, only atomicity
        memory_order_release: ensures that all previous writes in the thread are visible to other threads that
        memory_order_acquire: ensures that all subsequent reads in the thread see the effects of writes that were released by other threads
        memory_order_seq_cst: provides a total ordering of all operations, ensuring that all threads
        memory_order_consume: similar to acquire, but only enforces ordering on dependent operations (not widely supported)
        memory_order_acq_rel: combines acquire and release semantics, used for read-modify-write operations

        acquire semantics: Load | Load, Load | Store
        release semantics: Store | Load, Store | Store
        Store | Load
*/


std::atomic<int> data{10};

void increment(int inc) {
    int expected = data.load(); // 1. Читаем текущее
    int desired;

    do {
        desired = expected + inc; // 2. Готовим новое
        
        // 3. Пытаемся применить. 
        // Если вернул false (из-за несовпадения ИЛИ ложного отказа),
        // 'expected' обновится сам, и мы пойдем на новый круг.
    } while (!data.compare_exchange_weak(expected, desired));
}
