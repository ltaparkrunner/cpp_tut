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

    std::atomic<std::shared_ptr<Node>> head{nullptr};

public:
    // Метод для рантайм-проверки
    bool check_if_lock_free() const noexcept {
        return head.is_lock_free();
    }

    void push(const T& data) {
        auto new_node = std::make_shared<Node>(data);
        new_node->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }

    std::optional<T> pop() {
        std::shared_ptr<Node> old_head = head.load(std::memory_order_acquire);
        while (old_head && !head.compare_exchange_weak(old_head, old_head->next,
                                                       std::memory_order_release,
                                                       std::memory_order_acquire));
        if (old_head) {
            return old_head->data;
        }
        return std::nullopt;
    }
};

int main() {
    LockFreeStackShared<int> stack;

    std::cout << "=== Проверка Lock-Free статуса ===" << "\n";
    
    // 1. Рантайм проверка конкретного объекта
    if (stack.check_if_lock_free()) {
        std::cout << "[SUCCESS] std::atomic<std::shared_ptr> на этой платформе ЧЕСТНО LOCK-FREE!\n";
    } else {
        std::cout << "[WARNING] Платформа эмулирует атомарность shared_ptr через блокировки (мьютексы/спин-локи).\n";
    }

    // 2. Статическая проверка для обычных атомарных указателей (встроенное свойство)
    std::cout << "Статус обычных атомарных указателей (0 - никогда, 1 - иногда, 2 - всегда): " 
              << ATOMIC_POINTER_LOCK_FREE << "\n";
    std::cout << "==================================\n\n";

    // Простой тест работы
    stack.push(42);
    stack.push(43);
    if (auto val = stack.pop()) std::cout << "Успешно извлечено: " << *val << "\n";
    if (auto val = stack.pop()) std::cout << "Успешно извлечено: " << *val << "\n";

    return 0;
}
