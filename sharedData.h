// shareddata.h
#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include "queue.h"
#include <semaphore.h>

// Holds all shared data the producers need
struct ProducerArgs {
    RequestQueue* requestQueue; // pointer to shared request queue
    int totalRequests;          // total number of requests to produce
    int vipSleepTime;           // VIP producer sleep time (ms)
    int generalSleepTime;       // General producer sleep time (ms)
    RequestType role;
};

// Holds all shared data the consumers need
struct ConsumerArgs {
    RequestQueue* requestQueue; // pointer to shared request queue
    int totalRequests;          // total number of requests to consume
    sem_t* barrier;             // pointer to barrier semaphore
    int txSleepTime;            // T-X consumer sleep time (ms)
    int rev9SleepTime;          // Rev-9 consumer sleep time (ms)
    ConsumerType role;
};

#endif