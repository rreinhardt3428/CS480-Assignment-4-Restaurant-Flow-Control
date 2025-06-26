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
#include "sharedData.h"

const int MICROSECONDS = 1000; // converts milliseconds to microseconds for usleep

void* consumer(void* arg) { // function for consumer thread; input is either TX or REV9 and returns a nullptr on completion
    ConsumerArgs* args = static_cast<ConsumerArgs*>(arg); // Initialize pointer to ConsumerArgs struct  

    // Populate local variables with values and shared data from args
    ConsumerType consumerType = args->role;
    RequestQueue* requestQueue = args->requestQueue;
    int totalRequests = args->totalRequests;
    sem_t* barrier = args->barrier;
    int txSleepTime = args->txSleepTime;
    int rev9SleepTime = args->rev9SleepTime;

    // calculating how long the consumer should wait between processing the requests
    int sleepTime = 0;
    if (consumerType == TX) {
        sleepTime = txSleepTime;
    }
    if (consumerType == Rev9) {
        sleepTime = rev9SleepTime;
    }

    while (true) {
        pthread_mutex_lock(&requestQueue->mutex); // begin the monitor using mutex lock for access shared buffer

        bool queueIsEmpty = requestQueue->queue.empty(); // checking if bounded buffer is empty and if more requests are coming
        bool moreRequestsExpected = (requestQueue->totalConsumed < totalRequests);


        while (queueIsEmpty && moreRequestsExpected) { // checking if the queue is empty and requests are available for consumption
            pthread_cond_wait(&requestQueue->requestAvailable, &requestQueue->mutex);

            queueIsEmpty = requestQueue->queue.empty(); // checking conditions after waking up
            moreRequestsExpected = (requestQueue->totalConsumed < totalRequests);

            if (!queueIsEmpty && requestQueue->totalConsumed >= totalRequests) { // checking
                pthread_mutex_unlock(&requestQueue->mutex);
                break;
            }
        }

        if (requestQueue->queue.empty() && requestQueue->totalConsumed >= totalRequests) { // checking if all requests have been consumed
            pthread_mutex_unlock(&requestQueue->mutex); // releasing the monitor lock
            return nullptr;
        }

        RequestType request = requestQueue->queue.front(); // getting the next requests from the front of the queue
        requestQueue->queue.pop();
        pthread_cond_signal(&requestQueue->spaceAvailable);// signaling the other side

        requestQueue->inQueue[request]--; // updating buffer state and consumer statistics
        int tempCount = requestQueue->consumed[consumerType][request];
        tempCount++;
        requestQueue->consumed[consumerType][request] = tempCount;

        requestQueue->totalConsumed++;

        output_request_removed(consumerType, request, requestQueue->consumed[consumerType], requestQueue->inQueue); // necessary logging

        if (requestQueue->queue.empty() && requestQueue->totalConsumed >= totalRequests) {
            if (!requestQueue->barrierPosted) { // added to ensure that we can make forward progress
                requestQueue->barrierPosted = true;
                sem_post(barrier);
                pthread_cond_broadcast(&requestQueue->requestAvailable); // waking all the consumers in the case that they are asleep

            }
            pthread_mutex_unlock(&requestQueue->mutex);
            return nullptr;
        }


        pthread_cond_signal(&requestQueue->spaceAvailable); // signals that the space is in the queue for producers
        pthread_mutex_unlock(&requestQueue->mutex); // releasing the monitor lock so other threads can access buffer

        if (sleepTime > 0) { // simulating the processing time to prevent blocking shared access to buffer
            usleep(sleepTime * MICROSECONDS);
        }
    }
}