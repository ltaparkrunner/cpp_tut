#include <atomic>
#include <optional>
#include <iostream>
#include <thread>
#include <vector>

constexpr int MAX_THREADS = 4;

template <typename T>
class HazardPointerStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& data) : data(data), next(nullptr) {}
    };

    std::atomic<Node*> head{nullptr};
    std::atomic<Node*> hazard_pointers[MAX_THREADS];

public:
    // ПЕРЕНЕСЛИ В PUBLIC: Структура для отложенного удаления памяти
    struct ThreadLocalTrash {
        // Выделяем фиксированный буфер под узлы, ожидающие удаления
        Node* chunks[100]; 
        int count = 0;
    };

private:
    void reclaim_memory(int thread_id, ThreadLocalTrash& trash) {
        for (int i = 0; i < trash.count; ) {
            Node* node_to_delete = trash.chunks[i];
            bool is_hazardous = false;

            for (int hp_id = 0; hp_id < MAX_THREADS; ++hp_id) {
                if (hazard_pointers[hp_id].load(std::memory_order_acquire) == node_to_delete) {
                    is_hazardous = true;
                    break;
                }
            }

            if (!is_hazardous) {
                delete node_to_delete;
                trash.chunks[i] = trash.chunks[--trash.count];
            } else {
                ++i;
            }
        }
    }

public:
    HazardPointerStack() {
        for (int i = 0; i < MAX_THREADS; ++i) {
            hazard_pointers[i].store(nullptr, std::memory_order_relaxed);
        }
    }

    ~HazardPointerStack() {
        Node* current = head.load();
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    bool is_stack_lock_free() const noexcept {
        return head.is_lock_free();
    }

    void push(const T& data) {
        Node* new_node = new Node(data);
        new_node->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }

    // Теперь метод pop принимает только ID потока, а корзину мы берем из thread_local
    std::optional<T> pop(int thread_id) {
        // У каждого потока будет свой собственный экземпляр trash
        static thread_local ThreadLocalTrash trash;

        Node* old_head = head.load(std::memory_order_relaxed);
        
        while (old_head) {
            hazard_pointers[thread_id].store(old_head, std::memory_order_release);

            if (head.load(std::memory_order_acquire) != old_head) {
                old_head = head.load(std::memory_order_relaxed);
                continue; 
            }

            if (head.compare_exchange_weak(old_head, old_head->next,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
                break; 
            }
        }

        hazard_pointers[thread_id].store(nullptr, std::memory_order_release);

        if (old_head) {
            T result = old_head->data;
            
            if (trash.count < 100) {
                trash.chunks[trash.count++] = old_head;
            } else {
                // Если буфер переполнен, принудительно удаляем (Фолбэк)
                delete old_head;
            }
            
            reclaim_memory(thread_id, trash);
            return result;
        }

        return std::nullopt;
    }
};

int main() {
    HazardPointerStack<int> stack;

    std::cout << "=== Проверка нового Lock-Free статуса ===" << "\n";
    if (stack.is_stack_lock_free()) {
        std::cout << "[SUCCESS] Стек на сырых указателях честно LOCK-FREE на уровне процессора!\n";
    } else {
        std::cout << "[FAIL] Ошибка архитектуры.\n";
    }
    std::cout << "=========================================\n\n";

    // Больше не нужно вручную создавать структуры корзин в main!
    std::thread t1([&stack]() {
        for (int i = 0; i < 5; ++i) stack.push(i);
        for (int i = 0; i < 5; ++i) {
            if (auto val = stack.pop(0)) { // Передаем только уникальный ID потока (0)
                std::cout << "Поток 1 достал: " << *val << "\n";
            }
        }
    });

    std::thread t2([&stack]() {
        for (int i = 10; i < 15; ++i) stack.push(i);
        for (int i = 0; i < 5; ++i) {
            if (auto val = stack.pop(1)) { // Передаем только уникальный ID потока (1)
                std::cout << "Поток 2 достал: " << *val << "\n";
            }
        }
    });

    t1.join();
    t2.join();

    return 0;
}
