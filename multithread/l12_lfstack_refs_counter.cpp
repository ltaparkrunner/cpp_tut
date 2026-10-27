#include <thread>
#include <atomic>
#include <stdexcept>
#include <functional>
#include <iostream>

template<typename T>
class LockFreeStack {
public:
    ~LockFreeStack() {
        for (auto h = m_head.load(); h;)
            h = std::exchange(h->next, nullptr);
            // or delete std::exchange(h, h->next);
    }
    void Push(const T& data) {
        PushImpl(data);
    }

    void Push(T&& data) {
        PushImpl(std::move(data));
    }

    template <typename U>
    void PushImpl(U&& data) {
        std::shared_ptr<Node> const newNode = 
            std::make_shared<Node>(std::forward<U>(data));
        newNode->next = m_head.load();
        while(!m_head.compare_exchange_weak(newNode->next, newNode))
        ;
    }

    std::unique_ptr<T> Pop() noexcept
    {
        auto oldHead = m_head.load();
        while (oldHead != nullptr 
            && !m_head.compare_exchange_weak(oldHead, oldHead->next))
            ;
        if (oldHead) {
            oldHead->next = std::shared_ptr<Node>();
            return std::move(oldHead->data);
        }   
        return {};
    }



private:
    struct Node {
        std::unique_ptr<T> data;
        std::shared_ptr<Node> next;
        Node(T&& d)
            : data(std::make_unique<T>(std::move(d))) {}
        Node(const T& d)
            : data(std::make_unique<T>(d)) {}
    };
    std::atomic<std::shared_ptr<Node>> m_head = nullptr;
};

int main() {
    std::shared_ptr<float> stack;
    std::atomic<std::shared_ptr<float>> stack2;
    // std::make_shared(LockTreeStack<float>);
    std::cout << std::boolalpha;
    std::cout << "atomic<int> is always lock free:" 
        << std::atomic<int>::is_always_lock_free << std::endl;
    std::cout << "is_shared_ptr_lock_free? " << stack2.is_lock_free() << std::endl;
}