#include <iostream>
#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>
#include <cassert>

// Идеальная структура для DWCAS: ровно 16 байт (два поля по 8 байт)
// alignas(16) гарантирует, что структура лежит в памяти по адресу, кратному 16.
struct alignas(16) Data16Byte {
    std::uint64_t low;
    std::uint64_t high;

    // Оператор сравнения необходим для работы некоторых методов std::atomic
    bool operator==(const Data16Byte& other) const noexcept {
        return low == other.low && high == other.high;
    }
};

int main() {
    // Обертываем нашу 16-байтную структуру в std::atomic
    std::atomic<Data16Byte> atomic_target;

    std::cout << "=== Тестирование аппаратного DWCAS (16-байт атомиков) ===\n\n";

    // 1. Проверка времени компиляции (Compile-time check)
    std::cout << "1. Проверка на уровне компилятора (is_always_lock_free):\n";
    if constexpr (std::atomic<Data16Byte>::is_always_lock_free) {
        std::cout << "   [SUCCESS] Компилятор гарантирует чистый аппаратный LOCK-FREE для 16 байт!\n";
    } else {
        std::cout << "   [INFO] Компилятор не гарантирует lock-free на этапе сборки (требуется флаг -mcx16).\n";
    }

    // 2. Проверка времени выполнения (Runtime check)
    std::cout << "\n2. Проверка в рантайме (is_lock_free()):\n";
    if (atomic_target.is_lock_free()) {
        std::cout << "   [SUCCESS] Процессор ИСПОЛЬЗУЕТ аппаратные инструкции для 16-байт атомиков!\n";
    } else {
        std::cout << "   [WARNING] Рантайм использует внутренние мьютексы библиотеки libatomic.\n";
    }

    // 3. Функциональный тест: Проверяем работу атомарного обмена (Compare-and-Swap)
    std::cout << "\n3. Тестирование работы инструкции CAS над 16 байтами:\n";
    
    Data16Byte initial = {100, 200};
    Data16Byte new_value = {999, 888};
    
    atomic_target.store(initial, std::memory_order_relaxed);
    
    // Пытаемся сделать CAS. Ожидаем успех, так как текущее значение совпадает с 'initial'
    Data16Byte expected = initial;
    bool cas_success = atomic_target.compare_exchange_strong(expected, new_value, 
                                                            std::memory_order_acq_rel, 
                                                            std::memory_order_relaxed);
    
    if (cas_success) {
        Data16Byte current = atomic_target.load(std::memory_order_relaxed);
        std::cout << "   [SUCCESS] CAS прошел успешно!\n";
        std::cout << "   Новые значения: low = " << current.low << ", high = " << current.high << "\n";
    } else {
        std::cout << "   [FAIL] CAS не сработал.\n";
    }

    // 4. Многопоточный стресс-тест для DWCAS
    std::cout << "\n4. Запуск конкурентного изменения (8 потоков борются за CAS):\n";
    
    atomic_target.store({0, 0}, std::memory_order_release);
    const int num_threads = 8;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&atomic_target, i]() {
            Data16Byte expected = atomic_target.load(std::memory_order_relaxed);
            // Каждый поток пытается записать свой ID в high и инкрементировать low
            for (int step = 0; step < 10000; ++step) {
                Data16Byte desired = {expected.low + 1, static_cast<std::uint64_t>(i)};
                // Цикл CAS (типичный lock-free паттерн)
                while (!atomic_target.compare_exchange_weak(expected, desired,
                                                           std::memory_order_acq_rel,
                                                           std::memory_order_relaxed)) {
                    // Если не получилось, expected автоматически обновился текущим значением
                    desired = {expected.low + 1, static_cast<std::uint64_t>(i)};
                }
                expected = desired;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    Data16Byte final_result = atomic_target.load(std::memory_order_acquire);
    std::cout << "   Все потоки завершили работу.\n";
    std::cout << "   Итоговый low (ожидается 80000): " << final_result.low << "\n";
    std::cout << "   Последний победивший поток (ID): " << final_result.high << "\n";
    
    assert(final_result.low == 80000);
    std::cout << "   [SUCCESS] Данные не повреждены (Data race отсутствует)!\n";

    return 0;
}
