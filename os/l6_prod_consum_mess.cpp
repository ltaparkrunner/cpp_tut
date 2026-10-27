void consumer() {
    int N = 100;
    message m;
    for (i=0; i<N; i++)
        send(producer, &m);
    while(true) {
        receive(producer, &m);
        int item = extract(&m);
        send(producer, &empty);
        consume(item);
    }
}

void producer(void){
    message m;
    while(true) {
        int item = produce_item();
        receive(consumer, &m);
        build_message(&m, item);
        send(consumer, &m);
    }

}

// monitor,  MPI  message passing interface
//

#include <barrier>

std::barrier br;
// Posix phread_barrier_t
// OpenMP  #pragma omp barrier
// can be made fron mutex + conditional_variable


// memory fences

#include <atomic>

std::atomic<int> turn{0};
std::atomic<int> x{0};

// writer
int proc(){
    x.store(100, std::memory_order_relaxed);
    turn.store(1, std::memory_order_release);

    while(turn.load(std::memory_order_acquire) != 1) { /* spin */}
    int v = x.load(std::memory_order_release);

    return 0;
}

