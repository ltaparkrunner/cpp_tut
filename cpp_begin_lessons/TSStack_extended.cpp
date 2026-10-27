#include <iostream>
#include <stack>
#include <mutex>
#include <thread>
#include <memory>
#include <exception>
#include <optional>

// A custom exception for an empty stack
// struct empty_stack : std::exception {
//     const char* what() const noexcept override {
//         return "Stack is empty";
//     }
// };

template<typename T>
class TSStack {
public:
    void Push(T new_value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex
            data.push(std::move(new_value)); // Use std::move for efficiency
            // Mutex is automatically unlocked when lock_guard goes out of scope
        }
        m_dataCond.notify_one();
    }

    void WaitAndPop(T& out) {
        std::unique_lock lk{ m_mutex };
        m_dataCond.wait(lk, [this] { return !m_stack.empty();})
        out = m_stack.top();
        m_stack.pop();
    }

    std::unique_ptr<T> WaitAndPop() {
        std::unique_lock lk{ m_mutex };
        m_dataCond.wait(lk, [this] { return !m_stack.empty();})
        std::unique_ptr<T> value;
        if constexpr (std::is_nothrow_move_constructible_v<T>)
            value = std::make_unique<T>(std::move(m_stack.top()));
        else
            value = std::make_unique<T>(m_stack.top());
        m_stack.pop();
        return value;
    }
    ThreadSafeStack() = default;
    ~ThreadSafeStack() = default;

    // Disable copy constructor and assignment operator for simplicity
    ThreadSafeStack(const ThreadSafeStack&) = delete;
    ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

    // A pop operation that retrieves the value and removes it
    // std::shared_ptr<T> pop() {
    //     std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex
    //     if (data.empty()) {
    //         throw empty_stack(); // Throw exception if empty
    //     }
    //     std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
    //     data.pop();
    //     return res; // Return value via std::shared_ptr for safe memory management
    // }

    // An alternative pop operation that takes a reference and returns void
    void pop(T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (data.empty()) {
            throw empty_stack();
        }
        value = std::move(data.top());
        data.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex); // Lock for read access
        return data.empty();
    }

private:
    mutable std::shared_mutex m_mutex;  
    std::stack<T> m_stack;
    std::conditional_variable_any m_dataCond;
};
// int main() {
//     ThreadSafeStack<int> stack;
//     std::thread t1(worker_push, std::ref(stack));
//     std::thread t2(worker_pop, std::ref(stack));

//     t1.join();
//     t2.join();

//     return 0;
// }
