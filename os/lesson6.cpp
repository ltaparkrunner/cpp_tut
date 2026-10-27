#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>
#include <semaphore>

class Queue {
public:
    explicit Queue(unsigned capacity): m_emptySlots(capacity) {}

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void Push(int value) {
        m_emptySlots.acquire();
        {
            std::lock_guard lock{ m_mut };
            m_queue.push_back(value);
        }
        m_usedSlots.release();
    }

    int Pop() {
        m_usedSlots.acquire();
        std::unique_lock lock{ m_mut };
        int value = m_queue.front();
        m_queue.pop_front();
        lock.unlock();
        m_emptySlots.release();
        return value;
    }
private:
    std::counting_semaphore<256> m_emptySlots{255};
    std::counting_semaphore<256> m_usedSlots{0};
    std::mutex m_mut;
    std::deque<int> m_queue;
};

int main() {
    std::binary_semaphore event{0};
    std::jthread worker{[&event]{
        std::cout << "Worker: waiting for event..." << std::endl;
        event.acquire();
        std::cout << "Worker: event received." << std::endl;
    }};

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Main: signaling event." << std::endl;
    event.release();
    worker.join();
    return 0;
}