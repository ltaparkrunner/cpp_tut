#include <iostream>
#include <memory>
#include <string>

// =================================================================
// 1. СОВРЕМЕННЫЙ ИНТЕРФЕЙС (То, что наше приложение ожидает увидеть)
// =================================================================
class ModernPrinter {
public:
    virtual ~ModernPrinter() = default;
    // Наше приложение привыкло передавать обычные JSON/строковые данные
    virtual void printDocument(const std::string& text) = 0;
};

// =================================================================
// 2. СТАРАЯ (LEGACY) БИБЛИОТЕКА (То, что у нас есть, но интерфейс не совпадает)
// =================================================================
class LegacyPrinter {
public:
    // Старый метод требует сначала открыть буфер, передать массив char, а затем закрыть
    void openSpooler() { std::cout << "[Legacy System]: Буфер печати открыт.\n"; }
    
    void printRawChars(const char* bytes, size_t length) {
        std::cout << "[Legacy System]: Печать " << length << " байт: ";
        for(size_t i = 0; i < length; ++i) {
            std::cout << bytes[i];
        }
        std::cout << "\n";
    }
    
    void closeSpooler() { std::cout << "[Legacy System]: Буфер печати закрыт.\n"; }
};

// =================================================================
// 3. АДАПТЕР (Связующее звено)
// =================================================================
// Он притворяется современным принтером, но внутри управляет старым
class PrinterAdapter : public ModernPrinter {
private:
    // Используем композицию: храним старый принтер внутри
    std::unique_ptr<LegacyPrinter> m_legacy_printer;

public:
    PrinterAdapter() {
        m_legacy_printer = std::make_unique<LegacyPrinter>();
    }

    // Реализуем современный метод, переводя его на язык старой библиотеки
    void printDocument(const std::string& text) override {
        std::cout << "[Адаптер]: Получена строка. Перевожу для старой системы...\n";
        
        // Переводим один вызов в последовательность низкоуровневых команд
        m_legacy_printer->openSpooler();
        m_legacy_printer->printRawChars(text.c_str(), text.length());
        m_legacy_printer->closeSpooler();
    }
};

// =================================================================
// 4. ИСПОЛЬЗОВАНИЕ
// =================================================================
int main() {
    std::cout << "--- Запуск приложения ---\n";

    // Наше приложение думает, что работает с обычным современным принтером
    std::unique_ptr<ModernPrinter> printer = std::make_unique<PrinterAdapter>();

    // Мы просто шлем строку, не задумываясь о внутренностях старого кода
    printer->printDocument("Важный системный отчет.");

    return 0;
}
