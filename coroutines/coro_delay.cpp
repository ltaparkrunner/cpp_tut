#include <chrono>
#include <iostream>
#include <coroutine>
#include <thread>
#include <future>

namespace Timer {
class AwaitableTimer {
    std::chrono::steady_clock::time_point _timepoint;
public:
    AwaitableTimer(unsigned int seconds) {
        _timepoint = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    }

    bool await_ready() { 
        return _timepoint <= std::chrono::steady_clock::now(); 
    }

    void await_suspend(std::coroutine_handle<> coroutine) {
        // РЕШЕНИЕ: Создаем поток, который подождет до нужной временной точки
        // и затем разбудит корутину через .resume()
        std::thread([this, coroutine]() {
            std::this_thread::sleep_until(_timepoint);
            coroutine.resume(); // Будим корутину!
        }).detach(); // Отсоединяем поток, чтобы он работал сам по себе
    }

    void await_resume() {}
};
}

// Простейший хэндлер для корутины, чтобы main мог дождаться её окончания
struct timerCoroutineHandler {
    struct promise_type {
        std::promise<void> _promise;
        timerCoroutineHandler get_return_object() { return { _promise.get_future() }; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() { _promise.set_value(); } // Сигнал, что корутина завершилась
        void unhandled_exception() { std::terminate(); }
    };
    std::future<void> _future;
};

timerCoroutineHandler timerCoroutine() {
    co_await Timer::AwaitableTimer(5); // Для теста уменьшим до 2 секунд
}

int main() {
    std::cout << "The simple timer coroutine. Start...\n";
    
    auto handler = timerCoroutine();
    
    // РЕШЕНИЕ: Ждем в main, пока корутина полностью не отработает
    handler._future.wait(); 
    
    std::cout << "End!\n";
    return 0;
}
