#include <coroutine>
#include <iostream>

// There is coroutine handle somewhere. The type which handle with coroutine.

class ReturnObject
{
    public:
        struct promise_type
        {
            // This method return object, that will return from coroutine
            ReturnObject get_return_object() { return {}; }
            // This method is called, when a coroutine is suspended when enter it.
            //      std::suspend_never initial_suspend() { return {}; }
            std::suspend_always initial_suspend() { return {}; }
            // This method is called, when a coroutine is suspended when exit it.
            std::suspend_never final_suspend() noexcept { return {}; }
            // This method is called, when a coroutine is suspended when finish it.
            void return_void() {}   // return_value(){...}
            // This method is called, when an exception happend in a coroutine.
            void unhandled_exception() {}
        };
};

ReturnObject ResumableFunc()
{
    std::cout << "Enter ResumableFunc\n";
    co_await std::suspend_always{};
    std::cout << "Exit ResumableFunc\n";
}

int main()
{
    auto result = ResumableFunc();
    std::cout << "Returned from ResumableFunc\n";
}