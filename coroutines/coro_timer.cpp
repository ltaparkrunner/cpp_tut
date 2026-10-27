#include <iostream>
#include <chrono>
#include <coroutine>

namespace Timer {
  class AwaitableTimer {
  public:
    AwaitableTimer (unsigned int seconds);
    bool await_ready();
    void await_suspend(std::coroutine_handle<> coroutine);
    void await_resume();
  private:
    std::chrono::time_point<std::chrono::steady_clock> _timepoint;
    std::coroutine_handle<> _coroutine;
  };
}

namespace Timer {
  AwaitableTimer::AwaitableTimer (unsigned int seconds) {
    // std::cout << "AwaitableTimer ctor" << std::endl;
    _timepoint = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(_timepoint.time_since_epoch()).count();
    std::cout << "AwaitableTimer ctor _timepoint = " << sec << "sec\n";
  }

  bool AwaitableTimer::await_ready() {
    std::cout << "AwaitableTimer::await_ready" << std::endl;
    return _timepoint <= std::chrono::steady_clock::now();
  }
    
  void AwaitableTimer::await_suspend(std::coroutine_handle<> coroutine) {
    std::cout << "AwaitableTimer::await_suspend" << std::endl;
    _coroutine = coroutine;
  }
  void AwaitableTimer::await_resume() {
    std::cout << "AwaitableTimer::await_resume" << std::endl;
  }
}

namespace Coroutine {
    struct promise;
}

struct timerCoroutineHandler : std::coroutine_handle<Coroutine::promise> {
  using promise_type = Coroutine::promise;
};

namespace Coroutine {
    struct promise {
        timerCoroutineHandler get_return_object() {
            std::cout << "promise::get_return_object" << std::endl;
            return { timerCoroutineHandler::from_promise(*this) };
        }
        std::suspend_never initial_suspend() noexcept {
            std::cout << "promise::initial_suspend" << std::endl;
            return {};
        }
        std::suspend_never final_suspend() noexcept {
            std::cout << "promise::final_suspend" << std::endl;
            return {};
        }
        void return_void() {
            std::cout << "promise::return_void" << std::endl;
        }
        void unhandled_exception() {
            std::cout << "promise::unhandled_exception" << std::endl;
        }
    };
}

timerCoroutineHandler timerCoroutine() {
    co_await Timer::AwaitableTimer(10);
}

int main()
{
    std::cout << "The simple timer coroutine. Time now " << std::chrono::system_clock::now() << "\n";
    timerCoroutine();
    return 0;
}
