#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

// 1. Включаем макрос для выбора стандартной сборки RCU перед заголовком
#define URCU_INLINE_SMALL_FUNCTIONS
#include <urcu.h> 

struct Config {
    int update_counter;
    std::string server_url;
};

// Атомарный указатель на текущую конфигурацию
std::atomic<Config*> global_config{new Config{0, "http://initial.com"}};

// Функция безопасного чтения (Lock-Free)
void read_config(int thread_id) {
    // В liburcu каждый рабочий поток ОБЯЗАН зарегистрироваться перед вызовом RCU
    rcu_register_thread();
    
    // Входим в критическую секцию RCU (аналог std::rcu_reader)
    rcu_read_lock(); 
    
    Config* ptr = global_config.load(std::memory_order_acquire);
    
    // Чтение происходит абсолютно без мьютексов
    std::cout << "[Поток " << thread_id << "] Чтение URL: " << ptr->server_url 
              << " (Версия: " << ptr->update_counter << ")\n";
              
    // Выходим из критической секции          
    rcu_read_unlock();
    
    // Отменяем регистрацию перед завершением работы потока
    rcu_unregister_thread();
}

// Функция обновления данных (Write)
void update_config(int new_version, std::string new_url) {
    Config* new_config = new Config{new_version, new_url};
    
    // Атомарно подменяем указатель для новых читателей
    Config* old_config = global_config.exchange(new_config, std::memory_order_acq_rel);
    
    // Ждем, пока ВСЕ текущие потоки вышли из своих rcu_read_lock()
    synchronize_rcu(); 
    
    // Теперь безопасно удаляем старую память
    delete old_config;
    std::cout << "--- Конфигурация успешно обновлена в памяти! ---\n";
}

int main() {
    // Главный поток тоже должен быть зарегистрирован в RCU
    rcu_register_thread();

    std::thread t1([]() {
        for(int i = 0; i < 3; ++i) {
            read_config(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    std::thread t2([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        update_config(1, "https://new-endpoint.com");
    });

    t1.join();
    t2.join();

    rcu_unregister_thread();
    delete global_config.load();
    return 0;
}
