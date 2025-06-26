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
#include "consumer.h"
#include "producer.h"
#include "sharedData.h"

const int DEFAULT_REQUESTS = 120;

// Parses command line arguments, filling the parameters by reference
void handleArgs(int argc, char* argv[], int& totalRequests, int& txSleepTime, int& rev9SleepTime, int& generalSleepTime, int& vipSleepTime) {
    int opt;
    while ((opt = getopt(argc, argv, "s:x:r:g:v:")) != -1) {
        switch (opt) {
            case 's':
                totalRequests = std::atoi(optarg); // total requests to produce
                break;
            case 'x':
                txSleepTime = std::atoi(optarg); // sleep time for TX consumer
                break;
            case 'r':
                rev9SleepTime = std::atoi(optarg); // sleep time for Rev9 consumer
                break;
            case 'g':
                generalSleepTime = std::atoi(optarg); // sleep time for general producer
                break;
            case 'v':
                vipSleepTime = std::atoi(optarg); // sleep time for VIP producer
                break;
            default:
                std::cerr << "Incorrect format. Use -s, -x, -r, -g, -v flags.\n";
                std::exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize the shared request queue object
    RequestQueue requestQueue;
    requestQueue.barrierPosted = false; // Initialize barrier flag to false

    // Initialize counters in the queue to zero to avoid garbage values
    for (int i = 0; i < RequestTypeN; i++) {
        requestQueue.produced[i] = 0;  // Total produced for each request type
        requestQueue.inQueue[i] = 0;   // Current requests in queue for each type
        for (int j = 0; j < ConsumerTypeN; j++) {
            requestQueue.consumed[j][i] = 0; // Total consumed by each consumer for each request type
        }
    }
    requestQueue.totalProduced = 0; // Overall number of requests produced
    requestQueue.totalConsumed = 0; // Overall number of requests consumed

    // Initialize the mutex and condition variables used in the queue monitor
    pthread_mutex_init(&requestQueue.mutex, nullptr);
    pthread_cond_init(&requestQueue.spaceAvailable, nullptr);
    pthread_cond_init(&requestQueue.requestAvailable, nullptr);

    // Initialize simulation parameters with defaults
    int totalRequests = DEFAULT_REQUESTS;
    int txSleepTime = 0;
    int rev9SleepTime = 0;
    int generalSleepTime = 0;
    int vipSleepTime = 0;

    // Parse command line arguments to overwrite defaults if specified
    handleArgs(argc, argv, totalRequests, txSleepTime, rev9SleepTime, generalSleepTime, vipSleepTime);

    // Initialize a semaphore to act as a barrier to block main thread until consumption is complete
    sem_t barrier;
    sem_init(&barrier, 0, 0);

    // Prepare argument structs to pass all necessary shared data to producer and consumer threads
    ProducerArgs producerArgsGeneral {&requestQueue, totalRequests, vipSleepTime, generalSleepTime, GeneralTable};
    ProducerArgs producerArgsVIP {&requestQueue, totalRequests, vipSleepTime, generalSleepTime, VIPRoom};

    ConsumerArgs consumerArgsTX {&requestQueue, totalRequests, &barrier, txSleepTime, rev9SleepTime, TX};
    ConsumerArgs consumerArgsRev9 {&requestQueue, totalRequests, &barrier, txSleepTime, rev9SleepTime, Rev9};
    
    // Declare arrays to hold thread IDs for producers and consumers
    pthread_t producerThreads[RequestTypeN];
    pthread_t consumerThreads[ConsumerTypeN];

    // Create producer threads for General Table and VIP Room requests, passing their respective argument structs
    pthread_create(&producerThreads[GeneralTable], nullptr, producer, &producerArgsGeneral);
    pthread_create(&producerThreads[VIPRoom], nullptr, producer, &producerArgsVIP);

    // Create consumer threads for TX and Rev9, passing their respective argument structs
    pthread_create(&consumerThreads[TX], nullptr, consumer, &consumerArgsTX);
    pthread_create(&consumerThreads[Rev9], nullptr, consumer, &consumerArgsRev9);

    // Wait on the semaphore barrier until all requests have been consumed, blocking main thread here
    sem_wait(&barrier);

    // Join all consumer threads (wait for them to finish)
    for (int i = 0; i < ConsumerTypeN; ++i) {
        pthread_join(consumerThreads[i], nullptr);
    }
    // Join all producer threads (wait for them to finish)
    for (int i = 0; i < RequestTypeN; ++i) {
        pthread_join(producerThreads[i], nullptr);
    }

    // Prepare pointers to each consumer's consumed array for final output
    unsigned int* consumedPtrs[ConsumerTypeN] = {
        requestQueue.consumed[TX],
        requestQueue.consumed[Rev9]
    };
    // Output summary of production and consumption statistics
    output_production_history(requestQueue.produced, consumedPtrs);

    // Clean up mutex, condition variables, and semaphore resources
    pthread_mutex_destroy(&requestQueue.mutex);
    pthread_cond_destroy(&requestQueue.spaceAvailable);
    pthread_cond_destroy(&requestQueue.requestAvailable);
    sem_destroy(&barrier);

    return 0;
}