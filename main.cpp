#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <semaphore.h>

#include "seating.h"
#include "queue.h"
#include "log.h"

// External thread functions
extern void* producer(void*);
extern void* consumer(void*);

// Shared simulation parameters
int totalRequests = 120;
int tx_sleep_ms = 0;
int rev9_sleep_ms = 0;
int general_sleep_ms = 0;
int vip_sleep_ms = 0;

sem_t barrier;

// Parse optional CLI arguments: -s -x -r -g -v
void parse_args(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "s:x:r:g:v:")) != -1) {
        if (opt == 's') {
            totalRequests = std::atoi(optarg);
        } else if (opt == 'x') {
            tx_sleep_ms = std::atoi(optarg);
        } else if (opt == 'r') {
            rev9_sleep_ms = std::atoi(optarg);
        } else if (opt == 'g') {
            general_sleep_ms = std::atoi(optarg);
        } else if (opt == 'v') {
            vip_sleep_ms = std::atoi(optarg);
        } else {
            std::cerr << "Usage: " << argv[0]
                      << " [-s total_requests] [-x tx_ms] [-r rev9_ms] "
                         "[-g general_ms] [-v vip_ms]\n";
            exit(EXIT_FAILURE);
        }
    }
}


int main(int argc, char* argv[]) {
    parse_args(argc, argv);

    sem_init(&barrier, 0, 0);

    pthread_t producers[RequestTypeN];
    pthread_t consumers[ConsumerTypeN];

    // Required launch order: General producer, VIP producer
    pthread_create(&producers[GeneralTable], nullptr, producer,
                   reinterpret_cast<void*>(GeneralTable));
    pthread_create(&producers[VIPRoom], nullptr, producer,
                   reinterpret_cast<void*>(VIPRoom));

    // Then: TX consumer, Rev-9 consumer
    pthread_create(&consumers[TX], nullptr, consumer,
                   reinterpret_cast<void*>(TX));
    pthread_create(&consumers[Rev9], nullptr, consumer,
                   reinterpret_cast<void*>(Rev9));

    // Main thread waits for the last consumer to signal barrier
    sem_wait(&barrier);

    // Join threads (spec-compliant clean-up)
    for (int i = 0; i < ConsumerTypeN; ++i)
        pthread_join(consumers[i], nullptr);
    for (int i = 0; i < RequestTypeN; ++i)
        pthread_join(producers[i], nullptr);

    // Output summary statistics
    unsigned int* consumedPtrs[ConsumerTypeN] = {
        requestQueue.consumed[TX],
        requestQueue.consumed[Rev9]
    };
    output_production_history(requestQueue.produced, consumedPtrs);

    sem_destroy(&barrier);
    return 0;
}
