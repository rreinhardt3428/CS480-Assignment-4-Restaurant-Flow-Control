// Ethan Kent CS480
// REDID: 826843661
// Roger Reinhardt
// REDID: 826470808

#include <unistd.h>
#include <iostream>
#include "queue.h"
#include "log.h"
#include "seating.h"
#include "sharedData.h"

const int VIP_LIMIT = 5; // max number of VIP requests allowed in queue at a time
const int MICROSECONDS = 1000; // used to convert milliseconds to microseconds for usleep

void* producer(void* arg) { // function for producer thread, input is either generaltable or viproom and returns nullptr on completion
    ProducerArgs* args = static_cast<ProducerArgs*>(arg); // Initialize pointer to ProducerArgs struct

    // Populate local variables with values and shared data from args
    RequestQueue* requestQueue = args->requestQueue;
    int totalRequests = args->totalRequests;
    int vipSleepTime = args->vipSleepTime;
    int generalSleepTime = args->generalSleepTime;
    RequestType producerType = args->role;

    int sleepTime = 0; // calculating how long the producer needs to pause inbetween productions
    if (producerType == VIPRoom) {
        sleepTime = vipSleepTime;
    }
    if (producerType == GeneralTable) {
        sleepTime = generalSleepTime;
    }

    while (true) { // loop for generating requests until the production is met
        if (sleepTime > 0) {
            usleep(sleepTime * MICROSECONDS); // sleep outside critical section
        }

        pthread_mutex_lock(&requestQueue->mutex); // begin the monitor using mutex lock for the access shared buffer

        if (requestQueue->totalProduced >= totalRequests) { // checking if the production limit has been reached
            pthread_mutex_unlock(&requestQueue->mutex); // releasing the monitor lock
            break;
        }

        bool queueFull = requestQueue->isFull(producerType); // checking if the buffer is full and doesn't have room for more
        bool vipFull = (producerType == VIPRoom && requestQueue->inQueue[VIPRoom] >= VIP_LIMIT); // checking if the vip room exceeds the limit of 5 concurrent entries


        while (queueFull || vipFull) { // waiting for the spaceAvailable condition until there is more space
            pthread_cond_wait(&requestQueue->spaceAvailable, &requestQueue->mutex);

            if (requestQueue->totalProduced >= totalRequests) { // checking production limit after waking up
                pthread_mutex_unlock(&requestQueue->mutex); // releasing the monitor lock
                return nullptr;
            }

            queueFull = requestQueue->isFull(producerType); // computing the new condition after any changes
            vipFull = (producerType == VIPRoom && requestQueue->inQueue[VIPRoom] >= VIP_LIMIT);
        }

        requestQueue->queue.push(producerType); // adding the new requests to the queue and updating the production state
        pthread_cond_signal(&requestQueue->requestAvailable); // signaling the other side to make sure there are no threads asleep

        requestQueue->inQueue[producerType]++;
        requestQueue->produced[producerType]++;
        requestQueue->totalProduced++;

        output_request_added(producerType, requestQueue->produced, requestQueue->inQueue); // logging the new request

        //pthread_cond_signal(&requestQueue->requestAvailable); // signalling that a request is available for a waiting consumer
        pthread_mutex_unlock(&requestQueue->mutex); // releasing the monitor lock on the queue so that other threads can access it
    }

    return nullptr;
}