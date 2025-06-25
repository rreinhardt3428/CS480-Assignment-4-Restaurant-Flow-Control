#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <queue>
#include "seating.h"

struct RequestQueue {
    std::queue<RequestType> queue; // first in first out queue to hold the requests waiting

    unsigned int produced[RequestTypeN] = {0}; // tracking the requests for each type that has been produced
    unsigned int consumed[ConsumerTypeN][RequestTypeN] = {{0}}; // tracks how many requests each consumer type has handled
    unsigned int inQueue[RequestTypeN] = {0}; // the number of each request within the queue
    int totalProduced = 0; // number of requests produced
    int totalConsumed = 0; // number of requests consumed

    pthread_mutex_t mutex; // controls the access to the queue and counters
    pthread_cond_t spaceAvailable; // conditional signaling when there is available queue space
    pthread_cond_t requestAvailable; // conditional signaling when there is new data in the queue space

    bool isFull(RequestType type) const; // function to check if the queue is full based upon and limits or constraints
};

extern RequestQueue requestQueue;

#endif // QUEUE_H
