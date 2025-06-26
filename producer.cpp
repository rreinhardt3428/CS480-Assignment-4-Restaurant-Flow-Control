#include <unistd.h>
#include <iostream>
#include "queue.h"
#include "log.h"
#include "seating.h"


extern RequestQueue requestQueue;
extern int totalRequests; // -s flag or the default 120
extern int vipSleepTime; // -v flag
extern int generalSleepTime; // -g flag

void* producer(void* arg) { // function for producer thread, input is either generaltable or viproom and returns nullptr on completion
    intptr_t typeInt = reinterpret_cast<intptr_t>(arg);
    RequestType type = static_cast<RequestType>(typeInt);

    int sleepTime = 0; // calculating how long the producer needs to pause inbetween productions
    if (type == VIPRoom) {
        sleepTime = vipSleepTime;
    } else if (type == GeneralTable) {
        sleepTime = generalSleepTime;
    }

    while (true) { // loop for generating requests until the production is met
        if (sleepTime > 0) {
            usleep(sleepTime * 1000); // Sleep outside critical section
        }

        pthread_mutex_lock(&requestQueue.mutex); // locking the queue in order to perform operations

        if (requestQueue.totalProduced >= totalRequests) { //exit condition if the total request production is met
            pthread_mutex_unlock(&requestQueue.mutex);
            break;
        }

        bool queueFull = requestQueue.isFull(type); // checking if the queue is full and does not have room for more
        bool vipFull = (type == VIPRoom && requestQueue.inQueue[VIPRoom] >= 5); // checking if the vip room exceeds the limit of 5 concurrent entries


        while (queueFull || vipFull) { // waiting for the spaceAvailable condition until there is more space
            pthread_cond_wait(&requestQueue.spaceAvailable, &requestQueue.mutex);

            if (requestQueue.totalProduced >= totalRequests) { //checking the exit condition
                pthread_mutex_unlock(&requestQueue.mutex);
                return nullptr;
            }

            queueFull = requestQueue.isFull(type); // computing the new condition after any changes
            vipFull = (type == VIPRoom && requestQueue.inQueue[VIPRoom] >= 5);
        }

        requestQueue.queue.push(type); // adding the new requests to the queue and updating the production state
        requestQueue.inQueue[type]++;
        requestQueue.produced[type]++;
        requestQueue.totalProduced++;

        output_request_added(type, requestQueue.produced, requestQueue.inQueue); // logging the new request

        pthread_cond_signal(&requestQueue.requestAvailable); // signaling that there is an available request to a waiting consumer
        pthread_mutex_unlock(&requestQueue.mutex); // unlocking the queue so that any other threads can access
    }

    return nullptr;
}
