#include <iostream>
#include <atomic>
// #include <rcu>      // Новинка C++26
#include <urcu.h>
#include <thread>
#include <chrono>

// Структура с данными конфигурации
struct Config {
    int update_counter;
    std::string server_url;
};

// Атомарный указатель на текущую конфигурацию
std::atomic<Config*> global_config{new Config{0, "http://initial.com"}};

// Функция безопасного чтения (Lock-Free)
void read_config(int thread_id) {
    // Входим в критическую секцию RCU. 
    // Пока мы здесь, память, на которую указывает ptr, не будет физически удалена.
    std::rcu_reader reader; 
    
    Config* ptr = global_config.load(std::memory_order_acquire);
    
    // Чтение происходит абсолютно без мьютексов и атомарных инструкций на запись
    std::cout << "[Поток " << thread_id << "] Чтение URL: " << ptr->server_url 
              << " (Версия: " << ptr->update_counter << ")\n";
}

// Функция обновления данных (Write)
void update_config(int new_version, std::string new_url) {
    Config* new_config = new Config{new_version, new_url};
    
    // Атомарно подменяем указатель для новых читателей
    Config* old_config = global_config.exchange(new_config, std::memory_order_acq_rel);
    
    // C++26 Синхронизация: ждем, пока ВСЕ текущие потоки-читатели (std::rcu_reader),
    // которые успели захватить old_config, выйдут из своих критических секций.
    std::rcu_synchronize();
    
    // Теперь мы железно уверены, что old_config больше никто не использует. 
    // Безопасно удаляем старую память.
    delete old_config;
    std::cout << "--- Конфигурация успешно обновлена в памяти! ---\n";
}

int main() {
    // Запускаем поток для симуляции чтения
    std::thread t1([]() {
        for(int i = 0; i < 3; ++i) {
            read_config(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    // Пишущий поток обновляет данные
    std::thread t2([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        update_config(1, "https://new-endpoint.com");
    });

    // Еще один читающий поток
    std::thread t3([]() {
        for(int i = 0; i < 3; ++i) {
            read_config(3);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
    });

    t1.join();
    t2.join();
    t3.join();

    // Очистка при выходе
    delete global_config.load();
    return 0;
}
