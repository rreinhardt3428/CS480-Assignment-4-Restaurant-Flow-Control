// Ethan Kent CS480
// REDID: 826843661
// Roger Reinhardt
// REDID: 826470808

#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include "queue.h"
#include <semaphore.h>

// Holds all shared data the producers need
struct ProducerArgs {
    RequestQueue* requestQueue; // pointer to shared request queue
    int totalRequests; // total number of requests to produce
    int vipSleepTime; // Sleep times for each producer type in ms
    int generalSleepTime; 
    RequestType role; // Determine whether producer handles VIP Room or General Table 
};

// Holds all shared data the consumers need
struct ConsumerArgs {
    RequestQueue* requestQueue; // pointer to shared request queue
    int totalRequests; // total number of requests to consume
    sem_t* barrier; // pointer to barrier semaphore
    int txSleepTime; // Sleep times for each consumer type in ms
    int rev9SleepTime; 
    ConsumerType role; // Determines whether Consumer is T-X or Rev-9
};

#endif