#include <iostream>
#include <string>

class Logger {
public:
    // 1. Удаляем конструктор копирования и оператор присваивания,
    // чтобы нельзя было скопировать объект.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 2. Глобальная точка доступа
    static Logger& getInstance() {
        // Гарантированно создается один раз при первом вызове.
        // Начиная с C++11 — это абсолютно потокобезопасно.
        static Logger instance; 
        return instance;
    }

    void log(const std::string& message) {
        std::cout << "[LOG]: " << message << std::endl;
    }

private:
    // 3. Закрываем конструктор и деструктор в private зоне
    Logger() { std::cout << "Logger инициализирован.\n"; }
    ~Logger() = default;
};

int main() {
    // Вызов из любой части программы
    Logger::getInstance().log("Запуск системы.");
    Logger::getInstance().log("Всё работает штатно.");
    
    // Ошибка компиляции, если попытаться создать вручную:
    // Logger myLogger; 
    
    return 0;
}
