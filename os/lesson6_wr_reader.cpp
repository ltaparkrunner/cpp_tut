#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>
#include <semaphore>

#define N 100
typedef int semathore;
void down(semaphore *s); // atomic: if s==0 -> block else s--;
void up(semathore *s);   // atomic: s++, if waiting_process wake_up

semaphore mutex = 1;
semaphore db = 1;
int rc = 0;

void reader() {
    while(1) {
        down(&mutex);
        rc++;
        // first reader blocks DB
        if(rc == 1) down(&db);
        up(&mutex);
        read_data_base();
        down(&mutex);
        rc--;
        // the last reader release DB
        if (rc == 0) up(&db);
        up(&mutex);
        use_data_read();
    }
}

void writer(void) {
    while(1) {
        think_up_data();
        down(&db);
        write_data_base();
        up(&db);
    }
}