#include <atomic>
#include <optional>
#include <iostream>
#include <thread>
#include <vector>

template <typename T>
class SplitRefCountStack {
private:
    struct Node;

    // Структура внешнего счетчика (упакована для Double-Width CAS)
    struct alignas(16) counted_node_ptr {
        std::int64_t external_counters; // Внешний счетчик обращений (чтений)
        Node* ptr;             // Указатель на узел
    };

    struct Node {
        T data;
        std::atomic<int> internal_count; // Внутренний счетчик ссылок
        counted_node_ptr next;

        Node(const T& data_) 
            : data(data_), internal_count(0), next{0, nullptr} {}
    };

    // Вершина стека. Должна занимать 16 байт и выравниваться по границе 16 байт
    alignas(16) std::atomic<counted_node_ptr> head;

    // Вспомогательная функция уменьшения счетчиков и удаления памяти
    void decrease_ref_count(Node* old_node) {
        // Уменьшаем внутренний счетчик на 1
        if (old_node->internal_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // Если мы были последними, кто держал этот узел — удаляем память
            delete old_node;
        }
    }

public:
    SplitRefCountStack() {
        head.store({0, nullptr}, std::memory_order_relaxed);
    }

    ~SplitRefCountStack() {
        // Очистка при деструкции
        counted_node_ptr current = head.load();
        while (current.ptr) {
            Node* next_ptr = current.ptr->next.ptr;
            delete current.ptr;
            current.ptr = next_ptr;
        }
    }

    // Проверка рантайм-статуса
    bool is_lock_free() const noexcept {
        return head.is_lock_free();
    }

    void push(const T& data) {
        counted_node_ptr new_node;
        new_node.ptr = new Node(data);
        new_node.external_counters = 1; // 1 — потому что на него указывает head

        new_node.ptr->next = head.load(std::memory_order_relaxed);
        
        while (!head.compare_exchange_weak(new_node.ptr->next, new_node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }

    std::optional<T> pop() {
        counted_node_ptr old_head = head.load(std::memory_order_relaxed);
        
        while (true) {
            // Шаг 1: Увеличиваем внешний счетчик, "бронируя" узел для чтения
            counted_node_ptr new_head;
            new_head.ptr = old_head.ptr;
            new_head.external_counters = old_head.external_counters + 1;

            if (!old_head.ptr) {
                return std::nullopt; // Стек пуст
            }

            // Пытаемся атомарно увеличить внешний счетчик в head
            if (head.compare_exchange_weak(old_head, new_head,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
                // Успешно забронировали узел, теперь его никто не удалит!
                old_head = new_head;
                break;
            }
            // Если CAS не удался, старый head обновился новым значением, пробуем снова
        }

        Node* ptr = old_head.ptr;
        
        // Шаг 2: Пытаемся извлечь узел из стека (сдвинуть head)
        while (ptr) {
            if (head.compare_exchange_weak(old_head, ptr->next,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
                // МЫ выиграли гонку и забрали элемент!
                T result = ptr->data;

                // Добавляем к внутреннему счетчику все внешние просмотры, которые мы зафиксировали
                // Минус 2 (один за push/указатель из head, второй — наше текущее чтение)
                int count_increase = old_head.external_counters - 2;
                
                if (ptr->internal_count.fetch_add(count_increase, std::memory_order_acq_rel) == -count_increase) {
                    // Если сумма обнулилась, узел больше никто не читает, удаляем
                    delete ptr;
                }
                return result;
            } 
            
            // Если сдвинуть head не получилось (кто-то другой сделал pop быстрее нас),
            // значит мы стали просто "читателем", который не забрал узел.
            // Уменьшаем счетчик этого узла.
            decrease_ref_count(ptr);
            break;
        }
        
        return std::nullopt;
    }
};

int main() {
    SplitRefCountStack<int> stack;

    std::cout << "=== Проверка DWCAS ===" << "\n";
    if (stack.is_lock_free()) {
        std::cout << "[SUCCESS] Указатель lock-free в рантайме.\n";
    } else {
        std::cout << "[WARNING] Используются внутренние блокировки libatomic.\n";
    }
    std::cout << "======================\n\n";

    const int num_producers = 8;
    const int num_consumers = 8;
    const int items_per_thread = 100000;

    std::vector<std::thread> threads;
    
    auto start_time = std::chrono::high_resolution_clock::now();

    // Запускаем 8 потоков-продюсеров (суммарно 800 000 push)
    for (int i = 0; i < num_producers; ++i) {
        threads.emplace_back([&stack]() {
            for (int j = 0; j < items_per_thread; ++j) {
                stack.push(j);
            }
        });
    }

    // Запускаем 8 потоков-консьюмеров (суммарно 800 000 pop)
    std::atomic<int> total_popped{0};
    for (int i = 0; i < num_consumers; ++i) {
        threads.emplace_back([&stack, &total_popped]() {
            int popped_count = 0;
            while (popped_count < items_per_thread) {
                if (stack.pop()) {
                    popped_count++;
                }
            }
            total_popped += popped_count;
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "=== Результаты стресс-теста ===" << "\n";
    std::cout << "Всего успешно извлечено элементов: " << total_popped.load() << "\n";
    std::cout << "Время выполнения: " << duration << " мс\n";
    std::cout << "===============================\n";

    return 0;
}

