#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx; // Mutex to protect the shared data
std::condition_variable cv; // Condition variable for synchronization
bool data_ready = false; // The shared condition/flag

void consumer_thread() {
    std::cout << "Consumer: Waiting for data to be ready..." << std::endl;
    std::unique_lock<std::mutex> lck(mtx); // Acquire lock

    // Wait until the condition (data_ready) is true.
    // The wait() function atomically releases the lock and puts the thread to sleep.
    // When notified, it wakes up, reacquires the lock, and checks the condition.
    cv.wait(lck, []{ return data_ready; });

    std::cout << "Consumer: Data is ready! Proceeding with work." << std::endl;
    // The lock is automatically released when lck goes out of scope (or by unique_lock's flexibility).
}

void producer_thread() {
    std::cout << "Producer: Preparing data..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate work

    {
        std::lock_guard<std::mutex> lck(mtx); // Lock the mutex to modify shared data safely
        data_ready = true; // Set the condition
        std::cout << "Producer: Data prepared." << std::endl;
    } // Lock guard goes out of scope here, releasing the mutex

    cv.notify_one(); // Notify one waiting thread (the consumer)
    std::cout << "Producer: Notification sent." << std::endl;
}

int main() {
    std::thread t1(consumer_thread);
    std::thread t2(producer_thread);

    t1.join();
    t2.join();

    std::cout << "Main: All threads finished." << std::endl;

    return 0;
}
