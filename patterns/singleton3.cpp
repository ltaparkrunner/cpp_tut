#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <thread>

class Singleton {
private:
    // 1. Закрытый конструктор, чтобы предотвратить создание копий извне
    Singleton() {
        std::cout << "[Singleton] Создан единственный экземпляр.\n";
    }

    // 2. Удаляем конструктор копирования и оператор присваивания
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // Управляющие переменные для std::call_once
    static inline Singleton* instance = nullptr;
    static inline std::once_flag initInstanceFlag;

    // 3. Рабочие поля синглтона (полезная нагрузка)
    std::mutex dataMutex; // Защищает данные при одновременном чтении/записи
    int counter = 0;
    std::string settingName = "DefaultConfig";

public:
    // 4. Статический метод доступа к единственному экземпляру
    static Singleton& getInstance() {
        std::call_once(initInstanceFlag, []() {
            instance = new Singleton();
        });
        return *instance;
    }

    // 5. Рабочие методы для демонстрации логики
    void incrementCounter() {
        std::lock_guard<std::mutex> lock(dataMutex); // Защита поля counter
        ++counter;
    }

    int getCounter() {
        std::lock_guard<std::mutex> lock(dataMutex);
        return counter;
    }

    void setSettingName(const std::string& name) {
        std::lock_guard<std::mutex> lock(dataMutex);
        settingName = name;
    }

    std::string getSettingName() {
        std::lock_guard<std::mutex> lock(dataMutex);
        return settingName;
    }
};

// Функция, которую будут выполнять параллельные потоки
void workerTask(int id) {
    // Несколько потоков одновременно пытаются получить доступ к Singleton
    Singleton& s = Singleton::getInstance();
    
    s.incrementCounter();
    std::cout << "Поток #" << id << " видит имя настройки: " << s.getSettingName() << "\n";
}

int main() {
    std::cout << "--- Запуск main() ---\n";

    // Сначала настроим наш Singleton в главном потоке
    Singleton& mainConfig = Singleton::getInstance();
    mainConfig.setSettingName("ProductionConfig");

    // Создаем группу потоков, чтобы проверить потокобезопасность
    const unsigned numThreads = 4;
    std::vector<std::thread> threads;

    for (unsigned i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerTask, i + 1);
    }

    // Ждем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    // Проверяем финальное состояние рабочих полей
    std::cout << "\nФинальное значение счетчика: " << mainConfig.getCounter() << "\n";
    std::cout << "--- Завершение main() ---\n";

    return 0;
}
// RAII (Resource Acquisition Is Initialization — «Получение ресурса есть инициализация») 
