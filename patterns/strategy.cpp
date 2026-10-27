#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

// ==========================================
// 1. ИНТЕРФЕЙС СТРАТЕГИИ (Абстрактный сортировщик)
// ==========================================
class SortingStrategy {
public:
    virtual ~SortingStrategy() = default; // Обязательный виртуальный деструктор
    virtual void sort(std::vector<int>& dataset) const = 0;
};

// ==========================================
// 2. КОНКРЕТНЫЕ СТРАТЕГИИ (Алгоритмы)
// ==========================================
class QuickSortStrategy : public SortingStrategy {
public:
    void sort(std::vector<int>& dataset) const override {
        std::cout << "[Стратегия]: Сортировка методом QuickSort (для больших массивов)...\n";
        std::sort(dataset.begin(), dataset.end()); // Имитируем быструю сортировку
    }
};

class BubbleSortStrategy : public SortingStrategy {
public:
    void sort(std::vector<int>& dataset) const override {
        std::cout << "[Стратегия]: Сортировка методом BubbleSort (для маленьких/почти отсортированных массивов)...\n";
        // Имитируем простую пузырьковую сортировку
        for (size_t i = 0; i < dataset.size(); ++i) {
            for (size_t j = 0; j < dataset.size() - 1; ++j) {
                if (dataset[j] > dataset[j+1]) std::swap(dataset[j], dataset[j+1]);
            }
        }
    }
};

// ==========================================
// 3. КОНТЕКСТ (Класс, который использует стратегию)
// ==========================================
class DataManager {
private:
    std::vector<int> m_data;
    std::unique_ptr<SortingStrategy> m_strategy; // Указатель на текущую стратегию

public:
    void addNumber(int num) { m_data.push_back(num); }

    // Метод позволяет динамически менять стратегию в любой момент работы программы
    void setStrategy(std::unique_ptr<SortingStrategy> new_strategy) {
        m_strategy = std::move(new_strategy);
    }

    void executeSort() {
        if (!m_strategy) {
            std::cout << "[Ошибка]: Стратегия сортировки не задана!\n";
            return;
        }
        m_strategy->sort(m_data); // Вызов алгоритма через интерфейс
    }
};

// ==========================================
// 4. ИСПОЛЬЗОВАНИЕ
// ==========================================
int main() {
    DataManager manager;
    manager.addNumber(5);
    manager.addNumber(1);
    manager.addNumber(9);

    // Сценарий 1: Данных мало, выбираем простую стратегию BubbleSort
    manager.setStrategy(std::make_unique<BubbleSortStrategy>());
    manager.executeSort();

    // Сценарий 2: Пришли новые условия в runtime, меняем алгоритм на QuickSort
    manager.setStrategy(std::make_unique<QuickSortStrategy>());
    manager.executeSort();

    return 0;
}
