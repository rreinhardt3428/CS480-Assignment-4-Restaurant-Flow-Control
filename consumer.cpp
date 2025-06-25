#include <unistd.h>
#include <iostream>
#include <semaphore.h>

#include "consumer.h"
#include "queue.h"
#include "log.h"
#include "seating.h"

// variables to be used and shared across the threads
extern RequestQueue requestQueue;
extern int totalRequests;
extern sem_t barrier;
extern int txSleepTime;
extern int rev9SleepTime;

void* consumer(void* arg) { // function for consumer thread
    intptr_t rawValue = reinterpret_cast<intptr_t>(arg); // converting the arg into a consumer role for later use
    ConsumerType consumerType = static_cast<ConsumerType>(rawValue);
    // calculating how long the consumer should wait between processing the requests
    int sleepTime = 0;
    if (consumerType == TX) {
        sleepTime = txSleepTime;
    } else if (consumerType == Rev9) {
        sleepTime = rev9SleepTime;
    }

    while (true) {
        pthread_mutex_lock(&requestQueue.mutex); // locking the shared request queue before checking/modifying the state

        bool queueIsEmpty = requestQueue.queue.empty(); // checking if the queue is empty and if more requests are coming
        bool moreRequestsExpected = (requestQueue.totalConsumed < totalRequests);


        while (queueIsEmpty && moreRequestsExpected) {
            pthread_cond_wait(&requestQueue.requestAvailable, &requestQueue.mutex); // waiting for the new requests to arrive if the queue is empty and there is still come requests to be consumed

            // queue conditions after the wait
            queueIsEmpty = requestQueue.queue.empty();
            moreRequestsExpected = (requestQueue.totalConsumed < totalRequests);
        }

        if (requestQueue.queue.empty() && requestQueue.totalConsumed >= totalRequests) { // exiting if there are no remiaining requests and everything is done being handled
            pthread_mutex_unlock(&requestQueue.mutex);
            return nullptr;
        }

        RequestType req = requestQueue.queue.front(); // getting the next requests from the front of the queue
        requestQueue.queue.pop();
        requestQueue.inQueue[req]--; // updating the queue state and consumer statistics
        requestQueue.consumed[consumerType][req]++;
        requestQueue.totalConsumed++;

        output_request_removed(consumerType, req, requestQueue.consumed[consumerType], requestQueue.inQueue); // necessary logging

        if (requestQueue.totalConsumed >= totalRequests && requestQueue.queue.empty()) { // notifier that it was the last request
            sem_post(&barrier);
        }

        pthread_cond_signal(&requestQueue.spaceAvailable); // notifier for waiting producers that there is now space in the queue
        pthread_mutex_unlock(&requestQueue.mutex); // releasing the mutex in order for other threads to safely access the queue

        if (sleepTime > 0) { // processing time in MS
            usleep(sleepTime * 1000);
        }
    }
}
