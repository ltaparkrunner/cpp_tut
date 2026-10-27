#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>

const int MAX_BUFFER_SIZE = 10;
std::queue<int> buffer;
std::mutex mtx;
std::condition_variable cv_producer; // Notifies producer when space is available
std::condition_variable cv_consumer; // Notifies consumer when an item is available

void producer(int items_to_produce) {
    for (int i = 0; i < items_to_produce; ++i) {
        // Acquire lock
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait if the buffer is full
        cv_producer.wait(lock, []{ return buffer.size() < MAX_BUFFER_SIZE; });
        
        // Produce item and add to buffer
        buffer.push(i);
        std::cout << "Produced: " << i << " (Buffer size: " << buffer.size() << ")" << std::endl;
        
        // Release the lock and notify a waiting consumer
        lock.unlock();
        cv_consumer.notify_one();
        
        // Simulate some work time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Producer finished producing." << std::endl;
}

void consumer(int items_to_consume) {
    for (int i = 0; i < items_to_consume; ++i) {
        // Acquire lock
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait if the buffer is empty
        cv_consumer.wait(lock, []{ return !buffer.empty(); });
        
        // Consume item from buffer
        int data = buffer.front();
        buffer.pop();
        std::cout << "Consumed: " << data << " (Buffer size: " << buffer.size() << ")" << std::endl;
        
        // Release the lock and notify a waiting producer
        lock.unlock();
        cv_producer.notify_one();
        
        // Simulate some work time
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "Consumer finished consuming." << std::endl;
}

int main() {
    int total_items = 20;

    // Create producer and consumer threads
    std::thread producerThread(producer, total_items);
    std::thread consumerThread(consumer, total_items);

    // Join threads to wait for them to finish
    producerThread.join();
    consumerThread.join();

    std::cout << "All threads finished." << std::endl;

    return 0;
}
