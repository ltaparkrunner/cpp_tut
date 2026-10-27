/*

#include <thread>

thread_create(fn, args);
tread_exit();
thread_join();
thread_yield();

fork();
pthread_atfork();


#include <pthread.h>

int pthread_create(pthread_t *restrict thread,
                    const pthread_attr_t *restrict attr,
                    typeof(void *(void *)) *start_routine,
                    void *restrict arg);

[[noreturn]] void pthread_exit(void *retval);
int pthread_join(pthread_t thread, void **retval);
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);

#include <sched.h>

int sched_yield(void);
[[deprecated]] int pthread_yield(void);
*/

#include <pthread.h>

void* worker(void* arg) {
    int id = *static_cast<int*>(arg);
    for(int k=0; k<3; ++k) {
        std::cout << "worker " << id << " iteration " << std::endl;
        sched_yield();
    }
    return nullptr;
}

int main() {
    constexpr int N=4;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1<<20);

    std::vector<pthread_t> th(N);
    std::vector<int> ids(N);

    for(int i=0; i <N; ++i) {
        ids[i] = i;
        if(int rc = pthread_create(&th[i], &attr, &worker,
                    &ids[i]); rc!=0) {
                        std::cerr << "pthread_create: " << std::strerror(rc) << std::endl;
                        return 1;
        }
        std::cout << "main: started thread # " << ids[i] << std::endl;
    }
    pthread_attr_destroy(&attr);
    for(int i=0; i<N; ++i) {
        if(int rc = pthread_join(th[i], nullptr); rc!=0) {
            std::cerr << "pthread_join: " << std::stderror(rc) << std::endl;
        }
    }
    std::cout << "main: all threads joined" << std::endl;
}