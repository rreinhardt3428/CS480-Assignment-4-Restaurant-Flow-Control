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

// handling command line arguments given using getopt
void handleArgs(int argc, char* argv[], int& totalRequests, int& txSleepTime, int& rev9SleepTime, int& generalSleepTime, int& vipSleepTime) { 
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
                std::cerr << "Incorrect format. Use -s, -x, -r, -g, -v flags.\n";
                std::exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[]) { // main for creating and handling necessary threads
    RequestQueue requestQueue;     // Initialize shared request queue object
    requestQueue.barrierPosted = false; // preinitializing

    // Set all counters in request queue to 0 
    for (int i = 0; i < RequestTypeN; i++) {
        requestQueue.produced[i] = 0;
        requestQueue.inQueue[i] = 0;
        for (int j = 0; j < ConsumerTypeN; j++) {
            requestQueue.consumed[j][i] = 0;
        }
    }
    requestQueue.totalProduced = 0; // Overall number of requests produced
    requestQueue.totalConsumed = 0; // Overall number of requests consumed

    // Initialize mutex and condition variables used in queue monitor
    pthread_mutex_init(&requestQueue.mutex, nullptr);
    pthread_cond_init(&requestQueue.spaceAvailable, nullptr);
    pthread_cond_init(&requestQueue.requestAvailable, nullptr);

    // simulation parameters
    int totalRequests = DEFAULT_REQUESTS; // requests to produce
    int txSleepTime = 0; // sleep time for TX consumer
    int rev9SleepTime = 0; // sleep time for rev9 consumer
    int generalSleepTime = 0; // sleep time for general producer
    int vipSleepTime = 0; // sleep time for vip producer

    // intializing the parameters from command line inputs
    handleArgs(argc, argv, totalRequests, txSleepTime, rev9SleepTime, generalSleepTime, vipSleepTime);

    sem_t barrier;
    sem_init(&barrier, 0, 0); // initializing the barrier semaphore in order to block the thread when needed

    pthread_t producerThreads[RequestTypeN];
    pthread_t consumerThreads[ConsumerTypeN];

    // launch producer threads starting with the General table, then the VIP Room   
    ProducerArgs generalArg {&requestQueue, totalRequests, vipSleepTime, generalSleepTime, GeneralTable};
    pthread_create(&producerThreads[GeneralTable], nullptr, producer, &generalArg);
    ProducerArgs vipArg {&requestQueue, totalRequests, vipSleepTime, generalSleepTime, VIPRoom};
    pthread_create(&producerThreads[VIPRoom], nullptr, producer, &vipArg);

    // launch the TX and REV9 consumer threads after producer threads
    ConsumerArgs txArg {&requestQueue, totalRequests, &barrier, txSleepTime, rev9SleepTime, TX};
    pthread_create(&consumerThreads[TX], nullptr, consumer, &txArg);
    ConsumerArgs rev9Arg {&requestQueue, totalRequests, &barrier, txSleepTime, rev9SleepTime, Rev9};
    pthread_create(&consumerThreads[Rev9], nullptr, consumer, &rev9Arg);

    sem_wait(&barrier); // blocking the main thread until all requests have been consumed

    for (int i = 0; i < ConsumerTypeN; ++i) {
        pthread_join(consumerThreads[i], nullptr); // joining the consumer threads for termination
    }
    for (int i = 0; i < RequestTypeN; ++i) {
        pthread_join(producerThreads[i], nullptr); // joining the producer threads for termination
    }

    unsigned int* consumedPtrs[ConsumerTypeN] = { // outputting the summary statistics when all the threads have finished
        requestQueue.consumed[TX],
        requestQueue.consumed[Rev9]
    };
    output_production_history(requestQueue.produced, consumedPtrs);

    // Clean up mutex, condition variables, and semaphore barrier once all threads are complete
    pthread_mutex_destroy(&requestQueue.mutex);
    pthread_cond_destroy(&requestQueue.spaceAvailable);
    pthread_cond_destroy(&requestQueue.requestAvailable);
    sem_destroy(&barrier);

    return 0;
}