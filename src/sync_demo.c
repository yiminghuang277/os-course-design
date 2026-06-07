#include "sync_demo.h"

#include "common.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 5
#define MAX_EVENTS 12
#define MAX_READERS 5

typedef struct {
    int data[BUFFER_SIZE];
    int in_index;
    int out_index;
    int produced_total;
    int consumed_total;
    int goal;
    pthread_mutex_t mutex;
    sem_t empty_slots;
    sem_t filled_slots;
} ProducerConsumerState;

typedef struct {
    int shared_value;
    int read_count;
    pthread_mutex_t read_mutex;
    pthread_mutex_t write_mutex;
} ReaderWriterState;

static void sleep_millis(long millis) {
    struct timespec ts;
    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void *producer_thread(void *arg) {
    ProducerConsumerState *state = (ProducerConsumerState *)arg;

    while (1) {
        int item;
        pthread_mutex_lock(&state->mutex);
        if (state->produced_total >= state->goal) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }
        item = state->produced_total + 1;
        state->produced_total++;
        pthread_mutex_unlock(&state->mutex);

        sem_wait(&state->empty_slots);
        pthread_mutex_lock(&state->mutex);
        state->data[state->in_index] = item;
        printf("[生产者] 生产数据 %d -> buffer[%d]\n", item, state->in_index);
        state->in_index = (state->in_index + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&state->mutex);
        sem_post(&state->filled_slots);
        sleep_millis(50);
    }
    return NULL;
}

static void *consumer_thread(void *arg) {
    ProducerConsumerState *state = (ProducerConsumerState *)arg;

    while (1) {
        int item;
        pthread_mutex_lock(&state->mutex);
        if (state->consumed_total >= state->goal) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }
        pthread_mutex_unlock(&state->mutex);

        sem_wait(&state->filled_slots);
        pthread_mutex_lock(&state->mutex);
        if (state->consumed_total >= state->goal) {
            pthread_mutex_unlock(&state->mutex);
            sem_post(&state->filled_slots);
            break;
        }
        item = state->data[state->out_index];
        printf("[消费者] 消费数据 %d <- buffer[%d]\n", item, state->out_index);
        state->out_index = (state->out_index + 1) % BUFFER_SIZE;
        state->consumed_total++;
        pthread_mutex_unlock(&state->mutex);
        sem_post(&state->empty_slots);
        sleep_millis(70);
    }
    return NULL;
}

static void run_producer_consumer(void) {
    ProducerConsumerState state;
    pthread_t producer_a;
    pthread_t producer_b;
    pthread_t consumer_a;
    pthread_t consumer_b;

    print_header("生产者-消费者问题");
    state.in_index = 0;
    state.out_index = 0;
    state.produced_total = 0;
    state.consumed_total = 0;
    state.goal = prompt_int("请输入总生产数量(1-12): ", 1, MAX_EVENTS);
    pthread_mutex_init(&state.mutex, NULL);
    sem_init(&state.empty_slots, 0, BUFFER_SIZE);
    sem_init(&state.filled_slots, 0, 0);

    pthread_create(&producer_a, NULL, producer_thread, &state);
    pthread_create(&producer_b, NULL, producer_thread, &state);
    pthread_create(&consumer_a, NULL, consumer_thread, &state);
    pthread_create(&consumer_b, NULL, consumer_thread, &state);

    pthread_join(producer_a, NULL);
    pthread_join(producer_b, NULL);

    while (1) {
        pthread_mutex_lock(&state.mutex);
        if (state.consumed_total >= state.goal) {
            pthread_mutex_unlock(&state.mutex);
            break;
        }
        pthread_mutex_unlock(&state.mutex);
        sleep_millis(30);
    }
    sem_post(&state.filled_slots);
    sem_post(&state.filled_slots);
    pthread_join(consumer_a, NULL);
    pthread_join(consumer_b, NULL);

    printf("总生产: %d, 总消费: %d\n", state.produced_total, state.consumed_total);
    sem_destroy(&state.empty_slots);
    sem_destroy(&state.filled_slots);
    pthread_mutex_destroy(&state.mutex);
}

static void *reader_thread(void *arg) {
    ReaderWriterState *state = (ReaderWriterState *)arg;
    int i;
    for (i = 0; i < 3; ++i) {
        pthread_mutex_lock(&state->read_mutex);
        state->read_count++;
        if (state->read_count == 1) {
            pthread_mutex_lock(&state->write_mutex);
        }
        pthread_mutex_unlock(&state->read_mutex);

        printf("[读者] 读取共享数据 = %d\n", state->shared_value);
        sleep_millis(40);

        pthread_mutex_lock(&state->read_mutex);
        state->read_count--;
        if (state->read_count == 0) {
            pthread_mutex_unlock(&state->write_mutex);
        }
        pthread_mutex_unlock(&state->read_mutex);
        sleep_millis(40);
    }
    return NULL;
}

static void *writer_thread(void *arg) {
    ReaderWriterState *state = (ReaderWriterState *)arg;
    int i;
    for (i = 0; i < 4; ++i) {
        pthread_mutex_lock(&state->write_mutex);
        state->shared_value += 10;
        printf("[写者] 更新共享数据 = %d\n", state->shared_value);
        pthread_mutex_unlock(&state->write_mutex);
        sleep_millis(60);
    }
    return NULL;
}

static void run_reader_writer(void) {
    ReaderWriterState state;
    pthread_t readers[MAX_READERS];
    pthread_t writer;
    int reader_count = prompt_int("请输入读者数量(1-5): ", 1, MAX_READERS);
    int i;

    print_header("读者-写者问题");
    state.shared_value = 100;
    state.read_count = 0;
    pthread_mutex_init(&state.read_mutex, NULL);
    pthread_mutex_init(&state.write_mutex, NULL);

    pthread_create(&writer, NULL, writer_thread, &state);
    for (i = 0; i < reader_count; ++i) {
        pthread_create(&readers[i], NULL, reader_thread, &state);
    }
    for (i = 0; i < reader_count; ++i) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(writer, NULL);

    printf("最终共享数据 = %d\n", state.shared_value);
    pthread_mutex_destroy(&state.read_mutex);
    pthread_mutex_destroy(&state.write_mutex);
}

void sync_menu(void) {
    int choice;

    while (1) {
        print_header("进程同步与并发控制");
        printf("1. 生产者-消费者\n");
        printf("2. 读者-写者\n");
        printf("0. 返回上级菜单\n");
        choice = prompt_int("请选择功能: ", 0, 2);
        switch (choice) {
            case 1:
                run_producer_consumer();
                break;
            case 2:
                run_reader_writer();
                break;
            case 0:
                return;
            default:
                printf("无效选择。\n");
                break;
        }
    }
}
