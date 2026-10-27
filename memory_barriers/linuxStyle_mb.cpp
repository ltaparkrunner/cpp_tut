#include <linux/kernel.h>
#include <asm/barrier.h>

/* Общие переменные */
int shared_data = 0;
int ready_flag = 0;

/* --- ПОТОК 1: Producer (Производитель) --- */
void producer_thread(void)
{
    shared_data = 42;          // 1. Записываем данные

    /* smp_store_release объединяет в себе:
       - Барьер записи (все предыдущие записи завершатся ДО этой)
       - Саму запись в ready_flag = 1
    */
    smp_store_release(&ready_flag, 1);
}

/* --- ПОТОК 2: Consumer (Потребитель) --- */
void consumer_thread(void)
{
    int flag;

    /* smp_load_acquire объединяет в себе:
       - Чтение значения ready_flag
       - Барьер чтения (все последующие чтения произойдут ПОСЛЕ этого)
    */
    do {
        flag = smp_load_acquire(&ready_flag);
        cpu_relax();
    } while (flag == 0);

    /* Теперь мы гарантированно видим shared_data = 42 */
    pr_info("Data received: %d\n", shared_data);
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
    return 0;
}