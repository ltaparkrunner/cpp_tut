#include <iostream>
#include <thread>
#include <chrono>

void task_function(std::stop_token stoken) {
    for (int i = 0; i < 10; ++i) {
        if (stoken.stop_requested()) { // Periodically check if a stop was requested
            std::cout << "Stop requested, exiting task." << std::endl;
            return; // Cooperatively stop the thread
        }
        std::cout << "Task running... " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    std::jthread thr(task_function); // std::jthread automatically passes a stop_token

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Main thread requesting stop." << std::endl;
    thr.request_stop(); // Request the thread to stop

    // std::jthread's destructor will automatically join (and request stop again if needed)
    // The task function will detect the stop request and return
}