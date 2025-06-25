#include <pthread.h>
#include "semaphore.h"

void P(semaphore *s) {
    pthread_mutex_lock(&s->mtx);
    s->value--;
    if (s->value < 0) {
        pthread_cond_wait(&s->cv, &s->mtx);
    }
    pthread_mutex_unlock(&s->mtx);
}

void V(semaphore *s) {
    pthread_mutex_lock(&s->mtx);
    s->value++;
    if (s->value <= 0) {
        pthread_cond_signal(&s->cv);
    }
    pthread_mutex_unlock(&s->mtx);
}