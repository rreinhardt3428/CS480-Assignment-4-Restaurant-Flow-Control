#include <unistd.h>
#include <iostream>
#include "queue.h"
#include "log.h"
#include "seating.h"

extern RequestQueue requestQueue;
extern int totalRequests;         // Set via -s flag or default (120)
extern int vip_sleep_ms;          // Set via -v flag
extern int general_sleep_ms;      // Set via -g flag

void* producer(void* arg) {
    intptr_t raw_value = reinterpret_cast<intptr_t>(arg);
    RequestType type = static_cast<RequestType>(raw_value);

    int sleep_time = 0;
    if (type == VIPRoom){
        sleep_time = vip_sleep_ms;
    }
    else if (type == GeneralTable){
        sleep_time = general_sleep_ms;
    }

    while (true) {
        if (sleep_time > 0) {
            usleep(sleep_time * 1000); // Sleep outside critical section
        }

        pthread_mutex_lock(&requestQueue.mutex);

        if (requestQueue.totalProduced >= totalRequests) {
            pthread_mutex_unlock(&requestQueue.mutex);
            break;
        }

        bool queue_full = requestQueue.is_full(type);
        bool vip_full = (type == VIPRoom && requestQueue.inQueue[VIPRoom] >= 5);

        // Wait if the queue is full or VIP constraint is hit
        while (queue_full || vip_full) {
            pthread_cond_wait(&requestQueue.not_full, &requestQueue.mutex);

            if (requestQueue.totalProduced >= totalRequests) {
                pthread_mutex_unlock(&requestQueue.mutex);
                return nullptr;
            }

            queue_full = requestQueue.is_full(type);
            vip_full = (type == VIPRoom && requestQueue.inQueue[VIPRoom] >= 5);
        }

        requestQueue.queue.push(type);
        requestQueue.inQueue[type]++;
        requestQueue.produced[type]++;
        requestQueue.totalProduced++;

        output_request_added(type, requestQueue.produced, requestQueue.inQueue);

        pthread_cond_signal(&requestQueue.not_empty);
        pthread_mutex_unlock(&requestQueue.mutex);
    }

    return nullptr;
}
