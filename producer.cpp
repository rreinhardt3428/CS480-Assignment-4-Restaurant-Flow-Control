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
    RequestType type = static_cast<RequestType>(reinterpret_cast<intptr_t>(arg));
    int sleep_time = (type == VIPRoom) ? vip_sleep_ms : general_sleep_ms;

    while (true) {
        if (sleep_time > 0) {
            usleep(sleep_time * 1000); // Sleep outside critical section
        }

        pthread_mutex_lock(&requestQueue.mutex);

        if (requestQueue.totalProduced >= totalRequests) {
            pthread_mutex_unlock(&requestQueue.mutex);
            break;
        }

        // Wait if the queue is full or VIP constraint is hit
        while (requestQueue.is_full(type) ||
               (type == VIPRoom && requestQueue.inQueue[VIPRoom] >= 5)) {
            pthread_cond_wait(&requestQueue.not_full, &requestQueue.mutex);

            if (requestQueue.totalProduced >= totalRequests) {
                pthread_mutex_unlock(&requestQueue.mutex);
                return nullptr;
            }
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
