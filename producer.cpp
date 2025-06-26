// Ethan Kent CS480
// REDID: 826843661
// Roger Reinhardt
// REDID: 826470808

#include <unistd.h>
#include <iostream>
#include "queue.h"
#include "log.h"
#include "seating.h"

const int VIP_LIMIT = 5; // Maximum number of VIP requests allowed in queue at a time
const int MICROSECONDS = 1000; // Converts milliseconds to microseconds for usleep

extern RequestQueue requestQueue;
extern int totalRequests; // -s flag or the default 120
extern int vipSleepTime; // -v flag
extern int generalSleepTime; // -g flag

void* producer(void* arg) { // function for producer thread, input is either generaltable or viproom and returns nullptr on completion
    intptr_t typeInt = reinterpret_cast<intptr_t>(arg);
    RequestType type = static_cast<RequestType>(typeInt);

    int sleepTime = 0; // Calculate how long the producer needs to pause inbetween productions
    if (type == VIPRoom) {
        sleepTime = vipSleepTime;
    } else if (type == GeneralTable) {
        sleepTime = generalSleepTime;
    }

    while (true) { // loop for generating requests until the production is met
        if (sleepTime > 0) {
            usleep(sleepTime * MICROSECONDS); // Sleep outside critical section
        }

        pthread_mutex_lock(&requestQueue.mutex); // Begin monitor using mutex lock for access shared buffer

        if (requestQueue.totalProduced >= totalRequests) { // Check if production limit has been reached
            pthread_mutex_unlock(&requestQueue.mutex); // Releases monitor lock
            break;
        }

        bool queueFull = requestQueue.isFull(type); // checking if the buffer is full and doesn't have room for more
        bool vipFull = (type == VIPRoom && requestQueue.inQueue[VIPRoom] >= VIP_LIMIT); // checking if the vip room exceeds the limit of 5 concurrent entries


        while (queueFull || vipFull) { // waiting for the spaceAvailable condition until there is more space
            pthread_cond_wait(&requestQueue.spaceAvailable, &requestQueue.mutex);

            if (requestQueue.totalProduced >= totalRequests) { // Check production limit after waking up
                pthread_mutex_unlock(&requestQueue.mutex); 
                return nullptr;
            }

            queueFull = requestQueue.isFull(type); // computing the new condition after any changes
            vipFull = (type == VIPRoom && requestQueue.inQueue[VIPRoom] >= VIP_LIMIT);
        }

        requestQueue.queue.push(type); // adding the new requests to the queue and updating the production state
        requestQueue.inQueue[type]++;
        requestQueue.produced[type]++;
        requestQueue.totalProduced++;

        output_request_added(type, requestQueue.produced, requestQueue.inQueue); // logging the new request

        pthread_cond_signal(&requestQueue.requestAvailable); // Signals that a request is available for a waiting consumer
        pthread_mutex_unlock(&requestQueue.mutex); // Release monitor lock on the queue so other threads can access it
    }

    return nullptr;
}
