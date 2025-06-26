// Ethan Kent CS480
// REDID: 826843661
// Roger Reinhardt
// REDID: 826470808

#include <unistd.h>
#include <iostream>
#include <semaphore.h>

#include "consumer.h"
#include "queue.h"
#include "log.h"
#include "seating.h"

const int MICROSECONDS = 1000; // Converts milliseconds to microseconds for usleep

// variables to be used and shared across the threads
extern RequestQueue requestQueue;
extern int totalRequests;
extern sem_t barrier;
extern int txSleepTime;
extern int rev9SleepTime;

void* consumer(void* arg) { // function for consumer thread; input is either TX or REV9 and returns a nullptr on completion
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
        pthread_mutex_lock(&requestQueue.mutex); // Begin monitor using mutex lock for access shared buffer

        bool queueIsEmpty = requestQueue.queue.empty(); // Check if bounded buffer is empty and if more requests are coming
        bool moreRequestsExpected = (requestQueue.totalConsumed < totalRequests);


        while (queueIsEmpty && moreRequestsExpected) { // Waiting for new requests if the queue is empty and requests are available for consumption
            pthread_cond_wait(&requestQueue.requestAvailable, &requestQueue.mutex); 

            queueIsEmpty = requestQueue.queue.empty(); // Check conditions after waking up
            moreRequestsExpected = (requestQueue.totalConsumed < totalRequests);
        }

        if (requestQueue.queue.empty() && requestQueue.totalConsumed >= totalRequests) { // Check if all requests have been consumed
            pthread_mutex_unlock(&requestQueue.mutex); // Release monitor lock 
            return nullptr;
        }

        RequestType request = requestQueue.queue.front(); // getting the next requests from the front of the queue
        requestQueue.queue.pop();
        requestQueue.inQueue[request]--; // updating buffer state and consumer statistics
        requestQueue.consumed[consumerType][request]++;
        requestQueue.totalConsumed++;

        output_request_removed(consumerType, request, requestQueue.consumed[consumerType], requestQueue.inQueue); // necessary logging

        if (requestQueue.totalConsumed >= totalRequests && requestQueue.queue.empty()) { // Notifier that it was the last request
            sem_post(&barrier); // Signals barrier to unlock the main thread
        }

        pthread_cond_signal(&requestQueue.spaceAvailable); // Signals that space is in the queue for producers
        pthread_mutex_unlock(&requestQueue.mutex); // Release monitor lock so other threads can access buffer

        if (sleepTime > 0) { // Simulate processing time to prevent blocking shared access to buffer
            usleep(sleepTime * MICROSECONDS);
        }
    }
}
