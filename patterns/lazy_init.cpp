#include <iostream>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <chrono>
#include <syncstream>

// Имитация тяжелого ресурса (например, подключение к БД или загрузка текстуры)
class HeavyResource {
public:
    HeavyResource() {
        std::osyncstream(std::cout) << "[Ресурс] Начат долгий процесс создания ресурса...\n";
        // Имитируем задержку создания в 1 секунду
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::osyncstream(std::cout) << "[Ресурс] Ресурс успешно и полностью создан!\n";
    }

    void use(int thread_id) const {
        std::osyncstream(std::cout) << "[Ресурс] Поток #" << thread_id << " успешно использует ресурс.\n";
    }
};

// Менеджер, управляющий ленивой инициализацией
class ResourceManager {
private:
    mutable std::once_flag resource_flag;
    mutable std::unique_ptr<HeavyResource> resource;

    // Закрытый метод для непосредственного создания объекта
    void init_resource() const {
        resource = std::make_unique<HeavyResource>();
    }

public:
//    ResourceManager() = default;
    ResourceManager(int i){
        std::osyncstream(std::cout) << "Создание ResourceManager: " << i << std::endl;
    }
    ResourceManager(){
        std::osyncstream(std::cout) << "Создание ResourceManager: " << std::endl;
    }

    // Метод, который безопасно вызывает ленивую инициализацию
    void do_work(int thread_id) const {
        std::osyncstream(std::cout) << "Поток #" << thread_id << " запросил доступ к работе.\n";
        
        // Гарантирует потокобезопасный вызов init_resource ровно один раз
        std::call_once(resource_flag, &ResourceManager::init_resource, this);
        
        // К этой строчке ресурс гарантированно создан любым из потоков
        resource->use(thread_id);
    }
};

int main() {
    // Включаем синхронизацию вывода для корректного отображения логов из разных потоков
    // std::ios_base::sync_with_stdio(false);
    std::ios_base::sync_with_stdio(true);

    ResourceManager manager{0};
    std::vector<std::thread> threads;

    std::osyncstream(std::cout) << "[Main] Запускаем 5 параллельных потоков...\n\n";

    // Создаем 5 потоков, которые одновременно обращаются к менеджеру
    for (int i = 1; i <= 5; ++i) {
        threads.emplace_back(&ResourceManager::do_work, &manager, i);
    }

    // Дожидаемся завершения работы всех потоков
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::osyncstream(std::cout) << "\n[Main] Все потоки завершили работу. Программа успешно выполнена.\n";
    return 0;
}
