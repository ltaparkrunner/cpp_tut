#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false; // Флаг готовности данных (предикат)

// Функция Потребителя (ждет данные)
void worker_thread() {
    // 1. Блокируем мьютекс перед проверкой состояния
    std::unique_lock<std::mutex> lock(mtx);
    
    std::cout << "[Потребитель] Жду сигнал о готовности...\n";
    
    // 2. Засыпаем на условной переменной.
    // Передаем lock и лямбда-функцию (предикат) для защиты от ложных пробуждений.
    cv.wait(lock, []{ return ready; });
    
    // 3. После пробуждения мьютекс снова захвачен данным потоком, работаем с данными
    std::cout << "[Потребитель] Сигнал получен! Данные обработаны.\n";
}

// Функция Производителя (готовит данные)
void publisher_thread() {
    // Имитируем подготовку данных (например, загрузку файла)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    {
        // Изменяем разделяемые данные под защитой мьютекса
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
        std::cout << "[Производитель] Данные готовы. Отправляю сигнал...\n";
    } // lock_guard уничтожается здесь, освобождая мьютекс
    
    // 4. Будим один ожидающий поток
    cv.notify_one();
}

int main() {
    // Запускаем оба потока параллельно
    std::thread consumer(worker_thread);
    std::thread producer(publisher_thread);
    
    // Ждем завершения работы обоих потоков
    consumer.join();
    producer.join();
    
    std::cout << "[Main] Программа успешно завершена.\n";
    return 0;
}
