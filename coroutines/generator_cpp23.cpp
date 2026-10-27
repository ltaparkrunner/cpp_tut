#include <generator>
#include <iostream>

std::generator<int> Counter()
{
    for (int i=0; i<10; ++i){
        co_yield i;
    }
}

std::generator<int> CounterToThree() {
    co_yield 1;
    co_yield 2;
    co_yield 3;
}

class ReturnObject {
public:
    struct promise_type;
    using CoroHandle = std::coroutine_handle<promise_type>;
    using ch = std::coroutine_handle<promise_type>;
    ReturnObject(CoroHandle h) : m_handle(h) {
        std::cout << "ReturnObject(CoroHandle h) constructor: " << this << std::endl;
    }
    ReturnObject(ReturnObject&& other) noexcept
        : m_handle(std::exchange(other.m_handle, nullptr)) {
            std::cout << "ReturnObject(CoroHandle h) move: " << this << std::endl;
        }
    ~ReturnObject() {
        std::cout << "ReturnObject(CoroHandle h) destructor: " << this << std::endl;
        if (m_handle) m_handle.destroy();
    }
    bool Resume() {
        if (!m_handle.done()) m_handle.resume();
        return !m_handle.done();
    }
    struct promise_type {
        promise_type() {
            std::cout << "Promise_type constructor: " << this << std::endl;
        }
        promise_type(const promise_type&) = delete;
        promise_type& operator=(const promise_type&) = delete;
        ~promise_type() {
            std::cout << "Promise_type destructor" << std::endl;
        }
        // This method return object, that will be returned from coroutine.
//        ReturnObject get_return_object() { return {};}
        ReturnObject get_return_object() {
            std::cout << "get_return_object()" << std::endl;
        // We get a pointer to a coroutine handle and return ReturnObject
//            return { CoroHandle::from_promise(*this) };
            return {ch::from_promise(*this)};
        }
        // This method is called, then coroutine is suspended upon entering it.
//        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always initial_suspend() {
            std::cout << "initial_suspend()" << std::endl;
            return {}; }
        // This method is called when the coroutine is suspended upon exiting.
//        std::suspend_never final_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { 
            std::cout << "final_suspend()" << std::endl;
            return {}; }
        // This method is called, then coroutine is finishing its execution.
        void return_void() {
            std::cout << "return_void()" << std::endl;
        }
        // This method is called when an exception occurs in the coroutine.
        void unhandled_exception() {
            std::cout << "unhandled_exception()" << std::endl;
        }
    };
    CoroHandle m_handle;
};

void NestedFunc(){
    int i = 0;
    std::cout << "Enter the NestedFunc addr local i:" << &i << std::endl; 
}

ReturnObject ResumableFunc() {
    int i;
    std::cout << "Enter ResumableFunc" << std::endl;
    std::cout << "local variable addr: " << &i << std::endl;
    NestedFunc();
    co_await std::suspend_always{};
    std::cout << "Exit ResumableFunc local i addr: " << &i << std::endl;
    NestedFunc();
}

int main() {
    // for (auto number : Counter()) {
    //     std::cout << number << std::endl;
    // }
    // auto gen = CounterToThree();
    // for(auto it = gen.begin(); it != gen.end(); ++it)
    //     std::cout << *it << std::endl;
    int i = 0;
    std::cout << "The address of i: " << (&i) << std::endl;
    auto result = ResumableFunc();
    std::cout << "The address of result: " << (&result) << "  result.m_handle:  " 
    << result.m_handle.address() <<  std::endl;
    
    std::cout << "Returned from ResumableFunc" << std::endl;
    while (result.Resume())
        std::cout << "ResumableFunc resumed" << std::endl;
}

std::coroutine_handle hdl;
// it include in resume(), done(), destroy(), promise()
