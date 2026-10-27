#include <iostream>
#include <memory>
#include <string>

// ==========================================
// 1. ИНТЕРФЕЙС ПРОДУКТА (Абстрактное соединение)
// ==========================================
class Connection {
public:
    // Обязательный виртуальный деструктор для корректного удаления наследников
    virtual ~Connection() = default;
    
    virtual void connect() = 0;
    virtual void send(const std::string& data) = 0;
};

// ==========================================
// 2. КОНКРЕТНЫЕ ПРОДУКТЫ (Реализации соединений)
// ==========================================
class HttpConnection : public Connection {
public:
    void connect() override {
        std::cout << "[HTTP]: Установка соединения по протоколу HTTP (порт 80)...\n";
    }
    void send(const std::string& data) override {
        std::cout << "[HTTP]: Отправка POST-запроса: " << data << "\n";
    }
};

class FtpConnection : public Connection {
public:
    void connect() override {
        std::cout << "[FTP]: Установка соединения по протоколу FTP (порт 21)...\n";
    }
    void send(const std::string& data) override {
        std::cout << "[FTP]: Загрузка файла на сервер: " << data << "\n";
    }
};

// ==========================================
// 3. ИНТЕРФЕЙС СОЗДАТЕЛЯ (Абстрактная Фабрика)
// ==========================================
class ConnectionFactory {
public:
    virtual ~ConnectionFactory() = default;

    // Тот самый ФАБРИЧНЫЙ МЕТОД. 
    // Возвращает умный указатель unique_ptr, передавая владение вызывающему коду.
    virtual std::unique_ptr<Connection> createConnection() const = 0;

    // Бизнес-логика, которая использует созданный продукт, не зная его конкретного класса
    void sendData(const std::string& message) const {
        std::unique_ptr<Connection> conn = createConnection();
        conn->connect();
        conn->send(message);
    }
};

// ==========================================
// 4. КОНКРЕТНЫЕ СОЗДАТЕЛИ (Реализации фабрик)
// ==========================================
class HttpFactory : public ConnectionFactory {
public:
    std::unique_ptr<Connection> createConnection() const override {
        // std::make_unique безопасно выделяет память в стиле современного C++
        return std::make_unique<HttpConnection>();
    }
};

class FtpFactory : public ConnectionFactory {
public:
    std::unique_ptr<Connection> createConnection() const override {
        return std::make_unique<FtpConnection>();
    }
};

// ==========================================
// 5. КЛИЕНТСКИЙ КОД (Использование)
// ==========================================
int main() {
    std::cout << "--- Старт клиентского приложения ---\n\n";

    // Указатель на абстрактную фабрику
    std::unique_ptr<ConnectionFactory> currentFactory;

    // Имитируем чтение конфигурации программы (например, из файла настроек)
    std::string configProtocol = "FTP"; 

    if (configProtocol == "HTTP") {
        currentFactory = std::make_unique<HttpFactory>();
    } else if (configProtocol == "FTP") {
        currentFactory = std::make_unique<FtpFactory>();
    }

    // Вся остальная логика программы работает ТОЛЬКО через абстрактные интерфейсы.
    // Код ниже вообще ничего не знает про существование классов HttpConnection или FtpConnection!
    if (currentFactory) {
        currentFactory->sendData("Hello, World!");
    } else {
        std::cerr << "Неизвестный протокол в конфигурации!\n";
    }

    return 0; // Вся память очистится автоматически благодаря unique_ptr
}
