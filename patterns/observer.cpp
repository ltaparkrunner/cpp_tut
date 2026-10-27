#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

// ==========================================
// 1. ИНТЕРФЕЙС НАБЛЮДАТЕЛЯ (Подписчик)
// ==========================================
class Observer {
public:
    virtual ~Observer() = default;
    // Метод, который вызывается при обновлении состояния Издателя
    virtual void update(const std::string& message) = 0;
};

// ==========================================
// 2. ИЗДАТЕЛЬ (Субъект наблюдения)
// ==========================================
class NotificationService {
private:
    // Храним слабые указатели weak_ptr, чтобы избежать dangling pointers (висячих указателей),
    // если подписчик будет уничтожен где-то в другом месте программы.
    std::vector<std::weak_ptr<Observer>> m_observers;
    std::string m_latest_news;

public:
    // Подписать на уведомления
    void subscribe(std::shared_ptr<Observer> observer) {
        m_observers.push_back(observer);
    }

    // Изменить состояние и сразу уведомить всех
    void setNewMessage(const std::string& news) {
        m_latest_news = news;
        notifyObservers();
    }

private:
    void notifyObservers() {
        // Итерируемся по списку подписчиков
        for (auto it = m_observers.begin(); it != m_observers.end(); ) {
            // Пытаемся временно захватить владение объектом (превратить weak_ptr в shared_ptr)
            if (auto observer_ptr = it->lock()) {
                observer_ptr->update(m_latest_news);
                ++it;
            } else {
                // Если lock() вернул nullptr, значит подписчик уже удален из памяти!
                // Нам нужно почистить наш список от мертвых ссылок.
                it = m_observers.erase(it);
            }
        }
    }
};

// ==========================================
// 3. КОНКРЕТНЫЕ НАБЛЮДАТЕЛИ (Реализации)
// ==========================================
class ConsoleLogger : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "[Логгер]: Записано в системный лог -> " << message << "\n";
    }
};

class UIWidget : public Observer {
private:
    std::string m_name;
public:
    UIWidget(std::string name) : m_name(std::move(name)) {}
    
    void update(const std::string& message) override {
        std::cout << "[Экранный виджет " << m_name << "]: Обновлен текст на -> " << message << "\n";
    }
};

// ==========================================
// 4. ИСПОЛЬЗОВАНИЕ
// ==========================================
int main() {
    NotificationService service;

    // Создаем подписчиков через shared_ptr
    auto logger = std::make_shared<ConsoleLogger>();
    auto mainWidget = std::make_shared<UIWidget>("ГлавноеОкно");

    // Подписываем их
    service.subscribe(logger);
    service.subscribe(mainWidget);

    std::cout << "--- Первое событие ---\n";
    service.setNewMessage("Система успешно запущена.");

    std::cout << "\n--- Симуляция удаления объекта ---\n";
    {
        auto temporaryWidget = std::make_shared<UIWidget>("ВсплывающееУведомление");
        service.subscribe(temporaryWidget);
        service.setNewMessage("Появилось предупреждение о памяти.");
        // Здесь temporaryWidget выходит из области видимости и уничтожается автоматически!
    }

    std::cout << "\n--- Второе событие (после удаления временного виджета) ---\n";
    // Сервис автоматически поймет, что один подписчик умер, удалит его из списка и не упадет
    service.setNewMessage("Программа завершает работу.");

    return 0;
}
