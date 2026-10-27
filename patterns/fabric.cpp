#include <iostream>
#include <memory>
#include <string>

// 1. Абстрактный продукт (Интерфейс кнопки)
class Button {
public:
    virtual ~Button() = default; // Обязательный виртуальный деструктор!
    virtual void render() = 0;
};

// 2. Конкретные продукты
class WindowsButton : public Button {
public:
    void render() override { std::cout << "Отрисовка кнопки в стиле Windows\n"; }
};

class LinuxButton : public Button {
public:
    void render() override { std::cout << "Отрисовка кнопки в стиле Linux (GTK)\n"; }
};

// 3. Абстрактный Создатель (Фабрика)
class Dialog {
public:
    virtual ~Dialog() = default;

    // Тот самый Фабричный Метод
    virtual std::unique_ptr<Button> createButton() = 0;

    // Какая-то бизнес-логика, использующая продукт
    void renderWindow() {
        std::unique_ptr<Button> okButton = createButton();
        okButton->render();
    }
};

// 4. Конкретные Фабрики
class WindowsDialog : public Dialog {
public:
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<WindowsButton>();
    }
};

class LinuxDialog : public Dialog {
public:
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<LinuxButton>();
    }
};

int main() {
    std::unique_ptr<Dialog> myDialog;

    // Симулируем выбор платформы (например, через макросы или настройки)
    bool isWindows = true; 

    if (isWindows) {
        myDialog = std::make_unique<WindowsDialog>();
    } else {
        myDialog = std::make_unique<LinuxDialog>();
    }

    // Код ниже вообще ничего не знает про конкретную ОС!
    myDialog->renderWindow(); 

    return 0;
}
