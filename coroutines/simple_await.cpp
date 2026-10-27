#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>

auto switch_to_new_thread(std::jthread& out)
{
  struct awaitable
  {
    std::jthread* p_out;
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h)
    {
      std::jthread& out = *p_out;
      if (out.joinable())
        throw std::runtime_error("Выходной параметр jthread не пустой");
      out = std::jthread([h] { h.resume(); });
      // Возможно неопределённое поведение: доступ к потенциально уничтоженному *this
      // std::cout << "Идентификатор нового потока: " << p_out->get_id() << '\n';
      std::cout << "Идентификатор нового потока: " << out.get_id() << '\n'; // это OK
    }
    void await_resume() {}
  };
  return awaitable{&out};
}

struct task
{
  struct promise_type
  {
    task get_return_object() { return {}; }
    std::suspend_never initial_suspend() { std::cout << "initial_suspend \n"; return {}; }
    std::suspend_never final_suspend() noexcept { std::cout << "final_suspend \n"; return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };
};

task resuming_on_new_thread(std::jthread& out)
{
  std::cout << "Сопрограмма запущена в потоке: " << std::this_thread::get_id() << '\n';
  co_await switch_to_new_thread(out);
  // ожидающий уничтожен здесь
  std::cout << "Сопрограмма возобновлена в потоке: " << std::this_thread::get_id() << '\n';
}

int main()
{
  std::jthread out;
  resuming_on_new_thread(out);
}
