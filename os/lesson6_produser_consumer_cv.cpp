#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

const int MAX_BUFFER_SIZE = 10;
std::queue<int> buffer;
std::mutex mtx;
// Use two condition variables: one for when the buffer is not full, one for when it's not empty.
std::condition_variable buffer_not_full;
std::condition_variable buffer_not_empty;

void producer(int id) {
    for (int i = 0; i < 20; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait if the buffer is full
        buffer_not_full.wait(lock, []{ return buffer.size() < MAX_BUFFER_SIZE; });
        
        // Produce item
        buffer.push(i);
        std::cout << "Producer " << id << " produced " << i << ". Buffer size: " << buffer.size() << std::endl;
        
        // Unlock before notification for potentially better performance, or rely on unique_lock destructor
        lock.unlock(); 

        // Notify one consumer that the buffer is no longer empty
        buffer_not_empty.notify_one();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(int id) {
    for (int i = 0; i < 20; ++i) {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait if the buffer is empty
        buffer_not_empty.wait(lock, []{ return !buffer.empty(); });

        // Consume item
        int item = buffer.front();
        buffer.pop();
        std::cout << "Consumer " << id << " consumed " << item << ". Buffer size: " << buffer.size() << std::endl;

        // Unlock
        lock.unlock();

        // Notify one producer that the buffer is no longer full
        buffer_not_full.notify_one();

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() {
    std::thread prod1(producer, 1);
    std::thread cons1(consumer, 1);

    prod1.join();
    cons1.join();

    return 0;
}
