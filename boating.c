#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int value;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} semaphore;

void sem_init(semaphore *s, int init_value) {
    s->value = init_value;
    pthread_mutex_init(&s->mtx, NULL);
    pthread_cond_init(&s->cv, NULL);
}

void sem_destroy(semaphore *s) {
    pthread_mutex_destroy(&s->mtx);
    pthread_cond_destroy(&s->cv);
}

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

int m; // Number of boats
int n; // Number of visitors
semaphore boat; // Semaphore for boats
semaphore rider; // Semaphore for riders
pthread_mutex_t bmtx = PTHREAD_MUTEX_INITIALIZER; // Mutex for boat availability arrays
pthread_barrier_t EOS; // End of session barrier


int *BA; // Boat Available array (1 if available, 0 if not)
int *BC; // Boat-Visitor connection array (-1 if no visitor, visitor ID otherwise)
int *BT; // Boat ride Time array
pthread_barrier_t *BB; // Boat Barriers array

int visitors_completed = 0;
pthread_mutex_t completion_mutex = PTHREAD_MUTEX_INITIALIZER;
int simulation_done = 0;
int active_boats = 0;

int get_random(int min, int max) {
    return min + rand() % (max - min + 1);
}

void *boat_thread(void *arg) {
    int id = *((int *)arg);
    free(arg);
    pthread_barrier_init(&BB[id-1], NULL, 2);
    pthread_mutex_lock(&completion_mutex);
    active_boats++;
    pthread_mutex_unlock(&completion_mutex);
    printf("Boat %6d    Ready\n", id);
    while (1) {
        pthread_mutex_lock(&bmtx);
        BA[id-1] = 1;
        BC[id-1] = -1;
        pthread_mutex_unlock(&bmtx);
        V(&rider);
        P(&boat);
        pthread_mutex_lock(&completion_mutex);
        if (simulation_done) {
            active_boats--;
            if (active_boats == 0) {
                pthread_barrier_wait(&EOS);
            }
            pthread_mutex_unlock(&completion_mutex);
            break;
        }
        pthread_mutex_unlock(&completion_mutex);        
        pthread_barrier_wait(&BB[id-1]);
        pthread_mutex_lock(&bmtx);
        BA[id-1] = 0; 
        int visitor_id = BC[id-1];
        int ride_time = BT[id-1];
        pthread_mutex_unlock(&bmtx);
        printf("Boat %6d    Start of ride for visitor %3d\n", id, visitor_id);
        usleep(ride_time * 100000);
        printf("Boat %6d    End of ride for visitor %3d (ride time = %2d)\n", id, visitor_id, ride_time);
    }
    pthread_barrier_destroy(&BB[id-1]);
    return NULL;
}


void *visitor_thread(void *arg) {
    int id = *((int *)arg);
    free(arg);
    int vtime = get_random(30, 120); 
    int rtime = get_random(15, 60);  
    printf("Visitor %3d    Starts sightseeing for %3d minutes\n", id, vtime);
    usleep(vtime * 100000);
    printf("Visitor %3d    Ready to ride a boat (ride time = %2d)\n", id, rtime);
    P(&rider);
    int found_boat = -1;
    while (found_boat == -1) {
        pthread_mutex_lock(&bmtx);
        for (int i = 0; i < m; i++) {
            if (BA[i] && BC[i] == -1) {
                BC[i] = id;
                BT[i] = rtime;
                found_boat = i + 1;
                break;
            }
        }
        pthread_mutex_unlock(&bmtx);
        if (found_boat == -1) {
            usleep(1000); 
        }
    }
    printf("Visitor %3d    Finds boat %2d\n", id, found_boat);
    V(&boat);
    pthread_barrier_wait(&BB[found_boat-1]);
    printf("Visitor %3d    Leaving\n", id);
    pthread_mutex_lock(&completion_mutex);
    visitors_completed++;
    if (visitors_completed >= n) {
        simulation_done = 1;
        for (int i = 0; i < m; i++) {
            V(&boat);
        }
    }
    pthread_mutex_unlock(&completion_mutex);
    return NULL;
}

int main(int argc, char *argv[]) {

    srand(time(NULL));
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_boats> <num_visitors>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    if (m < 5 || m > 10 || n < 20 || n > 100) {
        fprintf(stderr, "Error: 5 ≤ m ≤ 10, 20 ≤ n ≤ 100\n");
        exit(EXIT_FAILURE);
    }
    sem_init(&boat, 0);
    sem_init(&rider, 0);
    pthread_barrier_init(&EOS, NULL, 2);
    BA = (int *)calloc(m, sizeof(int));
    BC = (int *)malloc(m * sizeof(int));
    BT = (int *)malloc(m * sizeof(int));
    BB = (pthread_barrier_t *)malloc(m * sizeof(pthread_barrier_t));
    for (int i = 0; i < m; i++) {
        BA[i] = 0;
        BC[i] = -1;
    }
    pthread_t *boat_threads = (pthread_t *)malloc(m * sizeof(pthread_t));
    for (int i = 0; i < m; i++) {
        int *id = (int *)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&boat_threads[i], NULL, boat_thread, id);
    }
    pthread_t *visitor_threads = (pthread_t *)malloc(n * sizeof(pthread_t));
    for (int i = 0; i < n; i++) {
        int *id = (int *)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&visitor_threads[i], NULL, visitor_thread, id);
    }
    pthread_barrier_wait(&EOS);
    for (int i = 0; i < n; i++) {
        pthread_join(visitor_threads[i], NULL);
    }
    
    for (int i = 0; i < m; i++) {
        pthread_join(boat_threads[i], NULL);
    }
    
    pthread_barrier_destroy(&EOS);
    pthread_mutex_destroy(&bmtx);
    sem_destroy(&boat);
    sem_destroy(&rider);
    pthread_mutex_destroy(&completion_mutex);
    
    free(BA);
    free(BC);
    free(BT);
    free(BB);
    free(boat_threads);
    free(visitor_threads);
    
    return 0;
}