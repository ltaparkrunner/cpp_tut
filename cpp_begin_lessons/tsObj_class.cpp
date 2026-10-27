#include <mutex>

class TSObject {
    public:
        int GetIntData() const {
            std::lock_guard lk(m_mutex);
            return m_intData;   
        }
        void SetIntData(int data) {
            std::lock_guard lk(m_mutex);
            m_intData = data;
        }
        std::string GetStringData() const {
            std::lock_guard lk(m_mutex);
            return m_stringData;   
        }
        void SetStringData(std::string data) {
            std::lock_guard lk(m_mutex);
            m_stringData = std::move(data);
        }
    private:
        mutable std::mutex m_mutex;
        int m_intData=0;
        std::string m_stringData;
};

#include <shared_mutex>
class TSCounter {
    public:
        int Get() const {
            std::shared_lock lk(m_mutex);
            return m_value;
        }
        void Increment() {
            std::unique_lock lk(m_mutex);
            ++m_value;
        }
        void Decrement() {
            std::unique_lock lk(m_mutex);
            --m_value;
        }
        void Reset() {
            std::unique_lock lk(m_mutex);
            m_value = 0;
        }
    private:
        mutable std::shared_mutex m_mutex;
        int m_value = 0;
};


#include <iostream>
#include <stack>
#include <mutex>
#include <thread>
#include <memory>
#include <exception>

// A custom exception for an empty stack
struct empty_stack : std::exception {
    const char* what() const noexcept override {
        return "Stack is empty";
    }
};

template<typename T>
class ThreadSafeStack {
private:
    std::stack<T> data;
    mutable std::mutex m_mutex; // Mutex to protect the shared data

public:
    ThreadSafeStack() = default;
    ~ThreadSafeStack() = default;

    // Disable copy constructor and assignment operator for simplicity
    ThreadSafeStack(const ThreadSafeStack&) = delete;
    ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex
        data.push(std::move(new_value)); // Use std::move for efficiency
        // Mutex is automatically unlocked when lock_guard goes out of scope
    }

    // A pop operation that retrieves the value and removes it
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex
        if (data.empty()) {
            throw empty_stack(); // Throw exception if empty
        }
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
        data.pop();
        return res; // Return value via std::shared_ptr for safe memory management
    }

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
};

// Example Usage (optional)
void worker_push(ThreadSafeStack<int>& stack) {
    for (int i = 0; i < 5; ++i) {
        stack.push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void worker_pop(ThreadSafeStack<int>& stack) {
    for (int i = 0; i < 5; ++i) {
        try {
            std::shared_ptr<int> value = stack.pop();
            std::cout << "Popped value: " << *value << std::endl;
        } catch (const empty_stack& e) {
            std::cout << "Stack is empty, sleeping..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

// int main() {
//     ThreadSafeStack<int> stack;
//     std::thread t1(worker_push, std::ref(stack));
//     std::thread t2(worker_pop, std::ref(stack));

//     t1.join();
//     t2.join();

//     return 0;
// }
#include <optional>
template<typename T>
class TSStack{
public:
    TSStack() = default;
    TSStack(const TSStack& other) {
        std::shared_lock lk(other.m_mutex);
        m_stack = other.m_stack;
    }
    TSStack& operator=(const TSStack&) = delete;

    void Push(T value) {
        std::unique_lock lk(m_mutex);
        m_stack.push(std::move(value));
    }
    bool IsEmpty() const {
        std::shared_lock lk(m_mutex);
        return m_stack.empty();
    }
    bool TryPop(T& out) {
        //std::unique_lock lk(m_mutex);
        std::lock_guard lk(m_mutex);
        if (m_stack.empty()) {
            // throw std::runtime_error("Stack is empty");
            return false;
        }
        if constexpr (std::is_move_assignable_v<T>) {
            //m_stack.top() = T(); // Clear the top element before popping
            out = std::move(m_stack.top());
        }
        else out = m_stack.top();
        m_stack.pop();
        return true;
    }

    std::unique_ptr<T> TryPop() {
        std::lock_guard lk(m_mutex);
        if (m_stack.empty()) {
            return nullptr;
        }
        std::unique_ptr<T> value;
        if constexpr (std::is_move_constructible_v<T>) {
            value = std::make_unique<T>(std::move(m_stack.top()));
        }
        else value = std::make_unique<T>(m_stack.top());
        m_stack.pop();
        return value;
    }
    T GetTop() const {
        std::shared_lock lk(m_mutex);
        if (m_stack.empty()) {
            //throw std::runtime_error("Stack is empty");
            throw std::logic_error("Stack is empty");
        }
        return m_stack.top();
    }   
    bool TryGetTop(T& value) const {
        std::unique_lock lk(m_mutex);
        if (m_stack.empty()) {
            return false;
        }
        value = m_stack.top();
        //m_stack.pop();
        return true;
    }
    std::optional<T> TryGetTop() const {
        std::unique_lock lk(m_mutex);
        if (m_stack.empty()) {
            return std::nullopt;
        }
        return m_stack.top();
    }   
private:
    mutable std::shared_mutex m_mutex;  
    std::stack<T> m_stack;
};

// std::this_thread::sleep_for(std::chrono::milliseconds(10));

// Conditional variable example
#include <condition_variable>

int main() {
    std::mutex mtx;
    std::condition_variable cv;
    bool flag = false;

    // std::jthread worker([&]() {
    //     std::unique_lock lk(mtx);
    //     cv.wait(lk, [&] { return ready; }); // Wait until ready is true
    //     std::cout << "Worker thread is processing..." << std::endl;
    // });

    std::jthread waitingThread{[&mtx, &flag, &cv]{
        long long checkCounter = 0;
        std::unique_lock lk(mtx);
        cv.wait(lk, [&flag, &checkCounter] { checkCounter++; return flag; }); // Wait until flag is true
        std::cout << "Waiting completed!" << "  checkCounter = " << checkCounter << std::endl;
    }};

    // Simulate some work in the main thread
    std::this_thread::sleep_for(std::chrono::seconds(15));
    {
        std::lock_guard lk(mtx);
        flag = true; // Set ready to true to wake up the worker thread
    }
    cv.notify_one(); // Notify the worker thread

//    waitingThread.join(); // Wait for the worker thread to finish
    return 0;
}
