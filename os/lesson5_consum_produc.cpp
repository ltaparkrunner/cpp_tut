#define N 100
typedef int semathore;
void down(semaphore *s); // atomic: if s==0 -> block else s--;
void up(semathore *s);   // atomic: s++, if waiting_process wake_up

semathore mutex = 1;
semathore empty = N;
semathore full  = 0;

void producer(void) {
    while(1) {
        int item = produce_item();
        down(&empty);
        down(&mutex);
        insert_item(item);
        up(&mutex);
        up(&full);
    }
}

void consumer(void) {
    while(1) {
        down(&full);
        down(&mutex);
        int item = remove_item();
        up(&mutex);
        up(&empty);
        consume_item(item);
    }
}