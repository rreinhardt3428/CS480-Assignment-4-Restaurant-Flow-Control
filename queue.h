#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <queue>
#include "seating.h"

// Thread-safe request queue
struct RequestQueue {
    std::queue<RequestType> queue;

    unsigned int produced[RequestTypeN] = {0};                   // Count by type
    unsigned int consumed[ConsumerTypeN][RequestTypeN] = {{0}};  // Count per consumer
    unsigned int inQueue[RequestTypeN] = {0};                    // Queue snapshot by type
    int totalProduced = 0;
    int totalConsumed = 0;

    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;

    bool is_full(RequestType type) const;
};

extern RequestQueue requestQueue;

#endif // QUEUE_H
