#include <atomic>
#include <memory>
#include <optional>
#include <iostream>
#include <thread>
#include <vector>

template <typename T>
class LockFreeStackShared {
private:
    struct Node {
        T data;
        std::shared_ptr<Node> next;
        Node(const T& data) : data(data), next(nullptr) {}
    };

    // Атомарный shared_ptr на вершину стека (C++20)
    std::atomic<std::shared_ptr<Node>> head{nullptr};

public:
    void push(const T& data) {
        auto new_node = std::make_shared<Node>(data);
        
        // Загружаем текущую вершину
        new_node->next = head.load(std::memory_order_relaxed);
        
        // Пытаемся атомарно подменить head.
        // Если head изменился в другом потоке, compare_exchange обновит new_node->next 
        // актуальным значением head и повторит попытку.
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }

    std::optional<T> pop() {
        std::shared_ptr<Node> old_head = head.load(std::memory_order_acquire);
        
        // Пытаемся сдвинуть head на head->next
        while (old_head && !head.compare_exchange_weak(old_head, old_head->next,
                                                       std::memory_order_release,
                                                       std::memory_order_acquire));
        
        // Если мы успешно забрали узел, возвращаем данные.
        // Память узла Node удалится автоматически, когда old_head выйдет из области видимости,
        // при условии, что ни один другой поток не держит копию этого узла.
        if (old_head) {
            return old_head->data;
        }
        
        return std::nullopt;
    }
};

int main() {
    LockFreeStackShared<int> stack;

    // Простой тест: один поток пишет, другой читает
    std::thread t1([&stack]() {
        for (int i = 0; i < 5; ++i) {
            stack.push(i);
        }
    });

    std::thread t2([&stack]() {
        for (int i = 0; i < 5; ++i) {
            auto val = stack.pop();
            if (val) {
                std::cout << "Popped: " << *val << "\n";
            } else {
                --i; // Ждем элементы
            }
        }
    });

    t1.join();
    t2.join();
    return 0;
}
