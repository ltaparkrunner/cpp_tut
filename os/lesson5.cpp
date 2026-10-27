#include <thread>
#include <vector>
#include <iostream>

// TSL (test and set lock) and XCHG(exchange)

class TSLLock {
public:
    TSLLock() = default;

    TSLLock(const TSLLock&) = delete;
    TSLLock& operator=(const TSLLock&) = delete;

    void lock() {
        while(m_locked.test_and_set(std::memory_order_acquire))
        ;
    }

    void unlock() {
        m_locked.clear(std::memory_order_release);
    }
private:
    std::atomic_flag m_locked = ATOMIC_FLAG_INIT;
};


// spin_lock()      spin_lock_irq()     spin_lock_irqsave()

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

class Spinlock {
private:
    // std::atomic_flag is guaranteed to be lock-free and ideal for spinlocks
    std::atomic_flag locked = ATOMIC_FLAG_INIT; 

public:
    void lock() {
        // test_and_set atomically sets the flag to true and returns its previous value.
        // The while loop continues as long as the flag was already true (locked).
        while (locked.test_and_set(std::memory_order_acquire)) {
            // Optional: add a 'pause' instruction for better performance on hyper-threaded CPUs
            // This is architecture specific (e.g., __builtin_ia32_pause() in GCC for x86)
        }
    }

    void unlock() {
        // clear atomically sets the flag to false (unlocked).
        locked.clear(std::memory_order_release);
    }
};

// Example usage
volatile int shared_value = 0; // volatile to ensure visibility, but atomic operations handle this better
Spinlock my_lock;

void task() {
    for (int i = 0; i < 1000; ++i) {
        my_lock.lock();
        shared_value++; // Critical section
        my_lock.unlock();
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(task);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Final shared value: " << shared_value << std::endl;
    return 0;
}

//  macOS    sleep()     tsleep()        msleep()    wakeup()
// Linux    wait_event()    wake_up()   schedule()
// futex()      sleep()     nanosleep()
//  Windows     Sleep(ms)
// SetEvent()       WaitForSingleObject()
// RTOS(FreeRTOS, Zephyr)   vTaskSuspend/xTaskResume()

/*
enter_region:         ; Function entry point
    TSL   REG, LOCK   ; Test and Set Lock: 
                      ; Copies the current value of LOCK into a register (REG)
                      ; and then atomically sets LOCK to 1.
    CMP   REG, #0     ; Compare the value in REG with 0
    JNZ   enter_region; Jump back to enter_region if REG was not zero (lock was held)
    RET               ; Exit the routine (lock acquired, enter critical section)

leave_region:         ; Function to exit critical section
    MOVE  LOCK, #0    ; Store 0 in LOCK to free the lock
    RET               ; Return to caller

*/