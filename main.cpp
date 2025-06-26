// Ethan Kent CS480
// REDID: 826843661
// Roger Reinhardt
// REDID: 826470808

#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <semaphore.h>

#include "seating.h"
#include "queue.h"
#include "log.h"

const int DEFAULT_REQUESTS = 120;

// outside thread functions for use
extern void* producer(void*);
extern void* consumer(void*);

// simulation parameters
int totalRequests = DEFAULT_REQUESTS; // requests to produce
int txSleepTime = 0; // sleep time for TX consumer
int rev9SleepTime = 0; // sleep time for rev9 consumer
int generalSleepTime = 0; // sleep time for general producer
int vipSleepTime = 0; // sleep time for vip producer

sem_t barrier; // Barrier semaphore for blocking the main thread until a simulation is done

void handleArgs(int argc, char* argv[]) { // Handle command line arguments given using getopt
    int opt;
    while ((opt = getopt(argc, argv, "s:x:r:g:v:")) != -1) {
        switch (opt) {
            case 's':
                totalRequests = std::atoi(optarg); // number of requests
                break;
            case 'x':
                txSleepTime = std::atoi(optarg); // TX consumer
                break;
            case 'r':
                rev9SleepTime = std::atoi(optarg); // REV9 consumer
                break;
            case 'g':
                generalSleepTime = std::atoi(optarg); // general producer
                break;
            case 'v':
                vipSleepTime = std::atoi(optarg); // vip producer
                break;
            default:
                std::cerr << "Incorrect format. Use -s, -x, -r, -g, -v flags.";
                std::exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[]) { // Main for creating and handling necessary threads
    handleArgs(argc, argv); // Initialize parameters from command line inputs

    sem_init(&barrier, 0, 0); // Initialize barrier semaphore in order to block the thread when needed

    pthread_t producerThreads[RequestTypeN];
    pthread_t consumerThreads[ConsumerTypeN];

    // launch producer threads starting with the General table, then the VIP Room
    pthread_create(&producerThreads[GeneralTable], nullptr, producer, reinterpret_cast<void*>(GeneralTable)); 
    pthread_create(&producerThreads[VIPRoom], nullptr, producer, reinterpret_cast<void*>(VIPRoom));

    // launch the TX and REV9 consumer threads after producer threads
    pthread_create(&consumerThreads[TX], nullptr, consumer, reinterpret_cast<void*>(TX)); 
    pthread_create(&consumerThreads[Rev9], nullptr, consumer, reinterpret_cast<void*>(Rev9));

    sem_wait(&barrier); // Block main thread until all requests have been consumed


    for (int i = 0; i < ConsumerTypeN; ++i){
        pthread_join(consumerThreads[i], nullptr); // joining the consumer threads for termination
    }
    for (int i = 0; i < RequestTypeN; ++i){
        pthread_join(producerThreads[i], nullptr); // joining the producer threads for termination
    }

    unsigned int* consumedPtrs[ConsumerTypeN] = { // Output summary statistics once all the threads have completed
        requestQueue.consumed[TX],
        requestQueue.consumed[Rev9]
    };
    output_production_history(requestQueue.produced, consumedPtrs);

    sem_destroy(&barrier); // Clean up barrier semaphore once all threads have completed
    return 0;
}
