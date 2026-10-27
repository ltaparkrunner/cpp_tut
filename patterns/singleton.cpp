class Singleton {
public:
    static Singleton& getInstance() {
        // Потокобезопасно, начиная с C++11
        static Singleton instance;
        return instance;
    }
private:
    Singleton() = default;
};

#include <mutex>
class Singleton2 {
private:
    static Singleton2* instance;
    static std::once_flag initFlag;

    Singleton2() = default;
    static void init() {
        instance = new Singleton2();
    }

public:
    static Singleton2* getInstance() {
        // guaranteed, that init was called only once
        std::call_once(initFlag, &Singleton2::init);
        return instance;
    }
};

// Singleton2 * Singleton2::instance = nullptr;
// Singleton* Singleton::instance = nullptr;
// std::once_flag Singleton2::initFlag;



#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <print> // Для C++23 (быстрый и потокобезопасный вывод)

class SystemMonitor {
public:
    // 1. Глобальная точка доступа (Синглтон Майерса)
    static SystemMonitor& getInstance() {
        static SystemMonitor instance; // Потокобезопасная ленивая инициализация
        return instance;
    }

    // Запрещаем копирование и присваивание
    SystemMonitor(const SystemMonitor&) = delete;
    SystemMonitor& operator=(const SystemMonitor&) = delete;

    // 2. ДОБАВЛЯЕМ КОД: Бизнес-логика (методы для работы)
    void logEvent(const std::string& event) {
        // Защищаем внутренние данные от одновременной записи из разных потоков
        std::lock_guard<std::mutex> lock(m_mutex); 
        
        m_events.push_back(event);
        m_total_clicks++;
    }

    void printReport() const {
        // mutable мьютекс позволяет делать lock() даже в const-методе
        std::lock_guard<std::mutex> lock(m_mutex); 
        
        std::println("--- ОТЧЕТ СИСТЕМЫ МОНИТОРИНГА ---");
        std::println("Всего зафиксировано кликов: {}", m_total_clicks);
        std::println("История событий:");
        for (const auto& event : m_events) {
            std::println("  - {}", event);
        }
    }

    int getClickCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_total_clicks;
    }

private:
    // Скрываем конструктор
    SystemMonitor() {
        std::println("[Система]: Мониторинг инициализирован.");
    }
    ~SystemMonitor() = default;

    // 3. ДОБАВЛЯЕМ КОД: Внутренние переменные состояния
    int m_total_clicks = 0;
    std::vector<std::string> m_events;
    
    // Мьютекс ОБЯЗАТЕЛЬНО должен быть mutable, чтобы работать в const-методах
    mutable std::mutex m_mutex; 
};

#include <thread>

// Симулируем функцию, которая срабатывает при клике пользователя в UI
void onUserClick(const std::string& button_name) {
    // Получаем ссылку на наш единственный объект и вызываем его метод
    SystemMonitor::getInstance().logEvent("Пользователь нажал кнопку: " + button_name);
}

// Симулируем работу фонового сетевого воркера
void networkWorker() {
    SystemMonitor::getInstance().logEvent("Сетевой воркер: данные успешно отправлены.");
}

int main() {
    std::println("Старт главного приложения.");

    // Имитируем действия в программе (можно запускать и в потоках `std::jthread`)
    std::jthread t1([] { onUserClick("Купить"); });
    std::jthread t2([] { onUserClick("Корзина"); });
    std::jthread t3(networkWorker);

    // Дождемся завершения потоков
    t1.join();
    t2.join();
    t3.join();

    // В конце программы выводим отчет
    SystemMonitor::getInstance().printReport();

    return 0;
}
// Ленивая инициализация
