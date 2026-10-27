pthread_mutex_t mutex
pthread_cond_t cond;
int buffer_full = 0;
int item;

void produser(void* arg) {
    while(1) {
        pthread_mutex_lock(&mutex);
        while(buffer_full)
            pthread_cond_wait(&cond, &mutex);
        item = produce_item();
        buffer_full = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

void consumer(void* arg) {
    while(1) {
        pthread_mutex_lock(&mutex);
        while(!buffer_full)
            pthread_cond_wait(&cond, &mutex);
        consume_tem(item);
        buffer_full = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}