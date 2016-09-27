/*
 *  mutex.c
 *
 *  Created on: Aug 18, 2016
 *  Author: k100
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_THREAD_NUM 10

pthread_mutex_t mtx_proc = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_proc = PTHREAD_COND_INITIALIZER;

int g_recv_count = 0;
int g_done_count = 0;

struct pthread_param {
    int thread_id;
};

void recv_thread(void *param)
{
    int thread_id;

    struct pthread_param *p = (struct pthread_param *) param;
    thread_id = p->thread_id;

    printf("recv thread_id = %d\n", thread_id);

    while(1) {

        g_recv_count++;
        printf("recv channel report %d\n", g_recv_count);

        // check bitmap
        if (g_recv_count == 4) {
            printf("recv thread: g_recv_count = %d\n", g_recv_count);
            g_recv_count = 0;

            pthread_mutex_lock(&mtx_proc);
            /* signal all waiting thread */
            pthread_cond_broadcast(&cond_proc);
            pthread_mutex_unlock(&mtx_proc);
        }

        sleep(1);
    }
}

void proc_thread(void *param)
{
    int thread_id;

    struct pthread_param *p = (struct pthread_param *) param;
    thread_id = p->thread_id;

    printf("proc thread_id = %d\n", thread_id);

    while (1) {

        pthread_mutex_lock(&mtx_proc);
        pthread_cond_wait(&cond_proc, &mtx_proc);
        pthread_mutex_unlock(&mtx_proc);

        printf("thread #%d: start process\n", thread_id);
        sleep(1);

        pthread_mutex_lock(&mtx_proc);
        g_done_count++;
        if (g_done_count == MAX_THREAD_NUM) {
            printf("all proc thread done, send precode\n");
            g_done_count = 0;
        }
        pthread_mutex_unlock(&mtx_proc);
    }
}

int main(int argc, char **argv)
{
    printf("main thread\n");

    pthread_t udn_recv_thread;
    struct pthread_param udn_recv_param;
    udn_recv_param.thread_id = 1001;

    if (pthread_create(&udn_recv_thread, NULL, (void *) &recv_thread, (void*) &udn_recv_param)) {
        fprintf(stderr, "Error creating recv thread \n");
        return 1;
    }

    pthread_t udn_proc_thread[MAX_THREAD_NUM];
    struct pthread_param udn_proc_param[MAX_THREAD_NUM];
    int i;
    for (i = 0; i < MAX_THREAD_NUM; i++) {
        udn_proc_param[i].thread_id = i;
        if (pthread_create(&udn_proc_thread[i], NULL, (void *) &proc_thread, (void*) &udn_proc_param[i])) {
            fprintf(stderr, "Error creating proc thread %d\n", i);
            return 1;
        }
    }


    pthread_join(udn_recv_thread, NULL);
    for (i = 0; i < MAX_THREAD_NUM; i++) {
        pthread_join(udn_proc_thread[i], NULL);
    }

    return 0;
}
